#ifndef THREAD_POOL_HPP
#define THREAD_POOL_HPP
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
namespace MiniTP {
class ThreadPool
{
  private:
    mutable std::mutex _mutex;
    std::condition_variable _cv;

    std::queue<std::function<void()>> _task_queue;
    std::vector<std::thread> _workers;
    std::atomic<bool> _running;

    std::atomic<int> _pending;

    void thread_func();

    class PendingGuard
    {
      private:
        std::atomic<int>& _pending;
        std::condition_variable& _cv;
        std::mutex& _mutex;

      public:
        PendingGuard(std::atomic<int>& pending, std::condition_variable& cv, std::mutex& mutex)
            : _pending(pending), _cv(cv), _mutex(mutex)
        {
        }
        PendingGuard(const PendingGuard&) = delete;
        PendingGuard(PendingGuard&&) = delete;
        PendingGuard& operator=(const PendingGuard&) = delete;
        PendingGuard& operator=(PendingGuard&&) = delete;
        ~PendingGuard()
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _pending--;
            if (_pending == 0) {
                _cv.notify_all();
            }
        }
    };

  public:
    ThreadPool(int num_threads);
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    ~ThreadPool();

    template <typename F, typename... Args>
    void run(F&& f, Args&&... args)
    {
        if (!_running) {
            throw std::runtime_error("ThreadPool is not running");
        }
        std::function<void()> task = [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() { f(args...); };
        std::lock_guard<std::mutex> lock(_mutex);
        _task_queue.push(std::move(task));
        _pending++;
        _cv.notify_one();
    }

    template <typename ReturnType, typename F, typename... Args>
    std::future<ReturnType> runWithReturn(F&& f, Args&&... args)
    {
        if (!_running) {
            throw std::runtime_error("ThreadPool is not running");
        }
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            [f = std::forward<F>(f), ... args = std::forward<Args>(args)]() { return f(args...); });
        std::future<ReturnType> future = task->get_future();
        std::lock_guard<std::mutex> lock(_mutex);
        _task_queue.push([task]() { (*task)(); });
        _pending++;
        _cv.notify_one();
        return future;
    }

    void waitAll();
};
} // namespace MiniTP
#endif // THREAD_POOL_HPP