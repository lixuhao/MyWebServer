/*
 *@Athor lixuhao
 *@Email xxxqq.com
 */
#pragma once

#include "noncopyable.h"
#include "MutexLock.h"
#include "Condition.h"


//CountDownLatch的主要作用是确保Thread中传进去的func真的启动了以后,外层的start才返回
class CountDownLatch:noncopyable
{
  public:
  explicit CountDownLatch(int count);
  void wait();
  void countDown();
  private:
  mutable MutexLock mutex_;//用mutable声明，可以打破const的修饰，当在CountDownLatch这个类的
  //const成员函数中想要修改这个mutable修饰的成员的时候，既可以修改了。
  Condition condition_;
  int count_;
};
