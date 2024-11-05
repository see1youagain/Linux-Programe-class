
// test.cpp
#include "FileBuffer.h"
#include <iostream>
#include <chrono>
#include <cstring>

// 测试写入性能的函数
void testWritePerformance(const char* filename, bool useFsync) {
    FileBuffer fileBuffer;
    // 打开或创建文件，如果文件存在则清空原有内容
    if (fileBuffer.openFile(filename, O_RDWR | O_CREAT | O_TRUNC, 0644)) {
        const char* data = "Hello, using lzzy's Linux's system buffered I/O!";
        // 记录开始时间，用于计算写入性能
        auto start = std::chrono::high_resolution_clock::now();

        // 循环执行 1000 次写入操作
        for (int i = 0; i < 1000; ++i) {
            fileBuffer.writeFile(data, strlen(data));  // 写入数据到缓存
            if (useFsync) {
                fileBuffer.flushFile();  // 使用 fsync 将缓存数据写入磁盘，确保数据持久化
            }
        }

        // 记录结束时间，用于计算写入操作所耗费的时间
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = end - start;

        // 输出写入测试的耗时，根据是否使用 fsync 进行区分
        std::cout << "Test with" << (useFsync ? " " : "out ") << "fsync took " << elapsed.count() << " seconds." << std::endl;

        // 关闭文件，确保文件句柄被释放
        fileBuffer.closeFile();
    } else {
        // 如果文件打开失败，输出错误信息
        std::cerr << "Failed to open file: " << filename << std::endl;
    }
}

int main() {
    // 测试不使用 fsync 的情况，数据将只写入缓存中，不立即写入磁盘
    testWritePerformance("test_no_fsync.txt", false);
    
    // 测试使用 fsync 的情况，数据在每次写入后都会被立即同步到磁盘
    testWritePerformance("test_with_fsync.txt", true);

    return 0;
}