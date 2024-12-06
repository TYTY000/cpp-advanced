import requests
from bs4 import BeautifulSoup
import os

# 定义目标网页
url = "https://hpc-tutorials.llnl.gov/mpi/exercise_1/"

# 发送HTTP请求获取网页内容
response = requests.get(url)
response.raise_for_status()  # 确保请求成功

# 使用BeautifulSoup解析网页内容
soup = BeautifulSoup(response.text, 'html.parser')

# 查找所有超链接
links = soup.find_all('a')

# 创建保存文件的目录
os.makedirs('exercise_1', exist_ok=True)

# 下载所有后缀为 .c 的文件
for link in links:
    href = link.get('href')
    if href and href.endswith('.c'):
        if not href.startswith('https://'):
            file_url = "https://hpc-tutorials.llnl.gov/mpi/examples/" + href.split('/')[-1]
        else:
            file_url = href
        file_name = os.path.join('exercise_1', os.path.basename(href))
        file_response = requests.get(file_url)
        file_response.raise_for_status()
        with open(file_name, 'wb') as file:
            file.write(file_response.content)
        print(f"Downloaded {file_name}")

print("All .c files have been downloaded.")

