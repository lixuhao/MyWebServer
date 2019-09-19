// @Author Lixuhao
// @Email xxx@qq.com


#pragma once
#include "Channel.h"
#include "HttpData.h"
#include "Timer.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>
#include <memory>

class Epoll
{
public:
    Epoll();
    ~Epoll();

    //epoll的三个操作
    void epoll_add(SP_Channel request, int timeout);
    void epoll_mod(SP_Channel request, int timeout);
    void epoll_del(SP_Channel request);

    //
    std::vector<std::shared_ptr<Channel>> poll();//这儿的poll是存储Channel类型的智能指针的
    //一个数组
    std::vector<std::shared_ptr<Channel>> getEventsRequest(int events_num);//获取事件请求
    void add_timer(std::shared_ptr<Channel> request_data, int timeout);//

    //获得epoll对象的fd
    int getEpollFd()
    {
        return epollFd_;
    }
    void handleExpired();
private:
    static const int MAXFDS = 100000;
    int epollFd_;
    std::vector<epoll_event> events_;
    std::shared_ptr<Channel> fd2chan_[MAXFDS];
    std::shared_ptr<HttpData> fd2http_[MAXFDS];
    TimerManager timerManager_;
};
