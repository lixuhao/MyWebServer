// @Author Lixuhao
// @Email xxx@qq.com


#pragma once
#include "base/Condition.h"
#include "base/MutexLock.h"
#include "base/Thread.h"
#include "base/noncopyable.h"
#include "EventLoop.h"

class EventLoopThread :noncopyable
{
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoop* startLoop();//启动成员thread_线程，该线程就成了I/O线程，内部调用thread_.start()

private:
    void threadFunc();//线程运行函数
    EventLoop *loop_;//指向一个EventLoop对象，一个I/O线程有且只有一个EventLoop对象
    bool exiting_;
    Thread thread_;//基于对象，包含了一个thread类对象
    MutexLock mutex_;
    Condition cond_;
};
