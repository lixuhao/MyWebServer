// @Author Lixuhao
// @Email xxx@qq.com


// This file has not been used
#include "ThreadPool.h"

pthread_mutex_t ThreadPool::lock = PTHREAD_MUTEX_INITIALIZER;//PTHREAD_MUTEX_INITIALIZER是posix定义的
//一个宏，用来静态初始化互斥锁。
pthread_cond_t ThreadPool::notify = PTHREAD_COND_INITIALIZER;//条件变量和互斥锁一样，都有静态创建和动态创建两种，
//PTHREAD_COND_INITIALIZER就是静态创建条件变量
std::vector<pthread_t> ThreadPool::threads;//线程池中的线程是存储在一个pthread_t类型的数组里
std::vector<ThreadPoolTask> ThreadPool::queue;//工作线程存储在线程池中的任务队列里

//初始化各个变量
int ThreadPool::thread_count = 0;
int ThreadPool::queue_size = 0;
int ThreadPool::head = 0;
int ThreadPool::tail = 0;
int ThreadPool::count = 0;
int ThreadPool::shutdown = 0;
int ThreadPool::started = 0;


int ThreadPool::threadpool_create(int _thread_count, int _queue_size)
{
    bool err = false;
    do
    {  //如果线程池中的数量小于0或者大于线程池的最大线程数 又或者任务队列的线程数小于0或者大于任务队列的线程数。
        if(_thread_count <= 0 || _thread_count > MAX_THREADS || _queue_size <= 0 || _queue_size > MAX_QUEUE) 
        {
            _thread_count = 4;//那么就给线程池中的线程数量赋初始值为4
            _queue_size = 1024;//任务队列的初始值为1024
        }
    
        thread_count = 0;
        queue_size = _queue_size;
        head = tail = count = 0;
        shutdown = started = 0;

        threads.resize(_thread_count);//threads是一个vector类型，resize就是重置vector的capacity
        queue.resize(_queue_size);//同上一句
    
        /* Start worker threads */
        for(int i = 0; i < _thread_count; ++i) 
        {
          //如果线程创建不等于0，说明线程创建失败，则返回-1， 这儿填的第一个参数是存储在
          //vector中的第一个线程的地址，第二个参数是线程属性，一般写为NULL，由系统赋予
          //第三个参数是函数指针，是线程的启动函数，在ThreadPoll.h中有定义。
          //第4个参数是传给第三个参数的启动函数的参数。指定线程将要加载调用的那个函数的参数
            if(pthread_create(&threads[i], NULL, threadpool_thread, (void*)(0)) != 0) 
            {
                //threadpool_destroy(pool, 0);
                return -1;
            }
            ++thread_count;//到这儿说明创建线程成功，则线程数加1
            ++started;//则开始的线程加1
        }
    } while(false);//do while语句，而且这个while的判断语句是false，所以是上面do的循环体循环完//就不执行了
    
    if (err) //如果是err是true则return。这个不知道是什么作用
    {
        //threadpool_free(pool);
        return -1;
    }
    return 0;
}


////增加一个任务到线程池
int ThreadPool::threadpool_add(std::shared_ptr<void> args, std::function<void(std::shared_ptr<void>)> fun)
{
    int next, err = 0;
    if(pthread_mutex_lock(&lock) != 0)
        return THREADPOOL_LOCK_FAILURE;
    do 
    {
        next = (tail + 1) % queue_size;
        // 队列满
        if(count == queue_size) 
        {
            err = THREADPOOL_QUEUE_FULL;
            break;
        }
        // 已关闭
        if(shutdown)
        {
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        queue[tail].fun = fun;//queue是存储着ThreadPoolTask(线程池任务)类型的vector容器，所以queue的最后一个元素
        //是ThreadPoolTask类型的，而fun是ThreadPoolTask类型的成员，ThreadPoolTask是一个结构体，在ThreadPool.h的头文件里         //有定义，包含两个成员，一个是函数指针fun，一个是参数。
        queue[tail].args = args;//这个就是ThreadPoolTask类型的第二个成员：参数
        tail = next;//tail向后走
        ++count;//任务对列的count加加向后走
        
        /* pthread_cond_broadcast */
        if(pthread_cond_signal(&notify) != 0) //唤醒一个等待该条件的线程，多个线程阻塞在此条件变量上时，
          //哪一个线程被唤醒是由线程的调度策略所决定的,这个是如果唤醒函数的返回值是非0，说明唤醒失败
        {
            err = THREADPOOL_LOCK_FAILURE;//也是返回加锁错误
            break;
        }
    } while(false);

    if(pthread_mutex_unlock(&lock) != 0)
        err = THREADPOOL_LOCK_FAILURE;
    return err;
}

//销毁线程池中的线程
int ThreadPool::threadpool_destroy(ShutDownOption shutdown_option)
{
    printf("Thread pool destroy !\n");
    int i, err = 0;

    if(pthread_mutex_lock(&lock) != 0) 
    {
        return THREADPOOL_LOCK_FAILURE;
    }
    do 
    {
        if(shutdown) {
            err = THREADPOOL_SHUTDOWN;
            break;
        }
        shutdown = shutdown_option;

        if((pthread_cond_broadcast(&notify) != 0) ||
           (pthread_mutex_unlock(&lock) != 0)) {
            err = THREADPOOL_LOCK_FAILURE;
            break;
        }

        for(i = 0; i < thread_count; ++i)
        {
            if(pthread_join(threads[i], NULL) != 0)
            {
                err = THREADPOOL_THREAD_FAILURE;
            }
        }
    } while(false);

    if(!err) 
    {
        threadpool_free();
    }
    return err;
}

//释放
int ThreadPool::threadpool_free()
{
    if(started > 0)
        return -1;
    pthread_mutex_lock(&lock);
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&notify);
    return 0;
}

//创建线程的函数入口。也就是创建线程函数的第三个参数
void *ThreadPool::threadpool_thread(void *args)
{
    while (true)
    {
        ThreadPoolTask task;
        pthread_mutex_lock(&lock);
        while((count == 0) && (!shutdown)) 
        {
            pthread_cond_wait(&notify, &lock);
        }
        if((shutdown == immediate_shutdown) ||
           ((shutdown == graceful_shutdown) && (count == 0)))
        {
            break;
        }
        task.fun = queue[head].fun;
        task.args = queue[head].args;
        queue[head].fun = NULL;
        queue[head].args.reset();
        head = (head + 1) % queue_size;
        --count;
        pthread_mutex_unlock(&lock);
        (task.fun)(task.args);
    }
    --started;
    pthread_mutex_unlock(&lock);
    printf("This threadpool thread finishs!\n");
    pthread_exit(NULL);
    return(NULL);
}
