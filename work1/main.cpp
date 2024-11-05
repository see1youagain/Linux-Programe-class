// main.cpp
#include "FileBuffer.h"
#include <iostream>

int main() {
    FileBuffer fileBuffer;
    // 打开文件 work1_example.txt，如果文件不存在则创建一个新文件
    if (fileBuffer.openFile("work1_example.txt", O_RDWR | O_CREAT, 0644)) {
        // 待写入的数据
        const char* data = "Hello, using lzzy's Linux's system buffered I/O!";
        // 将数据写入文件，此时数据只写入到缓存中
        fileBuffer.writeFile(data, strlen(data));
        // 不调用 fsync() 函数，确保数据留在系统缓存中，减少磁盘 I/O
        // fileBuffer.flushFile(); 

        char readData[128];
        // 将文件指针定位到文件开头，以便进行读取操作
        fileBuffer.seekFile(0, SEEK_SET);
        // 读取写入的数据到 readData 缓冲区中
        ssize_t bytesRead = fileBuffer.readFile(readData, strlen(data));
        if (bytesRead > 0) {
            readData[bytesRead] = '\0';  // 确保读取的字符串以空字符结尾
            // 输出读取到的数据，验证写入是否成功
            std::cout << "Read data: " << readData << std::endl;
        }

        // 关闭文件，确保文件句柄被释放
        fileBuffer.closeFile();
    }
    return 0;
}