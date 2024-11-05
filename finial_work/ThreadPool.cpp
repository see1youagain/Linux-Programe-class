#include "ThreadPool.h"
#include <iostream>

ThreadPool::ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this, i] { 
            std::cout << "Worker thread " << i << " started." << std::endl;
            workerThread(); 
            std::cout << "Worker thread " << i << " finished." << std::endl;
        });
    }
}

ThreadPool::~ThreadPool() {
    stop.store(true);
    condition.notify_all();
    joinAll(); // Ensure all threads are joined on destruction.
}

void ThreadPool::workerThread() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this] {
                return stop.load() || !tasks.empty();
            });

            if (stop.load() && tasks.empty()) {
                // std::cout << "Stopping worker thread " << std::this_thread::get_id() << std::endl;
                return;
            }

            if (!tasks.empty()) {
                task = std::move(tasks.front());
                tasks.pop();
            }
        }

        if (task) {
            // std::cout << "Executing task by thread " << std::this_thread::get_id() << std::endl;
            task();
            // std::cout << "Task completed by thread " << std::this_thread::get_id() << std::endl;
        }
    }
}



void ThreadPool::joinAll() {
    {
        std::unique_lock<std::mutex> lock(queueMutex);
        stop.store(true);
    }
    condition.notify_all();  // 通知所有线程停止并处理剩余的任务
    std::cout << "Joining all threads." << std::endl;
    
    for (size_t i = 0; i < workers.size(); ++i) {
        if (workers[i].joinable()) {
            std::cout << "Joining worker thread " << i << "." << std::endl;
            workers[i].join();
            std::cout << "Worker thread " << i << " joined." << std::endl;
        }
    }
    std::cout << "All threads joined." << std::endl;
}
