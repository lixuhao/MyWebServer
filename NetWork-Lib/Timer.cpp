// @Author Lixuhao
// @Email xxx@qq.com
#include "Timer.h"
#include <sys/time.h>
#include <unistd.h>
#include <queue>


TimerNode::TimerNode(std::shared_ptr<HttpData> requestData, int timeout)
:   deleted_(false),
    SPHttpData(requestData)
{
    struct timeval now;//定义timeval结构体变量 now
    gettimeofday(&now, NULL);//gettimeofday()功能是两个参数，第一个参数是tv，第二个是tz，得到当前时间和时区，分别写到tv和tz中，如果tz为NULL则不向tz写入。而这儿的tz就是NULL
    // 以毫秒计，expiredTime_是过期时间的意思。
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;//这个是将struct timeval里的tv_sec和tv_usec以毫秒计.
}

TimerNode::~TimerNode()
{
    if (SPHttpData)
        SPHttpData->handleClose();
}

TimerNode::TimerNode(TimerNode &tn):
    SPHttpData(tn.SPHttpData)//在Timer头文件里有std::shared_ptr<HttpData> SPHttpData
{ }

//更新时间
void TimerNode::update(int timeout)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    expiredTime_ = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000)) + timeout;
}

//时间是有效的吗
bool TimerNode::isValid()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    size_t temp = (((now.tv_sec % 10000) * 1000) + (now.tv_usec / 1000));
    if (temp < expiredTime_)
        return true;
    else
    {
        this->setDeleted();//大于返回过期时间，则调用void setDeleted() { deleted_ = true;  }，这个函数是被定义在头文件里.
        return false;
    }
}

void TimerNode::clearReq()
{
    SPHttpData.reset();//SPHttpData是在Timer头文件里有std::shared_ptr<HttpData> SPHttpData，reset就是shared_ptr的reset方法。无参数的就是删除对象。
    this->setDeleted();
}


TimerManager::TimerManager()
{ }

TimerManager::~TimerManager()
{ }

void TimerManager::addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout)//HttpData是一个类，这个函数的作用是向优先队列中添加TimeNode类。TimeNode是一个关于时间节点的类
{
    SPTimerNode new_node(new TimerNode(SPHttpData, timeout));
    timerNodeQueue.push(new_node);//std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp> timerNodeQueue;timerNodeQueue是有限对列。
    SPHttpData->linkTimer(new_node);//这个是HttpData类里的。
}


/* 处理逻辑是这样的~
因为(1) 优先队列不支持随机访问
(2) 即使支持，随机删除某节点后破坏了堆的结构，需要重新更新堆结构。
所以对于被置为deleted的时间节点，会延迟到它(1)超时 或 (2)它前面的节点都被删除时，它才会被删除。
一个点被置为deleted,它最迟会在TIMER_TIME_OUT时间后被删除。
这样做有两个好处：
(1) 第一个好处是不需要遍历优先队列，省时。
(2) 第二个好处是给超时时间一个容忍的时间，就是设定的超时时间是删除的下限(并不是一到超时时间就立即删除)，如果监听的请求在超时后的下一次请求中又一次出现了，
就不用再重新申请RequestData节点了，这样可以继续重复利用前面的RequestData，减少了一次delete和一次new的时间。
*/

void TimerManager::handleExpiredEvent()
{
    //MutexLockGuard locker(lock);
    while (!timerNodeQueue.empty())//如果这个优先对列timerNodeQueue非空。
    {
        SPTimerNode ptimer_now = timerNodeQueue.top();//取出队列的首位元素。
        if (ptimer_now->isDeleted())//SPTimerNode，定义在类型里：typedef std::shared_ptr<TimerNode> SPTimerNode; ptimer_now是一个SPTimerNode类型，而这个isDeleted()是在头文件中定义的
          //返回值是bool isDeleted() const { return deleted_;  }
            timerNodeQueue.pop();//如果这个是TimerNode类型的共享内存被销毁，则pop带哦
        else if (ptimer_now->isValid() == false)
            timerNodeQueue.pop();
        else
            break;
    }
}
