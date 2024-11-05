// FileBuffer.h
#ifndef FILEBUFFER_H
#define FILEBUFFER_H

#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sys/stat.h>

// FileBuffer 类用于封装文件操作，实现缓存管理和系统级文件 I/O 操作
class FileBuffer {
private:
    int fd;  // 文件描述符，标识已打开的文件

public:
    FileBuffer();  // 构造函数，初始化文件描述符

    // 打开文件，指定文件路径、打开模式和权限
    bool openFile(const char* pathname, int flags, mode_t mode = 0644);

    // 从文件中读取数据到缓冲区
    ssize_t readFile(char* buffer, size_t count);

    // 向文件中写入数据
    ssize_t writeFile(const char* buffer, size_t count);

    // 刷新文件，将缓存中的数据写入磁盘
    void flushFile();

    // 调整文件指针的位置
    off_t seekFile(off_t offset, int whence);

    // 关闭文件，释放资源
    void closeFile();

    ~FileBuffer();  // 析构函数，确保文件被正确关闭
};

#endif