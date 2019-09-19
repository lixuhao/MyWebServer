// @Author Lixuhao
// @Email xxx@qq.com
#include "EventLoopThread.h"
#include <functional>


EventLoopThread::EventLoopThread()
:   loop_(NULL),
    exiting_(false),
    thread_(bind(&EventLoopThread::threadFunc, this), "EventLoopThread"),
    mutex_(),
    cond_(mutex_)
{ }

EventLoopThread::~EventLoopThread()
{
    exiting_ = true;
    if (loop_ != NULL)//如果EventLoop对象不为空，
    {
        loop_->quit();//则结束这个EventLoop线程，这个quit函数在EventLoop.cpp文件里
        //退出I/O线程，让I/O线程的loop循环退出，从而退出了I/O线程

        thread_.join();//因为默认创建的线程是joinable的，线程退出的时候，需要对其进行pthread_join操作,否则无法释放资源
        //调用join函数(这个join函数是封装了pthread_join),让这个线程成为一个可以被靠近的线程，这样线程资源就可以释放了
    }
}

//启动EventLoopThread中的loop循环，内部实际调用thread_.start,这个函数是启动EventLoopThread类所包含的线程thread_，
//可以理解为启动worker线程，启动了之后该函数一直因condition不满足等待这里，该函数的作用是启动EventLoopThread中的线程
//去启动EventLoop，然后把EventLoop对象的指针返给调用者，由于启动线程，它和thread_的worker线程threadFunc是并发执行的，
//所以它需要等待loop产生成功才能返回loop的指针。loop是I/O线程最重要的循环不必多说，它是事件处理的核心
EventLoop* EventLoopThread::startLoop()
{
    assert(!thread_.started());
    thread_.start();
    {
        MutexLockGuard lock(mutex_);
        // 一直等到threadFun在Thread里真正跑起来
        while (loop_ == NULL)
            cond_.wait();
    }
    return loop_;
}

//线程start之后，就要启动loop循环
//该函数是EventLoopThread类的核心函数，作用是启动loop循环
//该函数和上面的startLoop函数并发执行，所以需要上锁和condition
void EventLoopThread::threadFunc()
{
    EventLoop loop;

    {
        MutexLockGuard lock(mutex_);
        loop_ = &loop;//loop_指针指向了这个创建的栈上的对象，threadFunc退出之后，这个指针就失效了
        cond_.notify();////该函数退出，意味着线程就推出了，EventLoopThread对象也就没有存在的价值了。EventLoopThread实现///为自动销毁,一般loop函数退出整个程序就退出了，因而不会有什么大的问题，
        //因为这儿的线程池就是启动时分配，并没有释放。所以线程结束一般来说就是整个程序结束了
    }

    loop.loop();//开始loop循环
    //assert(exiting_);
    loop_ = NULL;
}
