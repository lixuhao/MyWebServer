// @Author Lixuhao
// @Email xxx@qq.com

// This file has not been used

#pragma once
#include "Channel.h"
#include <pthread.h>
#include <functional>
#include <memory>
#include <vector>

const int THREADPOOL_INVALID = -1;//线程池中失效的线程
const int THREADPOOL_LOCK_FAILURE = -2;//加锁失败
const int THREADPOOL_QUEUE_FULL = -3;//满队列
const int THREADPOOL_SHUTDOWN = -4;//非优雅关闭
const int THREADPOOL_THREAD_FAILURE = -5;//线程池读失败
const int THREADPOOL_GRACEFUL = 1;//GRACEFUL是得体，优雅的意思，所以这个是得体关闭的意思,
//

const int MAX_THREADS = 1024;//线程池中的线程数为1024
const int MAX_QUEUE = 65535;//任务队列中的最大值是65535.

typedef enum
{
    immediate_shutdown = 1,
    graceful_shutdown  = 2
} ShutDownOption;

struct ThreadPoolTask//线程池中的对象需执行的任务对象
{
    std::function<void(std::shared_ptr<void>)> fun;//函数指针
    std::shared_ptr<void> args;//参数
};


class ThreadPool
{
private:
    static pthread_mutex_t lock;
    static pthread_cond_t notify;

    static std::vector<pthread_t> threads;
    static std::vector<ThreadPoolTask> queue;
    static int thread_count;
    static int queue_size;
    static int head;
    // tail 指向尾节点的下一节点
    static int tail;
    static int count;
    static int shutdown;
    static int started;
public:
    static int threadpool_create(int _thread_count, int _queue_size);//创建线程池
    static int threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun);//给线程
    //池中添加任务
    static int threadpool_destroy(ShutDownOption shutdown_option = graceful_shutdown);//摧毁线程池
    static int threadpool_free();
    static void *threadpool_thread(void *args);//一个函数指针，在创建线程的时候，充当创建线程的第三个参数
};
