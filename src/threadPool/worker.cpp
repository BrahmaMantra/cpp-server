#include "worker.h"
#include <functional>
#include <mutex>
#include <condition_variable>

ThreadWorker::ThreadWorker(ThreadPool *pool, int id)
    : m_id(id),m_pool(pool)
{
}

void ThreadWorker::operator()()
{
    std::function<void()> func; // 定义基础函数类
    bool dequeued; // 是否正在取出队列中元素

    while (!m_pool->m_shutdown)
    {
        {
            std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
            if (m_pool->m_queue.is_empty())
            {
                m_pool->m_conditional_lock.wait(lock); // 等待条件变量通知，开启线程
            }
            dequeued = m_pool->m_queue.dequeue(func); // 取出任务
        }

        if (dequeued)
        {
            func(); // 执行任务
        }
    }
}
