#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <filesystem>
#include <future>
#include <queue>
#include <mutex>
#include "ThreadPool.h"

namespace fs = std::filesystem;

void sortFile(const std::string &inputFilePath, const std::string &outputFilePath) {
    std::ifstream inFile(inputFilePath);
    if (!inFile.is_open()) {
        std::cerr << "Error opening input file: " << inputFilePath << std::endl;
        return;
    }

    std::vector<int64_t> data;
    int64_t value;

    // 读取文件中的数据到内存
    while (inFile >> value) {
        data.push_back(value);
    }
    inFile.close();

    // 对数据进行排序
    std::sort(data.begin(), data.end());

    // 写入排序后的数据
    std::ofstream sortedFile(outputFilePath);
    if (!sortedFile.is_open()) {
        std::cerr << "Error opening output file: " << outputFilePath << std::endl;
        return;
    }

    for (const auto &val : data) {
        sortedFile << val << "\n";
    }

    sortedFile.close();
    std::cout << "Finished writing sorted file: " << outputFilePath << std::endl;
}

void mergeTwoFiles(const std::string &file1, const std::string &file2, const std::string &outputFilePath) {
    std::ifstream inFile1(file1);
    std::ifstream inFile2(file2);
    std::ofstream outFile(outputFilePath);

    if (!inFile1.is_open() || !inFile2.is_open() || !outFile.is_open()) {
        std::cerr << "Error opening files for merging." << std::endl;
        return;
    }

    int64_t value1, value2;
    bool hasValue1 = static_cast<bool>(inFile1 >> value1);
    bool hasValue2 = static_cast<bool>(inFile2 >> value2);

    while (hasValue1 && hasValue2) {
        if (value1 < value2) {
            outFile << value1 << "\n";
            hasValue1 = static_cast<bool>(inFile1 >> value1);
        } else {
            outFile << value2 << "\n";
            hasValue2 = static_cast<bool>(inFile2 >> value2);
        }
    }

    while (hasValue1) {
        outFile << value1 << "\n";
        hasValue1 = static_cast<bool>(inFile1 >> value1);
    }

    while (hasValue2) {
        outFile << value2 << "\n";
        hasValue2 = static_cast<bool>(inFile2 >> value2);
    }

    inFile1.close();
    inFile2.close();
    outFile.close();

    std::cout << "Finished merging files into: " << outputFilePath << std::endl;
}

void incrementalMerge(ThreadPool &mergePool, std::deque<std::string> &sortedFilePaths, const std::string &outputDirectoryPath, int &mergeCounter) {
    while (sortedFilePaths.size() > 1) {
        std::string file1 = sortedFilePaths.front();
        sortedFilePaths.pop_front();
        std::string file2 = sortedFilePaths.front();
        sortedFilePaths.pop_front();

        std::string outputFilePath = outputDirectoryPath + "/merge_0_" + std::to_string(mergeCounter++) + ".txt";
        
        // 合并两个文件并将合并结果添加到合并队列
        mergePool.enqueueTask([file1, file2, outputFilePath, &sortedFilePaths]() mutable {
            mergeTwoFiles(file1, file2, outputFilePath);
            sortedFilePaths.push_back(outputFilePath);
        });
    }
}

int main() {
    std::string inputDirectoryPath = "/mnt/hgfs/LinuxClass_TestDir/input";
    std::string outputDirectoryPath = "/mnt/hgfs/LinuxClass_TestDir/output";

    if (!fs::exists(outputDirectoryPath)) {
        fs::create_directory(outputDirectoryPath);
    }

    std::vector<std::string> filePaths;

    for (const auto &entry : fs::directory_iterator(inputDirectoryPath)) {
        if (entry.is_regular_file()) {
            filePaths.push_back(entry.path().string());
        }
    }

    size_t totalThreads = std::thread::hardware_concurrency();
    size_t sortThreads = totalThreads * 1 / 2;  // 2/3用于文件排序
    size_t mergeThreads = totalThreads - sortThreads;  // 1/3用于合并

    int mergeCounter = 0; // 合并文件的编号

    ThreadPool sortPool(sortThreads);
    ThreadPool mergePool(mergeThreads);

    std::deque<std::string> sortedFilePaths;

    // 开始文件排序并将结果加入合并队列
    for (const auto &filePath : filePaths) {
        std::string outputFilePath = outputDirectoryPath + "/sorted_" + fs::path(filePath).stem().string() + ".txt";
        sortPool.enqueueTask([filePath, outputFilePath, &mergePool, &sortedFilePaths, &outputDirectoryPath, &mergeCounter]() mutable {
            sortFile(filePath, outputFilePath);

            // 排序完成后，加入合并队列
            sortedFilePaths.push_back(outputFilePath);

            if (sortedFilePaths.size() > 1) {
                incrementalMerge(mergePool, sortedFilePaths, outputDirectoryPath, mergeCounter);
            }
        });
    }

    sortPool.joinAll();  // 等待所有排序任务完成
    mergePool.joinAll();  // 等待所有合并任务完成

    if (!sortedFilePaths.empty()) {
        std::cout << "Final output file: " << sortedFilePaths.front() << std::endl;
    }

    return 0;
}
