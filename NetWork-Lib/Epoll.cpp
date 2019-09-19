// @Author Lixuhao
// @Email xxx@qq.com


#include "Epoll.h"
#include "Util.h"
#include "base/Logging.h"
#include <sys/epoll.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <queue>
#include <deque>
#include <assert.h>
#include <arpa/inet.h>
#include <iostream>
using namespace std;


const int EVENTSNUM = 4096;
const int EPOLLWAIT_TIME = 10000;

typedef shared_ptr<Channel> SP_Channel;//这个在Channel.h最后一句也有定义

Epoll::Epoll():
    epollFd_(epoll_create1(EPOLL_CLOEXEC)),//自定义的Epoll类的epollFd_变量用epoll_create1
    //这个函数，这个epoll_create1是创建的epoll的另一种方式，
    //原型是int epoll_create1(int flags); epoll_create1，如果flags参数为0，则除了
    //省略了size参数之外，它与epoll_create是相同的，如果flags参数不为0，
    //则目前它只能是EPOLL_CLOEXEC，用于设置该描述符的close-on-exec(FD_CLOEXEC)标志。
    //成功时返回非负的文件描述符，失败时返回-1.
    events_(EVENTSNUM)//这个是注册事件的最大数目
{
    assert(epollFd_ > 0);
}
Epoll::~Epoll()
{ }


// 注册新描述符
void Epoll::epoll_add(SP_Channel request, int timeout)
{
    int fd = request->getFd();
    if (timeout > 0)
    {
        add_timer(request, timeout);
        fd2http_[fd] = request->getHolder();
    }
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getEvents();

    request->EqualAndUpdateLastEvents();

    fd2chan_[fd] = request;
    if(epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event) < 0)
    {
        perror("epoll_add error");
        fd2chan_[fd].reset();
    }
}


// 修改描述符状态
void Epoll::epoll_mod(SP_Channel request, int timeout)
{
    if (timeout > 0)
        add_timer(request, timeout);
    int fd = request->getFd();
    if (!request->EqualAndUpdateLastEvents())
    {
        struct epoll_event event;
        event.data.fd = fd;
        event.events = request->getEvents();
        if(epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event) < 0)
        {
            perror("epoll_mod error");
            fd2chan_[fd].reset();
        }
    }
}


// 从epoll中删除描述符
void Epoll::epoll_del(SP_Channel request)
{
    int fd = request->getFd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->getLastEvents();
    //event.events = 0;
    // request->EqualAndUpdateLastEvents()
    if(epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event) < 0)
    {
        perror("epoll_del error");
    }
    fd2chan_[fd].reset();
    fd2http_[fd].reset();
}




// 返回活跃事件数
std::vector<SP_Channel> Epoll::poll()
{
    while (true)
    {
        int event_count = epoll_wait(epollFd_, &*events_.begin(), events_.size(), EPOLLWAIT_TIME);
        if (event_count < 0)
            perror("epoll wait error");
        std::vector<SP_Channel> req_data = getEventsRequest(event_count);
        if (req_data.size() > 0)
            return req_data;
    }
}

void Epoll::handleExpired()
{
    timerManager_.handleExpiredEvent();
}

// 分发处理函数
std::vector<SP_Channel> Epoll::getEventsRequest(int events_num)
{
    std::vector<SP_Channel> req_data;
    for(int i = 0; i < events_num; ++i)
    {
        // 获取有事件产生的描述符
        int fd = events_[i].data.fd;

        SP_Channel cur_req = fd2chan_[fd];
            
        if (cur_req)
        {
            cur_req->setRevents(events_[i].events);
            cur_req->setEvents(0);
            // 加入线程池之前将Timer和request分离
            //cur_req->seperateTimer();
            req_data.push_back(cur_req);
        }
        else
        {
            LOG << "SP cur_req is invalid";
        }
    }
    return req_data;
}

void Epoll::add_timer(SP_Channel request_data, int timeout)
{
    shared_ptr<HttpData> t = request_data->getHolder();
    if (t)
        timerManager_.addTimer(t, timeout);
    else
        LOG << "timer add fail";
}
