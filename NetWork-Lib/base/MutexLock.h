/*
  *@Athor liixuhao
  *@Email xxxqq.com
*/
#pragma once

#include "noncopyable.h"
#include <pthread.h>
#include <cstdio>

//下面的这两个class 都不允许拷贝构造和赋值，所以继承类noncopyable

//给线程加锁的一个类,MutexLock 封装临界区（critical section），这是一个简单的资源类，用RAII 手法
class MutexLock:noncopyable //继承
{
  public:
  MutexLock()
  {
  //初始化锁,这个是动态分批锁
  pthread_mutex_init(&mutex, NULL);
  }
  ~MutexLock()
  {
  pthread_mutex_lock(&mutex);
  pthread_mutex_destroy(&mutex);
  }

  //加锁
  void lock()
  {
  pthread_mutex_lock(&mutex);               
  }

  //解锁
  void unlock()
  {
  pthread_mutex_unlock(&mutex);              
  }


  pthread_mutex_t *get()
  {
  return &mutex;              
  }

private:
  pthread_mutex_t mutex;//互斥量
 //友元类不受访问权限影响
private:
  friend class Condition;//友元类
};

//这个类的作用:封装临界区的进入和退出，即加锁和解锁,用类对象的管理mutex。有点像c++的std中的lock_guard。
class MutexLockGuard: noncopyable
{
  public:
  explicit MutexLockGuard(MutexLock &_mutex):mutex(_mutex)
  {
  mutex.lock();
  }
 ~MutexLockGuard()
 {
  mutex.unlock();
 }
  private:
  MutexLock &mutex;//这儿的取地址是让为了获取MetexLock类中的mutex,因为MutexLock中只有mutex这个变量，所以类MutexLock占据的内存也只是mutex所占的内存大小。
};
