/*
 *@Athor lixuhao
 *@Email xxx@qq.com
 **/

#pragma once

#include "CountDownLatch.h"
#include "noncopyable.h"
#include <functional>
#include <memory>
#include <pthread.h>
#include <string>
#include <sys/syscall.h>
#include <unistd.h>

class Thread : noncopyable
{
  public:
  typedef std::function<void ()> ThreadFunc;//类型的重定义，将std::function<void ()>定义为 ThreadFunc。

  explicit Thread(const ThreadFunc&, const std::string& name = std::string());//这个是
  ~Thread();
  void start();
  int join();
  bool started() const { return started_;}
  pid_t tid() const { return tid_;}
  const std::string& name() const { return name_;}

  private:
  void setDefaultName();
  bool started_;
  bool joined_;
  pthread_t pthreadId_;
  pid_t tid_;
  ThreadFunc func_;
  std::string name_;
  CountDownLatch latch_;

};
