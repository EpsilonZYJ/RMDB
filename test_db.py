import subprocess
import time
import argparse
import os
import signal
import threading
import queue

# 默认路径和参数
DEFAULT_SERVER_PATH = "./build/bin/rmdb"
DEFAULT_CLIENT_PATH = "./rmdb_client/build/rmdb_client"
DEFAULT_SQL_FILE = "./new_order.sql"
DEFAULT_DB_NAME = "test_db"

def parse_sql_statements(sql_file):
    """按分号分割SQL语句，支持多行和注释跳过"""
    statements = []
    with open(sql_file, 'r', encoding='utf-8') as f:
        sql = ''
        in_multiline_comment = False
        for line in f:
            line = line.strip()
            if not line:
                continue
            
            # 处理多行注释
            if in_multiline_comment:
                if '*/' in line:
                    in_multiline_comment = False
                    line = line.split('*/', 1)[1].strip()
                else:
                    continue
            if line.startswith('/*'):
                in_multiline_comment = True
                if '*/' in line:
                    in_multiline_comment = False
                    line = line.split('*/', 1)[1].strip()
                else:
                    continue
            
            # 跳过单行注释
            if line.startswith('--') or line.startswith('//'):
                continue
            
            # 去除行内注释
            if '--' in line:
                line = line.split('--', 1)[0].strip()
            if '//' in line:
                line = line.split('//', 1)[0].strip()
            
            sql += (' ' if sql else '') + line
            while ';' in sql:
                idx = sql.find(';')
                statement = sql[:idx+1].strip()
                if statement:
                    statements.append(statement)
                sql = sql[idx+1:].strip()
        
        if sql:
            statements.append(sql + ';')
    return statements

def read_output_with_timeout(process, timeout, output_queue, error_queue):
    """使用线程读取进程输出，避免阻塞"""
    try:
        while True:
            line = process.stdout.readline()
            if not line:
                if process.poll() is not None:
                    break
                time.sleep(0.01)
                continue
            output_queue.put(('output', line))
    except Exception as e:
        error_queue.put(('error', str(e)))

def execute_sql_statement(client_process, statement, timeout=10):
    """执行单条SQL语句并收集输出"""
    try:
        # 发送SQL语句
        client_process.stdin.write(statement + "\n")
        client_process.stdin.flush()
        
        # 创建输出队列和错误队列
        output_queue = queue.Queue()
        error_queue = queue.Queue()
        
        # 启动读取线程
        reader_thread = threading.Thread(
            target=read_output_with_timeout,
            args=(client_process, timeout, output_queue, error_queue)
        )
        reader_thread.daemon = True
        reader_thread.start()
        
        # 收集输出
        output_lines = []
        start_time = time.time()
        last_output_time = start_time
        
        # 根据SQL类型设置不同的超时
        if statement.upper().strip().startswith(('SELECT', 'SHOW', 'DESC')):
            base_timeout = min(timeout, 5)
        else:
            base_timeout = min(timeout, 3)
        
        while time.time() - start_time < base_timeout:
            try:
                # 检查是否有新输出
                msg_type, content = output_queue.get(timeout=0.1)
                if msg_type == 'output':
                    output_lines.append(content)
                    last_output_time = time.time()
                    
                    # 简化的完成检测
                    line_lower = content.lower().strip()
                    
                    # 检测错误关键词
                    error_keywords = ['segmentation', 'fault', 'assertion', 'fatal', 'exception', 'error:', 'failed']
                    if any(keyword in line_lower for keyword in error_keywords):
                        # 排除正常的操作（如 "0 record(s) affected"）
                        if not any(normal in line_lower for normal in ['record(s)', 'affected', 'total']):
                            return False, ''.join(output_lines), f"检测到错误: {content.strip()}"
                    
                    # 检测完成标志（更简单的逻辑）
                    completion_indicators = [
                        'record(s)',           # SELECT结果
                        'affected',            # INSERT/UPDATE/DELETE结果  
                        'created',             # CREATE结果
                        'dropped',             # DROP结果
                        'checkpoint',          # 检查点结果
                        'rucbase>',            # 提示符（表示命令完成）
                        '静态检查点创建成功',     # 中文提示
                        '事务',                # 事务相关操作完成
                    ]
                    
                    if any(indicator in line_lower for indicator in completion_indicators):
                        # 等待短暂时间收集可能的剩余输出
                        time.sleep(0.1)
                        
                        # 收集剩余输出
                        while not output_queue.empty():
                            try:
                                _, extra_content = output_queue.get_nowait()
                                output_lines.append(extra_content)
                            except queue.Empty:
                                break
                        
                        return True, ''.join(output_lines), None
                        
            except queue.Empty:
                # 没有新输出，检查是否超时
                if time.time() - last_output_time > 2.0:  # 2秒无输出则认为完成
                    break
            except Exception as e:
                return False, ''.join(output_lines), f"读取输出异常: {str(e)}"
        
        # 检查是否有错误
        if not error_queue.empty():
            _, error_msg = error_queue.get_nowait()
            return False, ''.join(output_lines), f"读取错误: {error_msg}"
        
        return True, ''.join(output_lines), None
        
    except BrokenPipeError:
        return False, '', "客户端管道断开"
    except Exception as e:
        return False, '', f"执行异常: {str(e)}"

