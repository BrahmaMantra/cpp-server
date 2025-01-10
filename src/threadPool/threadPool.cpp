#include "threadPool.h"
#include "worker.h"
#include <functional>
#include <memory>

ThreadPool::ThreadPool(int n_threads)
    :  m_shutdown(false),m_threads(std::vector<std::thread>(n_threads))
{
}

ThreadPool::~ThreadPool()
{
    shutdown(); // 确保线程池在销毁时关闭
}

void ThreadPool::init()
{
    for (size_t i = 0; i < m_threads.size(); ++i) // 使用 size_t 替代 int
    {
        m_threads.at(i) = std::thread(ThreadWorker(this, i)); // 分配工作线程
    }
}

void ThreadPool::shutdown()
{
    m_shutdown = true;
    m_conditional_lock.notify_all(); // 通知所有线程

    for (size_t i = 0; i < m_threads.size(); ++i) // 使用 size_t 替代 int
    {
        if (m_threads.at(i).joinable())
        {
            m_threads.at(i).join(); // 等待线程结束
        }
    }
}

// template <typename F, typename... Args>
// auto ThreadPool::submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))>
// {
//     std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

//     auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
//     std::function<void()> wrapped_func = [task_ptr]() { (*task_ptr)(); };

//     m_queue.enqueue(wrapped_func); // 将任务添加到队列中
//     m_conditional_lock.notify_one(); // 唤醒一个线程

//     return task_ptr->get_future(); // 返回任务的future
// }

