import subprocess
import time
import argparse
import os
import signal
import fcntl
import select 

# 默认路径和参数
DEFAULT_SERVER_PATH = "./build/bin/rmdb"
DEFAULT_CLIENT_PATH = "./rmdb_client/build/rmdb_client"
DEFAULT_SQL_FILE = "./new_order.sql"
DEFAULT_DB_NAME = "test_db"  # 默认数据库名

def parse_sql_statements(sql_file):
    """按分号分割SQL语句，支持多行和注释跳过"""
    statements = []
    with open(sql_file, 'r') as f:
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
            statements.append(sql+';')
    return statements

def execute_client_server_mode(server_path, client_path, sql_file, db_name=DEFAULT_DB_NAME, output_file=None):
    """使用客户端-服务端模式执行SQL文件，检测错误并立即停止"""
    print(f"客户端-服务端模式执行SQL文件: {sql_file}, 数据库: {db_name}")
    
    # 设置输出文件
    detail_output = f"{os.path.splitext(sql_file)[0]}_cs_detail.log"
    if output_file is None:
        output_file = f"{os.path.splitext(sql_file)[0]}_cs_result.txt"
    
    server_process = None
    try:
        # 启动服务端
        print(f"启动服务端: {server_path} {db_name}")
        server_process = subprocess.Popen(
            [server_path, db_name],
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True
        )
        
        print("等待服务器启动...")
        time.sleep(3)  # 增加服务器启动时间

        # 检查服务器是否仍在运行
        if server_process.poll() is not None:
            print(f"错误：服务器进程异常终止，返回码: {server_process.returncode}")
            # 尝试获取服务器错误输出
            output = server_process.stdout.read()
            print(f"服务器输出: {output}")
            return False
        
        # 使用 robust SQL 解析
        sql_statements = parse_sql_statements(sql_file)
        print(f"加载了 {len(sql_statements)} 条SQL语句")
        
        # 启动客户端
        client_process = subprocess.Popen(
            [client_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1  # 行缓冲，提高响应速度
        )
        
        # 创建输出文件
        with open(output_file, 'w') as out, open(detail_output, 'w') as detail:
            # 向客户端发送SQL命令
            for i, statement in enumerate(sql_statements):
                print(f"SQL #{i+1}: {statement[:50]}{'...' if len(statement) > 50 else ''}")
                
                # 检查服务器是否还在运行
                if server_process.poll() is not None:
                    print(f"\n[!] 服务器在SQL #{i+1}之前崩溃了!")
                    print(f"服务器返回码: {server_process.returncode}")
                    try:
                        output = server_process.stdout.read()
                        if output:
                            print(f"服务器输出: {output}")
                    except:
                        pass
                    return False
                
                # 发送SQL语句
                try:
                    client_process.stdin.write(statement + "\n")
                    client_process.stdin.flush()
                except BrokenPipeError:
                    print(f"\n[!] 客户端管道断开，服务器可能崩溃在SQL #{i+1}")
                    return False
                
                # 设置非阻塞读取
                fd = client_process.stdout.fileno()
                flags = fcntl.fcntl(fd, fcntl.F_GETFL)
                fcntl.fcntl(fd, fcntl.F_SETFL, flags | os.O_NONBLOCK)
                
                # 智能输出完成检测
                error_detected = False
                response_text = ""
                lines_received = []
                max_timeout = 8.0  # 增加最大超时时间
                idle_timeout = 1.0  # 增加空闲超时时间
                start_time = time.time()
                last_output_time = start_time
                
                while time.time() - start_time < max_timeout:
                    # 再次检查服务器状态
                    if server_process.poll() is not None:
                        print(f"\n[!] 服务器在执行SQL #{i+1}时崩溃!")
                        print(f"崩溃的SQL: {statement}")
                        print(f"服务器返回码: {server_process.returncode}")
                        return False
                    
                    # 检查是否有数据可读
                    r, _, _ = select.select([fd], [], [], 0.1)
                    if not r:
                        # 检查空闲超时
                        if time.time() - last_output_time > idle_timeout:
                            # 如果是SELECT语句且有输出，认为完成
                            if statement.upper().strip().startswith('SELECT') and lines_received:
                                break
                            # 如果是非SELECT语句且空闲超过阈值，认为完成
                            elif not statement.upper().strip().startswith('SELECT') and time.time() - last_output_time > 0.5:
                                break
                        continue
                        
                    try:
                        line = client_process.stdout.readline()
                        if line:
                            out.write(line)
                            detail.write(line)
                            response_text += line
                            lines_received.append(line.strip())
                            last_output_time = time.time()
                            
                            # 检查严重错误关键词（排除正常的abort操作）
                            line_lower = line.lower().strip()
                            serious_errors = ["segmentation", "fault", "core dumped", "assertion", "fatal", "exception"]
                            if any(keyword in line_lower for keyword in serious_errors):
                                error_detected = True
                                print(f"\n[!] 检测到错误: {line.strip()}")
                                break
                            
                            # 检查完成标志
                            line_lower = line.lower().strip()
                            if any(completion in line_lower for completion in ["affected", "inserted", "updated", "deleted", "created", "dropped"]):
                                # 操作完成标志，等待短暂时间收集剩余输出
                                time.sleep(0.1)
                                break
                                
                    except IOError:
                        # 没有更多数据可读，短暂等待
                        time.sleep(0.05)
                
                # 如果检测到错误
                if error_detected:
                    print(f"\n[!] SQL语句 #{i+1} 执行失败: {statement}")
                    print(f"[!] 错误信息: {response_text.strip()}")
                    
                    # 记录错误详情
                    detail.write(f"\n===== 失败SQL #{i+1} =====\n{statement}\n")
                    
                    # 发送退出并终止
                    client_process.stdin.write("exit;\n")
                    client_process.stdin.flush()
                    client_process.terminate()
                    return False
            
            # 所有SQL执行完毕，发送退出
            client_process.stdin.write("exit;\n")
            client_process.stdin.flush()
            
            # 收集最后输出
            try:
                fcntl.fcntl(fd, fcntl.F_SETFL, flags)  # 恢复阻塞模式
                remaining_output, _ = client_process.communicate(timeout=5)
                if remaining_output:
                    out.write(remaining_output)
                    detail.write(remaining_output)
            except subprocess.TimeoutExpired:
                print("客户端最终输出收集超时，强制终止")
                client_process.terminate()
                try:
                    client_process.wait(timeout=2)
                except subprocess.TimeoutExpired:
                    client_process.kill()
        
        print("所有SQL语句执行成功!")
        return True
        
    except Exception as e:
        print(f"执行出错: {e}")
        return False
        
    finally:
    # 关闭服务端
        if server_process:
            if server_process.poll() is None:  # 如果仍在运行
                print("关闭服务器进程...")
                server_process.terminate()
                try:
                    server_process.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    print("服务器未响应，强制终止")
                    server_process.kill()
            else:
                print(f"服务器已终止，返回码: {server_process.returncode}")

def execute_direct_mode(db_executable, sql_file, db_name=DEFAULT_DB_NAME, output_file=None):
    """执行单进程模式的SQL文件（无客户端-服务端）"""
    print(f"直接模式执行SQL文件: {sql_file}, 数据库: {db_name}")
    
    # 设置详细输出文件
    detail_output = f"{os.path.splitext(sql_file)[0]}_detail.log"
    # 如果未指定输出文件，则使用默认格式
    if output_file is None:
        output_file = f"{os.path.splitext(sql_file)[0]}_result.txt"
    
    try:
        # 启动数据库进程 - 添加数据库名称参数
        db_process = subprocess.Popen(
            [db_executable, db_name],  # 关键修改：添加数据库名称
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            bufsize=1
        )
        
        # 读取SQL文件内容
        with open(sql_file, 'r') as f:
            sql_content = f.read()
        
        # 发送SQL命令到数据库进程
        print("开始发送SQL命令...")
        db_process.stdin.write(sql_content)
        db_process.stdin.write("\nexit;\n")  # 确保退出
        db_process.stdin.flush()
        
        # 收集输出并保存详细日志
        all_output = []
        with open(output_file, 'w') as out, open(detail_output, 'w') as detail:
            while True:
                line = db_process.stdout.readline()
                if not line and db_process.poll() is not None:
                    break
                    
                if line:
                    out.write(line)
                    detail.write(line)
                    all_output.append(line.rstrip())
                    print(line.rstrip())
        
        # 等待进程退出
        return_code = db_process.wait(timeout=10)
        print(f"执行结果: {'成功' if return_code == 0 else f'失败 (代码: {return_code})'}")
        print(f"详细输出已保存至: {detail_output}")
        
        # 将完整命令和参数也记录到详细日志
        with open(detail_output, 'a') as f:
            f.write("\n\n===== 命令信息 =====\n")
            f.write(f"可执行文件: {db_executable}\n")
            f.write(f"数据库参数: {db_name}\n")
            f.write(f"SQL文件: {sql_file}\n")
            f.write(f"返回码: {return_code}\n")
        
        return return_code == 0
        
    except Exception as e:
        print(f"执行出错: {e}")
        with open(detail_output, 'a') as f:
            f.write(f"\n\n===== 异常信息 =====\n{str(e)}\n")
        return False

def main():
    parser = argparse.ArgumentParser(description="批量执行SQL测试文件")
    parser.add_argument("--mode", choices=["direct", "client-server"], default="client-server", 
                        help="执行模式: direct(单进程) 或 client-server(客户端服务端)")
    parser.add_argument("--server", default=DEFAULT_SERVER_PATH, 
                        help=f"服务端可执行文件路径 (默认: {DEFAULT_SERVER_PATH})")
    parser.add_argument("--client", default=DEFAULT_CLIENT_PATH, 
                        help=f"客户端可执行文件路径 (默认: {DEFAULT_CLIENT_PATH})")
    parser.add_argument("--sql", default=DEFAULT_SQL_FILE, 
                        help=f"SQL文件或目录 (默认: {DEFAULT_SQL_FILE})")
    parser.add_argument("--db-name", default=DEFAULT_DB_NAME,
                        help=f"数据库名称 (默认: {DEFAULT_DB_NAME})")
    parser.add_argument("--output", help="输出结果文件")
    
    args = parser.parse_args()
    
    # 检查是单个文件还是目录
    if os.path.isdir(args.sql):
        print(f"扫描目录: {args.sql}")
        sql_files = [os.path.join(args.sql, f) for f in os.listdir(args.sql) if f.endswith(".sql")]
        
        success_count = 0
        for sql_file in sql_files:
            if args.mode == "client-server":
                success = execute_client_server_mode(args.server, args.client, sql_file, args.db_name)
            else:
                success = execute_direct_mode(args.server, sql_file, args.db_name)
                
            if success:
                success_count += 1
            print(f"完成: {os.path.basename(sql_file)} - {'通过' if success else '失败'}")
            print("-" * 50)
            
        print(f"测试结果: {success_count}/{len(sql_files)} 个文件成功执行")
    else:
        print(f"执行单个文件: {args.sql}")
        if args.mode == "client-server":
            success = execute_client_server_mode(args.server, args.client, args.sql, args.db_name, args.output)
        else:
            success = execute_direct_mode(args.server, args.sql, args.db_name, args.output)
        print(f"测试结果: {'通过' if success else '失败'}")

if __name__ == "__main__":
    main()