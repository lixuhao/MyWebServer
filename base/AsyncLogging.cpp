/*
 *@Author Lixuhao
 *@Email xxx.qq.com
 */
#include "AsyncLogging.h"
#include "LogFile.h"
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <functional>


AsyncLogging::AsyncLogging(std::string logFileName_,int flushInterval)
  : flushInterval_(flushInterval),
  running_(false),
  basename_(logFileName_),
  thread_(std::bind(&AsyncLogging::threadFunc, this), "Logging"),
  mutex_(),
  cond_(mutex_),
  currentBuffer_(new Buffer),//当前缓冲区
  nextBuffer_(new Buffer),//备用还冲区
  buffers_(),
  latch_(1)
{
  assert(logFileName_.size() > 1);
  currentBuffer_->bzero();
  nextBuffer_->bzero();
  buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
  MutexLockGuard lock(mutex_);
  if (currentBuffer_->avail() > len)//当前缓冲区足够
  currentBuffer_->append(logline, len);
  else
  {
  buffers_.push_back(currentBuffer_);//不足的话，就把这个buf存入这个专门存储buffer的vector容器里.
  currentBuffer_.reset();//存储进vector后，再将当前这块缓冲区重置。
  if (nextBuffer_)
  currentBuffer_ = std::move(nextBuffer_);//移动语义，将一个左值移动为右值
  else
  currentBuffer_.reset(new Buffer);//如果两个都不足够，则重新开辟一个buffer
  currentBuffer_->append(logline, len);
  cond_.notify();
  }
}

//接收方的后端实现
void AsyncLogging::threadFunc()
{
  assert(running_ == true);
  latch_.countDown();//启用倒计时门闩
  LogFile output(basename_);//定义一个LogFile对象output，传入日志文件名

  //在头文件里有一个typedef std::shared_ptr<Buffer> BufferPtr;
  //所以这个是两个Buffer类型的share_ptr.而这个Buffer类型在头文件里是大小为4000的FixedBuffer
  //FixedBuffer又在LogStream.cpp里定义为template class FixedBuffer<kSmallBuffer>;
  BufferPtr newBuffer1(new Buffer);
  BufferPtr newBuffer2(new Buffer);
  //初始化为0
  newBuffer1->bzero();
  newBuffer2->bzero();

  //BufferVector是在头文件中定义为typedef std::vector<std::shared_ptr<Buffer>> BufferVector;
  //所以说是一个存储着shared_ptr的vector容器。
  BufferVector buffersToWrite;

  buffersToWrite.reserve(16);//vector的reserve函数，reserve有预留的意思，reserver函数用来给
  //vector预分配存储区大小，即capacity的值 ，但是没有给这段内存进行初始化
  while (running_)
  {
  assert(newBuffer1 && newBuffer1->length() == 0);
  assert(newBuffer2 && newBuffer2->length() == 0);
  assert(buffersToWrite.empty());

  {
  MutexLockGuard lock(mutex_);
  if (buffers_.empty())  // unusual usage!
  {
  cond_.waitForSeconds(flushInterval_);
  }
  buffers_.push_back(currentBuffer_);
  currentBuffer_.reset();

  currentBuffer_ = std::move(newBuffer1);
  buffersToWrite.swap(buffers_);
  if (!nextBuffer_)
  {
  nextBuffer_ = std::move(newBuffer2);
  }
  }

  assert(!buffersToWrite.empty());

  if (buffersToWrite.size() > 25)
  {
  //char buf[256];
  // snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
  //          Timestamp::now().toFormattedString().c_str(),
  //          buffersToWrite.size()-2);
  //fputs(buf, stderr);
  //output.append(buf, static_cast<int>(strlen(buf)));
  buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
  }

  for (size_t i = 0; i < buffersToWrite.size(); ++i)
  {
  // FIXME: use unbuffered stdio FILE ? or use ::writev ?
  output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
  }

  if (buffersToWrite.size() > 2)
  {
  // drop non-bzero-ed buffers, avoid trashing
  buffersToWrite.resize(2);
  }

  if (!newBuffer1)
  {
  assert(!buffersToWrite.empty());
  newBuffer1 = buffersToWrite.back();
  buffersToWrite.pop_back();
  newBuffer1->reset();
  }

  if (!newBuffer2)
  {
  assert(!buffersToWrite.empty());
  newBuffer2 = buffersToWrite.back();
  buffersToWrite.pop_back();
  newBuffer2->reset();
  }

  buffersToWrite.clear();
  output.flush();
  }
  output.flush();
}
