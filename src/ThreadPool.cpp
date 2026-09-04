#include "MiniTP/ThreadPool.hpp"
#include <exception>
#include <iostream>
#include <mutex>
#include <stdexcept>
namespace MiniTP {
ThreadPool::ThreadPool(int num_threads) : _running(true), _pending(0)
{
    if (num_threads <= 0) {
        throw std::runtime_error("num_threads should not less than 0");
    }
    _workers.reserve(num_threads);
    for (int i = 0; i < num_threads; i++) {
        _workers.emplace_back(&ThreadPool::thread_func, this);
    }
}

ThreadPool::~ThreadPool()
{
    _running = false;
    _cv.notify_all();
    for (auto& worker : _workers) {
        worker.join();
    }
}

void ThreadPool::thread_func()
{
    while (true) {
        std::function<void()> task;
        {
            auto lock = std::unique_lock<std::mutex>(_mutex);
            _cv.wait(lock, [this]() { return !_task_queue.empty() || !_running; });
            if (!_running && _task_queue.empty()) {
                break;
            }
            task = _task_queue.front();
            _task_queue.pop();
        }
        try {
            PendingGuard guard(_pending, _cv, _mutex);
            task();
        }
        catch (const std::exception& e) {
            std::cerr << "Exception in thread pool task: " << e.what() << std::endl;
        }
        catch (...) {
            std::cerr << "Unknown exception in thread pool task" << std::endl;
        }
    }
}

void ThreadPool::waitAll()
{
    std::unique_lock<std::mutex> lock(_mutex);
    _cv.wait(lock, [this]() { return _pending == 0; });
}
} // namespace MiniTP