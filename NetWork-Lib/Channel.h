// @Author Lixuhao
// @Email xxx@qq.com

#pragma once
#include "Timer.h"
#include <string>
#include <unordered_map>
#include <memory>
#include <sys/epoll.h>
#include <functional>
#include <sys/epoll.h>


class EventLoop;
class HttpData;


class Channel
{
private:
    typedef std::function<void()> CallBack;//这个是函数指针.用于事件回调
    EventLoop *loop_;//channel所属的loop
    int fd_;//channel负责的文件描述符
    __uint32_t events_;//注册的事件
    __uint32_t revents_;//poller设置的就绪的事件
    __uint32_t lastEvents_;//最后一次的事件

    // 方便找到上层持有该Channel的对象
    std::weak_ptr<HttpData> holder_;//定义一个HttpData类型的弱引用holder.

private:
    int parse_URI();
    int parse_Headers();
    int analysisRequest();

    //事件回调
    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;

public:
    Channel(EventLoop *loop);
    Channel(EventLoop *loop, int fd);
    ~Channel();
    int getFd();
    void setFd(int fd);

   //设置弱引用
    void setHolder(std::shared_ptr<HttpData> holder)
    {
        holder_ = holder;
    }
    //找到持有该Channel的对象
    std::shared_ptr<HttpData> getHolder()
    {
        std::shared_ptr<HttpData> ret(holder_.lock());
        return ret;
    }

    //注意下面的这几个回调函数的参数是右值引用,有的地方参数不是右值引用，是一般的参数，所以在函数体的时候会用std::move将其转换为右值引用。
    //设置读回调函数
    void setReadHandler(CallBack &&readHandler)
    {
        readHandler_ = readHandler;
    }

    //设置写回调函数
    void setWriteHandler(CallBack &&writeHandler)
    {
        writeHandler_ = writeHandler;
    }

    //设置错误回调函数
    void setErrorHandler(CallBack &&errorHandler)
    {
        errorHandler_ = errorHandler;
    }

    //设置连接回调函数
    void setConnHandler(CallBack &&connHandler)
    {
        connHandler_ = connHandler;
    }


    //根据就绪事件revents的值来分别调用不同的函数
    void handleEvents()//处理就绪事件
    {
        events_ = 0;
        if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN))//EPOLLHUP是挂断的意思。EPOLLIN是
          //可读取非高优先级的数据。总的来说就是对方的套接字关闭，EPOLLHUP与POLLHUP意义一样，
          //EPOLLIN与POLLIN意义一样。
        {
            events_ = 0;//注册事件置0
            return;
        }
        if (revents_ & EPOLLERR)//如果出现错误
        {
            if (errorHandler_) errorHandler_();//而且错误的回调函数非0，则调用这个错误的回
            //调函数
            events_ = 0;//注册事件置0
            return;
        }
        if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))//EPOLLIN可读取非高优先级的数据,
          //EPOLLPRI是可读取高优先级的数据,EPOLLRDHUP是写端出现挂断，这个总结也是对方的写端
          //关闭
        {
            handleRead();//处理写事件
        }
        if (revents_ & EPOLLOUT)
        {
            handleWrite();//处理读事件
        }
        handleConn();//两个都处理完了就处理连接事件。
    }
    void handleRead();
    void handleWrite();
    void handleError(int fd, int err_num, std::string short_msg);//这个是多余的,因为这个在handleEvents里是没有直接调用的，而是在里直接写了发生错误时的解决方法。而且这个函数在Channel.cpp
     //也没有实现
    void handleConn();

    void setRevents(__uint32_t ev)//设置就绪事件，used by poller  renents的r是ready的意思，即就绪
    {
        revents_ = ev;
    }

    void setEvents(__uint32_t ev)//设置注册事件
    {
        events_ = ev;
    }
    __uint32_t& getEvents()//获取注册事件
    {
        return events_;
    }

    bool EqualAndUpdateLastEvents()//与最近一次更新的事件是否相等
    {
        bool ret = (lastEvents_ == events_);//相等则ret为true
        lastEvents_ = events_;
        return ret;
    }

    __uint32_t getLastEvents()//获取最近的一次事件
    {
        return lastEvents_;
    }

};

typedef std::shared_ptr<Channel> SP_Channel;//这个是将Channel类型的智能指针重定义为SP_Channel
