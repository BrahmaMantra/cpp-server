#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <thread>
#include <vector>

#include "safeQueue.h"
#include "worker.h"

class ThreadWorker;  // 前向声明ThreadWorker

class ThreadPool {
    friend class ThreadWorker;

   private:
    bool m_shutdown;                           // 线程池是否关闭
    SafeQueue<std::function<void()>> m_queue;  // 执行函数安全队列，即任务队列
    std::vector<std::thread> m_threads;          // 工作线程队列
    std::mutex m_conditional_mutex;              // 线程休眠锁
    std::condition_variable m_conditional_lock;  // 线程环境锁

   public:
    ThreadPool(int n_threads = 4);  // 构造函数
    ~ThreadPool();                  // 析构函数

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool(ThreadPool &&) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;
    ThreadPool &operator=(ThreadPool &&) = delete;

    void init();      // 初始化线程池
    void shutdown();  // 关闭线程池

    // 提交任务
    template <typename F, typename... Args>
    auto submit(F &&f, Args &&...args)
        -> std::future<decltype(f(args...))> {
        std::function<decltype(f(args...))()> func =
            std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        auto task_ptr =
            std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
        std::function<void()> wrapped_func = [task_ptr]() { (*task_ptr)(); };

        m_queue.enqueue(wrapped_func);    // 将任务添加到队列中
        m_conditional_lock.notify_one();  // 唤醒一个线程

        return task_ptr->get_future();  // 返回任务的future
    }

    // 提供访问方法
    bool is_shutdown() const { return m_shutdown; }
    SafeQueue<std::function<void()>> &get_queue() { return m_queue; }
    std::mutex &get_mutex() { return m_conditional_mutex; }
    std::condition_variable &get_conditional_lock() {
        return m_conditional_lock;
    }
};

#endif  // THREAD_POOL_H
