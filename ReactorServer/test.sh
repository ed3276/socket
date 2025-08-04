#!/bin/bash

# 检查是否提供了参数
if [ $# -eq 0 ]; then
    echo "错误：请提供循环次数作为参数"
    echo "用法：$0 <循环次数>"
    exit 1
fi

# 获取循环次数
count=$1

# 检查参数是否是数字
if ! [[ "$count" =~ ^[0-9]+$ ]]; then
    echo "错误：参数必须是正整数"
    exit 1
fi

# 执行循环
for ((i=1; i<=$count; i++)); do
    ./build/client 192.168.74.130 8279 &
done

echo "完成执行 $count 次"
