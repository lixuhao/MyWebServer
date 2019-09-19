/*
 *@Athor liixuhao
 *@Email xxxqq.com
 */

#pragma once

#include "noncopyable.h"
#include "MutexLock.h"
#include <pthread.h>
#include <errno.h>
#include <cstdint>
#include <time.h>

class Condition:noncopyable
{
  public:
  explicit Condition(MutexLock &_mutex):mutex(_mutex)
  {
  pthread_cond_init(&cond, NULL);//初始化条件变量
  }
  
  ~Condition()
  {
  pthread_cond_destroy(&cond);//条件变量的销毁
  }

  //等待条件满足
  void wait()
  {
  pthread_cond_wait(&cond, mutex.get());
  }


  void notify()//notify是通知,通告的意思,
  {
  pthread_cond_signal(&cond); //唤醒等待的方式之一，这个函数是唤醒一个线程
  }


  void notifyAll()
  {
  pthread_cond_broadcast(&cond);//唤醒等待的方式之二，唤醒所有的线程
  }

bool waitForSeconds(int seconds)
  {
  struct timespec abstime; //time.h中的结构体,POSIX.4标准定义的一个时间结构， 共有两个成员：time_t tv_sec表示秒，long tv_nsec表示纳秒
  clock_gettime(CLOCK_REALTIME, &abstime);//"clock_gettime"是基于Linux C语言的时间函数,可以用于计算时间，有秒和纳秒两种精度,CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变

  abstime.tv_sec += static_cast<time_t>(seconds);//将seconds的类型强转为time_t类型,是在编译期间检查的，所以不能保证运行时类型识别的安全
  return ETIMEDOUT == pthread_cond_timedwait(&cond, mutex.get(), &abstime);//这个函数与pthread_cond_wait相似，不过多了一个超时时间参数。即这儿的第三个参数                                
  }

  private:
  MutexLock &mutex;
  pthread_cond_t cond;
};
