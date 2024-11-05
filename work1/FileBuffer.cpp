// FileBuffer.cpp
#include "FileBuffer.h"

// 构造函数，初始化文件描述符为 -1，表示文件尚未打开
FileBuffer::FileBuffer() : fd(-1) {}

// 打开文件，指定文件路径、打开模式和权限
bool FileBuffer::openFile(const char* pathname, int flags, mode_t mode) {
    fd = open(pathname, flags, mode);  // 使用 open 系统调用打开文件
    if (fd == -1) {  // 如果打开失败，打印错误信息
        std::cerr << "Error opening file: " << strerror(errno) << std::endl;
        return false;
    }
    return true;  // 返回 true 表示文件成功打开
}

// 从文件中读取数据到缓冲区
ssize_t FileBuffer::readFile(char* buffer, size_t count) {
    ssize_t bytesRead = read(fd, buffer, count);  // 使用 read 系统调用读取数据
    if (bytesRead == -1) {  // 如果读取失败，打印错误信息
        std::cerr << "Error reading from file: " << strerror(errno) << std::endl;
    }
    return bytesRead;  // 返回实际读取的字节数
}

// 向文件中写入数据
ssize_t FileBuffer::writeFile(const char* buffer, size_t count) {
    ssize_t bytesWritten = write(fd, buffer, count);  // 使用 write 系统调用写入数据
    if (bytesWritten == -1) {  // 如果写入失败，打印错误信息
        std::cerr << "Error writing to file: " << strerror(errno) << std::endl;
    }
    return bytesWritten;  // 返回实际写入的字节数
}

// 刷新文件，将缓存中的数据写入磁盘
void FileBuffer::flushFile() {
    // 使用 fsync 确保文件的所有缓存数据被写入磁盘
    if (fsync(fd) == -1) {
        std::cerr << "Error flushing file to disk: " << strerror(errno) << std::endl;
    }
}

// 调整文件指针的位置
off_t FileBuffer::seekFile(off_t offset, int whence) {
    // 使用 lseek 调整文件指针的位置
    return lseek(fd, offset, whence);
}

// 关闭文件，释放文件描述符
void FileBuffer::closeFile() {
    if (fd != -1) {  // 如果文件已打开
        close(fd);  // 使用 close 系统调用关闭文件
        fd = -1;  // 将文件描述符重置为 -1，表示文件已关闭
    }
}

// 析构函数，确保在对象销毁时文件被正确关闭
FileBuffer::~FileBuffer() {
    closeFile();  // 调用 closeFile 以确保资源释放
}