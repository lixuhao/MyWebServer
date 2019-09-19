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

/******************************************************************** 
 * Description : 
 * 前端在生成一条日志消息时，会调用AsyncLogging::append()。
 * 如果currentBuffer_够用，就把日志内容写入到currentBuffer_中，
 * 如果不够用(就认为其满了)，就把currentBuffer_放到已满buffer数组中，
 * 等待消费者线程（即后台线程）来取。则将预备好的另一块缓冲
 * （nextBuffer_）移用为当前缓冲区（currentBuffer_）。
 * *********************************************************************/
void AsyncLogging::append(const char* logline, int len)
{
  MutexLockGuard lock(mutex_);
  if (currentBuffer_->avail() > len)//当前缓冲区足够
  currentBuffer_->append(logline, len);

  //当前buffer已满
  else
  {
  buffers_.push_back(currentBuffer_);//不足的话，就把这个buf存入这个专门存储buffer的vector容器里.
  currentBuffer_.reset();//存储进vector后，再将当前这块缓冲区重置。

  // 如果另一块缓冲区不为空，则将预备好的另一块缓冲区移用为当前缓冲区。
  if (nextBuffer_)
  currentBuffer_ = std::move(nextBuffer_);//移动语义，将一个左值移动为右值

  // 如果把两块缓冲都用完了，那么只好分配一块新的buffer，作当前缓冲区。
  else
  currentBuffer_.reset(new Buffer);//如果两个都不足够，则重新开辟一个buffer

  //添加日志记录。
  currentBuffer_->append(logline, len);

  // 通知后端开始写入日志数据。
  cond_.notify();
  }
}

/******************************************************************** 
 * Description : 
 * 如果buffers_为空，使用条件变量等待条件满足（即前端线程把一个已经满了
 * 的buffer放到了buffers_中或者超时）。将当前缓冲区放到buffers_数组中。
 * 更新当前缓冲区（currentBuffer_）和另一个缓冲区（nextBuffer_）。
 * 将bufferToWrite和buffers_进行swap。这就完成了将写了日志记录的buffer
 * 从前端线程到后端线程的转变。
 * *********************************************************************/
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

  // 如果buffers_为空，那么表示没有数据需要写入文件，那么就等待指定的时间。
  if (buffers_.empty())  // unusual usage!
  {
  cond_.waitForSeconds(flushInterval_);
  }
  /******************************************************************** 
   * Description : 
   * 无论cond是因何（一是超时，二是当前缓冲区写满了）而醒来，都要将currentBuffer_放到buffers_中。  
   * 如果是因为时间到（3秒）而醒，那么currentBuffer_还没满，此时也要将之写入LogFile中。  
   * 如果已经有一个前端buffer满了，那么在前端线程中就已经把一个前端buffer放到buffers_中  
   * 了。此时，还是需要把currentBuffer_放到buffers_中（注意，前后放置是不同的buffer，  
   * 因为在前端线程中，currentBuffer_已经被换成nextBuffer_指向的buffer了）
   * *********************************************************************/

  buffers_.push_back(currentBuffer_);
  currentBuffer_.reset();

  // 将新的buffer（newBuffer1）移用为当前缓冲区（currentBuffer_
  currentBuffer_ = std::move(newBuffer1);

  // buffers_和buffersToWrite交换数据，此时buffers_所有的数据存放在buffersToWrite，而buffers_变为空。
  buffersToWrite.swap(buffers_);


  // 如果nextBuffer_为空，将新的buffer（newBuffer2）移用为另一个缓冲区（nextBuffer_）。
  if (!nextBuffer_)
  {
  nextBuffer_ = std::move(newBuffer2);
  }
  }

  assert(!buffersToWrite.empty());


// 如果将要写入文件的buffer列表中buffer的个数大于25，那么将多余数据删除。
// 前端陷入死循环，拼命发送日志消息，超过后端的处理能力，这是典型的生产速度超过消费速度
// 会造成数据在内存中的堆积，严重时引发性能问题(可用内存不足)或程序崩溃(分配内存失败)。

  if (buffersToWrite.size() > 25)
  {
  //char buf[256];
  // snprintf(buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
  //          Timestamp::now().toFormattedString().c_str(),
  //          buffersToWrite.size()-2);
  //fputs(buf, stderr);
  //output.append(buf, static_cast<int>(strlen(buf)));


     // 丢掉多余日志，以腾出内存，仅保留两块缓冲区
  buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
  }


  // 将buffersToWrite的数据写入到日志文件中
  for (size_t i = 0; i < buffersToWrite.size(); ++i)
  {
  // FIXME: use unbuffered stdio FILE ? or use ::writev ?
  output.append(buffersToWrite[i]->data(), buffersToWrite[i]->length());
  }


  // 重新调整buffersToWrite的大小
  if (buffersToWrite.size() > 2)
  {
  // drop non-bzero-ed buffers, avoid trashing
  buffersToWrite.resize(2);
  }


  // 从buffersToWrite中弹出一个作为newBuffer1 
  if (!newBuffer1)
  {
  assert(!buffersToWrite.empty());
  newBuffer1 = buffersToWrite.back();
  buffersToWrite.pop_back();
  newBuffer1->reset();
  }


  // 从buffersToWrite中弹出一个作为newBuffer2
  if (!newBuffer2)
  {
  assert(!buffersToWrite.empty());
  newBuffer2 = buffersToWrite.back();
  buffersToWrite.pop_back();
  newBuffer2->reset();
  }

  // 清空buffersToWrite
  buffersToWrite.clear();
  output.flush();
  }
  output.flush();
}
