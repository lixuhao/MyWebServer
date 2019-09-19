// @Author Lixuhao
// @Email xxx@qq.com
//
#include "EventLoop.h"
#include "base/Logging.h"
#include "Util.h"
#include <sys/eventfd.h>
#include <sys/epoll.h>
#include <iostream>
using namespace std;

__thread EventLoop* t_loopInThisThread = 0;//这一行的意思是线程变量，在eventfd.h头文件里面  

int createEventfd()
{
    int evtfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);//eventfd() 创建一个 eventfd 对象，可以由用户空间应用程序实现事件等待/通知机制，或由内核通知用户空间应用程序事件。
    //该对象包含了由内核维护的无符号64位整数计数器 count 。使用参数 initval 初始化此计数器。这儿写为0就是count为0，而第二个参数指的是用以改变 eventfd 的行为。
    ////EFD_NONBLOCK (since Linux 2.6.27)，文件被设置成 O_NONBLOCK，执行 read / write 操作时，不会阻塞。EFD_CLOEXEC (since Linux 2.6.27)，
    //
    //eventfd是一种线程间通信机制。简单来说eventfd就是一个文件描述符，它引用了一个内核维护的eventfd object，是uint64_t类型，也就是8个字节，可以作为counter。支持read，write，
    //以及有关epoll等操作
    //文件被设置成 O_CLOEXEC，创建子进程 (fork) 时不继承父进程的文件描述符。“|”是or运算。
    if (evtfd < 0)
    {
        LOG << "Failed in eventfd";//向日志缓冲区打印
        abort();//终止进程
    }
    return evtfd;
}

//创建轮询器（Poller），创建用于传递消息的管道，初始化各个部分，然后进入一个无限循环，每一次循环中调用轮询器的轮询函数,（等待函数），等待事件发生，如果有事件发生，就依次调用套接字
//（或其他的文件描述符）的事件处理器处理各个事件，然后调用投递的回调函数；接着进入下一次循环。
EventLoop::EventLoop()
:   looping_(false),
    poller_(new Epoll()),
    wakeupFd_(createEventfd()),
    quit_(false),
    eventHandling_(false),
    callingPendingFunctors_(false),
    threadId_(CurrentThread::tid()),
    pwakeupChannel_(new Channel(this, wakeupFd_))
{
    if (t_loopInThisThread)
    {
        //LOG << "Another EventLoop " << t_loopInThisThread << " exists in this thread " << threadId_;
    }
    else
    {
        t_loopInThisThread = this;
    }
    //pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
    pwakeupChannel_->setReadHandler(bind(&EventLoop::handleRead, this));
    pwakeupChannel_->setConnHandler(bind(&EventLoop::handleConn, this));
    poller_->epoll_add(pwakeupChannel_, 0);
}

void EventLoop::handleConn()
{
    //poller_->epoll_mod(wakeupFd_, pwakeupChannel_, (EPOLLIN | EPOLLET | EPOLLONESHOT), 0);
    updatePoller(pwakeupChannel_, 0);
}


EventLoop::~EventLoop()
{
    //wakeupChannel_->disableAll();
    //wakeupChannel_->remove();
    close(wakeupFd_);
    t_loopInThisThread = NULL;
}

//使用eventfd唤醒
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = writen(wakeupFd_, (char*)(&one), sizeof one);
    if (n != sizeof one)
    {
        LOG<< "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

//
void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = readn(wakeupFd_, &one, sizeof one);
    if (n != sizeof one)
    {
        LOG << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
    //pwakeupChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
    pwakeupChannel_->setEvents(EPOLLIN | EPOLLET);
}

// EventLoop线程除了等待poll、执行poll返回的激活事件，
// 还可以处理一些其他任务，例如调用某一个回调函数，处理其他EventLoop对象的，
// 调用EventLoop::runInLoop(cb)即可让EventLoop线程执行cb函数。
// 假设我们有这样的调用：loop->runInLoop(run)，说明想让IO线程执行一定的计算任务，
// 此时若是在当前的IO线程，就马上执行run()；如果是其他线程调用的，那么就
// 执行queueInLoop(run),将run异步添加到队列，当loop内处理完事件后，
// 就执行doPendingFunctors()，也就执行到了run()；最后想要结束线程的话，执行quit。
void EventLoop::runInLoop(Functor&& cb)
{
  //// 如果当前线程是EventLoop线程则立即执行，否则放到任务队列中，异步执行
    if (isInLoopThread())
        cb();//cb是一个函数指针
    else
        queueInLoop(std::move(cb));
}

//任务队列
void EventLoop::queueInLoop(Functor&& cb)
{
    {
        MutexLockGuard lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));//pendingFunctors_是一个存储着函数指针的vector
    }

    //如果当前线程不是EventLoop线程，或者正在执行pendingFunctors_中的任务
    //都要唤醒EventLoop线程，让其执行pendingFunctors_中的任务。
    if (!isInLoopThread() || callingPendingFunctors_)
        wakeup();
}

//事件循环。在某线程中实例化EventLoop对象，这个线程就是IO线程，必须在IO线程中执行loop操作
//在当前IO线程中进行updateChannel，在当前线程中进行removeChannel。
void EventLoop::loop()
{
    assert(!looping_);
    assert(isInLoopThread());
    looping_ = true;
    quit_ = false;
    //LOG_TRACE << "EventLoop " << this << " start looping";
    std::vector<SP_Channel> ret;//定义一个shared_ptr<Channel>类型的vector变量ret
    while (!quit_)//如果没有结束线程
    {
        //cout << "doing" << endl;
        ret.clear();//则把ret清空
        ret = poller_->poll();//获得活跃事件数.poller是Epoll类型的share_ptr,而Epoll类型是我们//自定义的一个类，在这个类中有这个poll函数。
        eventHandling_ = true;//事件处理开始

        //遍历活跃的Channel.执行每一个channel上的回调函数
        for (auto &it : ret)
            it->handleEvents();
       
        eventHandling_ = false;//事件处理结束 
        
        doPendingFunctors(); // 处理用户在其他线程注册给IO线程的事件

        poller_->handleExpired();//处理过期事件
    }
    looping_ = false;
}

//执行任务队列中的任务
void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    {
        MutexLockGuard lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
        functors[i]();
    callingPendingFunctors_ = false;
}

//结束EventLoop线程
void EventLoop::quit()
{
    quit_ = true;
    if (!isInLoopThread())
    {
        wakeup();
    }
}
