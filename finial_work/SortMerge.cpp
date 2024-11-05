#include <fstream>
#include <queue>
#include <vector>
#include <string>
#include <iostream>
#include <limits>

void mergeFiles(const std::vector<std::string> &inputFiles, const std::string &outputFile) {
    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Error opening output file: " << outputFile << std::endl;
        return;
    }

    std::vector<std::ifstream> streams;
    for (const auto &file : inputFiles) {
        streams.emplace_back(file);
        if (!streams.back().is_open()) {
            std::cerr << "Error opening input file: " << file << std::endl;
            return;
        }
    }

    struct FileEntry {
        int64_t value;
        size_t index;
    };

    auto compare = [](const FileEntry &a, const FileEntry &b) {
        return a.value > b.value;
    };

    std::priority_queue<FileEntry, std::vector<FileEntry>, decltype(compare)> minHeap(compare);

    for (size_t i = 0; i < streams.size(); ++i) {
        int64_t value;
        if (streams[i] >> value) {
            minHeap.push({value, i});
        }
    }

    while (!minHeap.empty()) {
        auto [val, index] = minHeap.top();
        minHeap.pop();
        outFile << val << "\n";  // 将排序后的数据写入输出文件
        std::cout << "Writing value: " << val << " from stream " << index << " to output file." << std::endl;

        int64_t value;
        if (streams[index] >> value) {
            minHeap.push({value, index});
        }
    }

    outFile.close();
}
