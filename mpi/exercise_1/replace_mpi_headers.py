import os

def replace_in_file(file_path, old_text, new_text):
    with open(file_path, 'r') as file:
        file_data = file.read()

    # 替换文本
    new_data = file_data.replace(old_text, new_text)

    with open(file_path, 'w') as file:
        file.write(new_data)

def traverse_and_replace(directory, old_text, new_text):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".c") or file.endswith(".cpp") or file.endswith(".h"):
                file_path = os.path.join(root, file)
                replace_in_file(file_path, old_text, new_text)
                print(f"Processed: {file_path}")

# 指定要遍历的目录
directory = "./"

# 替换 "mpi.h" 为 <mpi/mpi.h>
traverse_and_replace(directory, '"mpi.h"', '<mpi/mpi.h>')

