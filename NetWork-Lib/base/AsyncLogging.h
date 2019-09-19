/*
 *@Author Lixuhao
 *@Email xxx@qq.com
 */
#pragma once
#include "CountDownLatch.h"
#include "MutexLock.h"
#include "Thread.h"
#include "LogStream.h"
#include "noncopyable.h"
#include <functional>
#include <string>
#include <vector>

class AsyncLogging : noncopyable
{
public:
  AsyncLogging(const std::string basename, int flushInterval = 2);
  ~AsyncLogging()
  {
  if (running_)
  stop();
  }

  void append(const char* logline, int len);

  void start()
  {
    //在构造函数中latch_的值为1，
    //线程运行后将latch_减为0
  running_ = true;
  thread_.start();

  //必须等到latch_变为0才能从start函数中返回，这表面初始化已经开始。
  latch_.wait();
  }

  void stop()
  {
  running_ = false;
  cond_.notify();
  thread_.join();
  }


private:
  void threadFunc();
  typedef FixedBuffer<kLargeBuffer> Buffer;

  //用unique_ptr管理buffer，持有对对象的独有权，不能进行复制操作只能进行移动操
  //作（效率更高）
  typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
  typedef std::shared_ptr<Buffer> BufferPtr;

  const int flushInterval_;//// 定期（flushInterval_秒）将缓冲区的数据写到文件中
  bool running_;//// 是否正在运行
  std::string basename_;// 日志名字
  Thread thread_;// 执行该异步日志记录器的线程
  MutexLock mutex_;
  Condition cond_;
  BufferPtr currentBuffer_;//// 当前的缓冲区
  BufferPtr nextBuffer_;// 下一个缓冲区
  BufferVector buffers_;//缓冲区队列
  CountDownLatch latch_;
};
