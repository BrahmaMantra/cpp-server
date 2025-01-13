#ifndef THREAD_WORKER_H
#define THREAD_WORKER_H

#include "threadPool.h"

class ThreadPool;  // 前向声明ThreadPool类

class ThreadWorker {
   private:
    int m_id;            // 工作id
    ThreadPool *m_pool;  // 所属线程池

   public:
    ThreadWorker(ThreadPool *pool, int id);  // 构造函数
    void operator()();                       // 重载()操作符
};

#endif  // THREAD_WORKER_H
