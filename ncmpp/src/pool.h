/**
 * @file pool.h
 * @brief Thread pool implementation for concurrent NCM file processing
 * @details Provides a simple thread pool for parallel processing of NCM files
 * @note Uses C++11 features for cross-platform compatibility
 */

#pragma once
#include <vector>
#include <functional>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

/**
 * @brief Simple thread pool for concurrent task execution
 * @details Implements a basic thread pool with:
 * - Dynamic thread count based on hardware concurrency
 * - Safe task enqueuing with future support
 * - Automatic cleanup in destructor
 * - Graceful handling of empty thread counts
 */
class thread_pool {
    std::vector<std::thread> threads;
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<std::function<void()>> tasks;
    bool stop = false;

public:
    /**
     * @brief Construct a thread pool with specified number of threads
     * @param n Number of threads to create (0 = use hardware concurrency)
     * @details Automatically adjusts for edge cases:
     * - If n=0, uses std::thread::hardware_concurrency()
     * - If hardware_concurrency() returns 0, defaults to 2 threads
     * - Creates worker threads that wait for tasks
     */
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

    /**
     * @brief Destructor cleans up all threads
     * @details Signals all threads to stop and joins them safely
     */
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

    /**
     * @brief Enqueue a task for execution in the thread pool
     * @tparam F Function type
     * @tparam Args Argument types
     * @param f Function to execute
     * @param args Arguments to pass to the function
     * @return std::future for retrieving the result
     * @throws std::runtime_error if trying to enqueue on stopped pool
     * @details Provides thread-safe task enqueuing with future support
     */
    template <typename F, typename... Args> 
    auto enqueue(F&& f, Args &&...args) -> std::future<std::invoke_result_t<F, Args...>> {
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