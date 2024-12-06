#!/bin/bash

# 获取当前目录下所有的 .c 文件
for file in *.c; do
    # 获取不带扩展名的文件名
    filename="${file%.c}"
    # 使用 gcc 编译 .c 文件为可执行文件
    mpic++ -o "exe/$filename" "$file"
    if [ $? -eq 0 ]; then
        echo "Compiled $file successfully"
    else
        echo "Failed to compile $file"
    fi
done
