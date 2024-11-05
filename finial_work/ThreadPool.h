#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <future>
#include <iostream>

class ThreadPool {
public:
    ThreadPool(size_t threads);
    ~ThreadPool();

    template<typename F, typename... Args>
    auto enqueueTask(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            if (stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    void joinAll();

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::atomic<bool> stop;
    std::mutex queueMutex;
    std::condition_variable condition;

    void workerThread();
};

#endif // THREADPOOL_H