def execute_client_server_mode(server_path, client_path, sql_file, db_name=DEFAULT_DB_NAME, output_file=None):
    """优化后的客户端-服务端模式执行"""
    print(f"客户端-服务端模式执行SQL文件: {sql_file}, 数据库: {db_name}")
    
    # 设置输出文件
    detail_output = f"{os.path.splitext(sql_file)[0]}_cs_detail.log"
    if output_file is None:
        output_file = f"{os.path.splitext(sql_file)[0]}_cs_result.txt"
    
    server_process = None
    client_process = None
    
    try:
        # 启动服务端
        print(f"启动服务端: {server_path} {db_name}")
        server_process = subprocess.Popen(
            [server_path, db_name],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=0  # 无缓冲
        )
        
        print("等待服务器启动...")
        time.sleep(2)
        
        # 检查服务器是否正常启动
        if server_process.poll() is not None:
            output = server_process.stdout.read()
            print(f"错误：服务器启动失败，返回码: {server_process.returncode}")
            print(f"服务器输出: {output}")
            return False
        
        # 解析SQL语句
        sql_statements = parse_sql_statements(sql_file)
        print(f"加载了 {len(sql_statements)} 条SQL语句")
        
        # 启动客户端
        print("启动客户端...")
        client_process = subprocess.Popen(
            [client_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=0  # 无缓冲，立即输出
        )
        
        # 等待客户端连接
        time.sleep(1)
        
        # 执行SQL语句
        with open(output_file, 'w') as out, open(detail_output, 'w') as detail:
            out.write(f"=== SQL执行结果 ({sql_file}) ===\n\n")
            detail.write(f"=== 详细执行日志 ({sql_file}) ===\n\n")
            
            for i, statement in enumerate(sql_statements, 1):
                print(f"执行SQL #{i}: {statement[:60]}{'...' if len(statement) > 60 else ''}")
                
                # 检查服务器状态
                if server_process.poll() is not None:
                    error_msg = f"服务器在SQL #{i}之前崩溃，返回码: {server_process.returncode}"
                    print(f"[ERROR] {error_msg}")
                    detail.write(f"[ERROR] {error_msg}\n")
                    return False
                
                # 检查客户端状态
                if client_process.poll() is not None:
                    error_msg = f"客户端在SQL #{i}之前断开，返回码: {client_process.returncode}"
                    print(f"[ERROR] {error_msg}")
                    detail.write(f"[ERROR] {error_msg}\n")
                    return False
                
                # 执行SQL语句
                success, output_text, error_msg = execute_sql_statement(client_process, statement)
                
                # 记录结果
                detail.write(f"--- SQL #{i} ---\n")
                detail.write(f"语句: {statement}\n")
                detail.write(f"结果: {'成功' if success else '失败'}\n")
                if error_msg:
                    detail.write(f"错误: {error_msg}\n")
                detail.write(f"输出:\n{output_text}\n")
                detail.write("-" * 50 + "\n")
                
                out.write(f"SQL #{i}: {statement}\n")
                out.write(output_text)
                out.write("\n" + "="*50 + "\n\n")
                
                if not success:
                    print(f"[ERROR] SQL #{i} 执行失败: {error_msg}")
                    print(f"失败的语句: {statement}")
                    return False
                else:
                    print(f"[OK] SQL #{i} 执行成功")
            
            # 发送退出命令
            client_process.stdin.write("exit;\n")
            client_process.stdin.flush()
        
        print("所有SQL语句执行成功!")
        return True
        
    except Exception as e:
        print(f"执行出错: {e}")
        return False
        
    finally:
        # 清理进程
        if client_process and client_process.poll() is None:
            try:
                client_process.terminate()
                client_process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                client_process.kill()
        
        if server_process and server_process.poll() is None:
            try:
                server_process.terminate()
                server_process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                server_process.kill()

def execute_direct_mode(db_executable, sql_file, db_name=DEFAULT_DB_NAME, output_file=None):
    """优化后的直接模式执行"""
    print(f"直接模式执行SQL文件: {sql_file}, 数据库: {db_name}")
    
    detail_output = f"{os.path.splitext(sql_file)[0]}_detail.log"
    if output_file is None:
        output_file = f"{os.path.splitext(sql_file)[0]}_result.txt"
    
    try:
        # 启动数据库进程
        db_process = subprocess.Popen(
            [db_executable, db_name],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=0
        )
        
        # 读取SQL文件
        with open(sql_file, 'r') as f:
            sql_content = f.read()
        
        print("发送SQL命令...")
        db_process.stdin.write(sql_content + "\nexit;\n")
        db_process.stdin.close()
        
        # 收集输出
        output_lines = []
        with open(output_file, 'w') as out, open(detail_output, 'w') as detail:
            while True:
                line = db_process.stdout.readline()
                if not line and db_process.poll() is not None:
                    break
                
                if line:
                    output_lines.append(line)
                    out.write(line)
                    detail.write(line)
                    print(line.rstrip())
        
        return_code = db_process.wait(timeout=15)
        success = return_code == 0
        
        print(f"执行完成: {'成功' if success else f'失败 (返回码: {return_code})'}")
        print(f"输出已保存至: {output_file}")
        print(f"详细日志: {detail_output}")
        
        return success
        
    except subprocess.TimeoutExpired:
        print("执行超时，强制终止进程")
        db_process.kill()
        return False
    except Exception as e:
        print(f"执行出错: {e}")
        return False

def main():
    parser = argparse.ArgumentParser(description="优化的SQL测试工具")
    parser.add_argument("--mode", choices=["direct", "client-server"], default="client-server")
    parser.add_argument("--server", default=DEFAULT_SERVER_PATH)
    parser.add_argument("--client", default=DEFAULT_CLIENT_PATH)
    parser.add_argument("--sql", default=DEFAULT_SQL_FILE)
    parser.add_argument("--db-name", default=DEFAULT_DB_NAME)
    parser.add_argument("--output", help="输出文件")
    
    args = parser.parse_args()
    
    # 检查文件是否存在
    if not os.path.exists(args.sql):
        print(f"错误: SQL文件不存在: {args.sql}")
        return
    
    if args.mode == "client-server":
        if not os.path.exists(args.server):
            print(f"错误: 服务端文件不存在: {args.server}")
            return
        if not os.path.exists(args.client):
            print(f"错误: 客户端文件不存在: {args.client}")
            return
    
    # 执行测试
    if os.path.isdir(args.sql):
        sql_files = [os.path.join(args.sql, f) for f in os.listdir(args.sql) if f.endswith(".sql")]
        sql_files.sort()
        
        success_count = 0
        for sql_file in sql_files:
            print(f"\n{'='*60}")
            print(f"测试文件: {os.path.basename(sql_file)}")
            print(f"{'='*60}")
            
            if args.mode == "client-server":
                success = execute_client_server_mode(args.server, args.client, sql_file, args.db_name)
            else:
                success = execute_direct_mode(args.server, sql_file, args.db_name)
            
            if success:
                success_count += 1
                print(f"✅ {os.path.basename(sql_file)} - 通过")
            else:
                print(f"❌ {os.path.basename(sql_file)} - 失败")
        
        print(f"\n总结: {success_count}/{len(sql_files)} 个文件测试通过")
    else:
        if args.mode == "client-server":
            success = execute_client_server_mode(args.server, args.client, args.sql, args.db_name, args.output)
        else:
            success = execute_direct_mode(args.server, args.sql, args.db_name, args.output)
        
        print(f"测试结果: {'✅ 通过' if success else '❌ 失败'}")

if __name__ == "__main__":
    main()