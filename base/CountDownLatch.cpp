/*
 *@Athor liixuhao 
 *@Email xxxqq.com
 */

#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count):mutex_(),condition_(mutex_),count_(count)//成员变量
//mutex_类类型，括号里是空的，说明是使用的默认的构造函数,这个count_一开始就要初始化。
{}

void CountDownLatch::wait()
{
  MutexLockGuard lock(mutex_);
  while(count_>0)
  {
  condition_.wait();//wait函数是Condition类中的等待触发条件的函数
  }
}

void CountDownLatch::countDown()
{
  MutexLockGuard lock(mutex_);
  --count_;
  if(count_==0)
  {
  condition_.notifyAll();
  }
}
