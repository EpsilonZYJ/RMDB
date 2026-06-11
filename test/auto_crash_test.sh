#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

DB_SERVER="${ROOT_DIR}/build/bin/rmdb database"
DB_CLIENT="${ROOT_DIR}/rmdb_client/build/rmdb_client"
SQL_FILE="${SCRIPT_DIR}/recover_test.sql"
OUT_DIR="${SCRIPT_DIR}/crash_test_output"
LOG_FILE="${SCRIPT_DIR}/crash_test_run.log"
BLOCK_DIR="${SCRIPT_DIR}/.crash_blocks"

mkdir -p "${OUT_DIR}" "${BLOCK_DIR}"
rm -f "${OUT_DIR}"/* "${LOG_FILE}" "${BLOCK_DIR}"/*.sql

# 1. 预处理SQL，按crash分块
awk '
    BEGIN{block=0}
    {
        print > "'"${BLOCK_DIR}"'/block_"block".sql"
        if ($0 ~ /crash/) { block++ }
    }
' "${SQL_FILE}"

BLOCKS=$(ls "${BLOCK_DIR}"/block_*.sql | sort -V)

i=0
for block in $BLOCKS; do
    echo "========== 执行块 $i ==========" | tee -a "${LOG_FILE}"
    $DB_SERVER > "${OUT_DIR}/server_${i}.log" 2>&1 &
    SERVER_PID=$!
    sleep 2

    $DB_CLIENT < "${block}" > "${OUT_DIR}/client_${i}.out" 2>&1

    # 检查服务端是否还活着
    if ps -p "${SERVER_PID}" > /dev/null; then
        alive=1
    else
        alive=0
    fi

    if grep -q "crash" "${block}"; then
        echo "检测到crash，尝试杀死数据库进程 ${SERVER_PID}" | tee -a "${LOG_FILE}"
        if [ $alive -eq 1 ]; then
            kill "${SERVER_PID}"
            sleep 2
        else
            echo "数据库进程 ${SERVER_PID} 已经退出" | tee -a "${LOG_FILE}"
        fi
    else
        if [ $alive -eq 1 ]; then
            kill "${SERVER_PID}"
            sleep 2
        fi
    fi

    i=$((i+1))
done

echo "========== 所有块执行完毕 ==========" | tee -a "${LOG_FILE}"

# 清理临时SQL块
rm -f "${BLOCK_DIR}"/block_*.sql
