#pragma once
#include <vector>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

class thread_pool {
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks;
    bool stop = false;

public:
    thread_pool(unsigned int n) {
        if (n == 0) {
            n = std::thread::hardware_concurrency();
        }
        if (n == 0) { // hardware_concurrency might return 0
            n = 2;
        }
        for (auto i = 0u; i < n; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mtx);
                        cv.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty()) {
                            return;
                        }
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~thread_pool() {
        {
            std::unique_lock<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
        for (auto& i : threads) {
            if (i.joinable()) {
                i.join();
            }
        }
    }

    template <typename F, typename... Args> auto enqueue(F&& f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
        using return_type = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto ret = task->get_future();
        {
            std::unique_lock<std::mutex> lock(mtx);
            if(stop) throw std::runtime_error("enqueue on stopped thread_pool");
            tasks.emplace([task] { (*task)(); });
        }
        cv.notify_one();
        return ret;
    }
};