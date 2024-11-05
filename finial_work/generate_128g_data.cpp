#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <atomic>

namespace fs = std::filesystem;

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();
    void enqueueTask(std::function<void()> task);

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    void workerThread();
};

ThreadPool::ThreadPool(size_t numThreads) : stop(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { workerThread(); });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop = true;
    }
    condition.notify_all();
    for (std::thread &worker : workers) {
        worker.join();
    }
}

void ThreadPool::enqueueTask(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        tasks.push(task);
    }
    condition.notify_one();
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stop || !tasks.empty(); });
            if (stop && tasks.empty())
                return;
            task = std::move(tasks.front());
            tasks.pop();
        }
        task();
    }
}

void sortAndWriteFile(const std::string &inputFilePath, const std::string &outputFilePath) {
    std::ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening input file: " << inputFilePath << std::endl;
        return;
    }

    std::vector<int64_t> data;
    int64_t value;
    while (inputFile >> value) {
        data.push_back(value);
    }
    inputFile.close();

    std::sort(data.begin(), data.end());

    std::ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file: " << outputFilePath << std::endl;
        return;
    }

    for (const auto &val : data) {
        outputFile << val << "\n";
    }
    outputFile.close();
}

void mergeFiles(const std::vector<std::string> &inputFiles, const std::string &outputFilePath) {
    std::priority_queue<std::pair<int64_t, std::ifstream*>, std::vector<std::pair<int64_t, std::ifstream*>>, std::greater<>> minHeap;
    std::vector<std::ifstream> inputStreams;

    for (const auto &file : inputFiles) {
        inputStreams.emplace_back(file);
        if (inputStreams.back().is_open()) {
            int64_t value;
            if (inputStreams.back() >> value) {
                minHeap.emplace(value, &inputStreams.back());
            }
        }
    }

    std::ofstream outputFile(outputFilePath);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening output file: " << outputFilePath << std::endl;
        return;
    }

    while (!minHeap.empty()) {
        auto [val, stream] = minHeap.top();
        minHeap.pop();
        outputFile << val << "\n";

        if (*stream >> val) {
            minHeap.emplace(val, stream);
        }
    }

    outputFile.close();
}

int main() {
    const std::string directoryPath = "/mnt/hgfs/LinuxClass_TestDir";
    const std::string outputFilePath = "sorted_output.txt";
    const size_t numThreads = std::thread::hardware_concurrency();

    ThreadPool threadPool(numThreads);
    std::vector<std::string> sortedFilePaths;
    std::mutex sortedFilesMutex;

    for (const auto &entry : fs::directory_iterator(directoryPath)) {
        if (entry.is_regular_file()) {
            std::string inputFilePath = entry.path().string();
            std::string outputFilePath = inputFilePath + ".sorted";

            threadPool.enqueueTask([inputFilePath, outputFilePath, &sortedFilePaths, &sortedFilesMutex] {
                sortAndWriteFile(inputFilePath, outputFilePath);
                {
                    std::lock_guard<std::mutex> lock(sortedFilesMutex);
                    sortedFilePaths.push_back(outputFilePath);
                }
            });
        }
    }

    // Wait for all sorting tasks to complete by destroying the thread pool
    // (This will wait for all threads to finish their tasks)
    {
        ThreadPool tempPool(0); // Temporary object to ensure all tasks complete
    }

    // Merge all sorted files into the final output file
    mergeFiles(sortedFilePaths, outputFilePath);

    std::cout << "All data has been sorted and merged into " << outputFilePath << std::endl;

    return 0;
}
