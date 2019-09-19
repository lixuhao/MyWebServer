// @Author Lixuhao
// @Email xxx@qq.com


#pragma once
#include "base/noncopyable.h"
#include "EventLoopThread.h"
#include "base/Logging.h"
#include <memory>
#include <vector>

//EventLoopThreadPool是事件循环驱动池.这个池子既是一个线程池，又是一个EventLoop池,一个EventLoop对应一个线程 
//这种方式称为one loop per thread即reactor + 线程池
class EventLoopThreadPool : noncopyable
{
public:
    EventLoopThreadPool(EventLoop* baseLoop, int numThreads);

    ~EventLoopThreadPool()
    {
        LOG << "~EventLoopThreadPool()";
    }

    ///* 开启线程池，创建线程 */
    void start();


    /* 获取一个线程（事件驱动循环），通常在创建TcpConnection时调用 */
    EventLoop *getNextLoop();

private:

    /* 主线程的事件驱动循环，Server所在的事件驱动循环(即Server所在的那个主线程)，这个事件驱动循环通常只负责监听客户端
     * 连接请求，即Acceptor的Channel，创建Server传入的EventLoop 
    */
    EventLoop* baseLoop_;
    bool started_;

    /* 线程数 */
    int numThreads_;

    /* 标记下次应该取出哪个线程，采用round_robin */
    int next_;

    /*
    两个vector保存着所有子线程即每个子线程对应的EventLoop。事件驱动循环线程被封装在EventLoopThread中，EventLoopThread
    中使用的Thread才是真正的线程封装
    */


    /* 线程池中所有的线程 */
    std::vector<std::shared_ptr<EventLoopThread>> threads_;

  /* 
    * 线程池中每个线程对应的事件驱动循环，从线程池取出线程实际上返回的是事件驱动循环
    * 每个事件驱动循环运行在一个线程中
  */
    std::vector<EventLoop*> loops_;
};
