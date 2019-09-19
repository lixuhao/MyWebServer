// @Author Lixuhao
// @Email xxx@qq.com


#pragma once
#include "EventLoop.h"
#include "Channel.h"
#include "EventLoopThreadPool.h"
#include <memory>


class Server
{
public:
    Server(EventLoop *loop, int threadNum, int port);
    ~Server() { }


    EventLoop* getLoop() const { return loop_; }
    void start();
    void handNewConn();//处理新连接
    void handThisConn() { loop_->updatePoller(acceptChannel_); }//处理当前连接

private:
    EventLoop *loop_;/* TcpServer所在的主线程下运行的事件驱动循环，负责监听Acceptor的Channel */
    int threadNum_;

    ///* 事件驱动线程池，池中每个线程运行一个EventLoop */
    std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_;

    bool started_;

    //
    std::shared_ptr<Channel> acceptChannel_;
    int port_;
    int listenFd_;


    static const int MAXFDS = 100000;
};
