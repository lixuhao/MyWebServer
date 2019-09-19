/*
 * @Author Lixuhao
 * @Email xxx@qq.com
 */
#pragma once

#include "noncopyable.h"
#include <assert.h>
#include <string.h>
#include <string>


class AsyncLogging;
//FixedBuffer模版类有两个特例化：这两个变量kSmallBuffer和KlargeBuffer在类LogStream里有应用。
const int kSmallBuffer = 4000;
const int kLargeBuffer = 4000 * 1000;

//非类型的模板参数SIZE，在这个FIXedBUffer类里面充当缓冲区的大小，
//通过成员 data_首地址、cur_指针、end()完成对缓冲区的各项操作
//通过append()接口把日志内容添加到缓冲区来。
template<int SIZE> 
class FixedBuffer: noncopyable
{
public:
  FixedBuffer()
  :cur_(data_)//缓冲区的首地址
  { }

  ~FixedBuffer()
  { }

  void append(const char* buf, size_t len)
  {
  if (avail() > static_cast<int>(len))//如果可用空间大于要写入的空间则
  {
  memcpy(cur_, buf, len);//从buf所指地址开始向后将len长度的数据写入cur_开头的缓冲区。
  cur_ += len;//写入之后cur_向后走,走到的位置是未写部分的开头。
  }
  }

  const char* data() const { return data_; }//返回的是缓冲区的首地址
  int length() const { return static_cast<int>(cur_ - data_); }//返回写入的log日志的长度

  char* current() { return cur_; }//返回当前的可用空间的首地址。
  int avail() const { return static_cast<int>(end() - cur_); }//返回的是当前可用的空间
  void add(size_t len) { cur_ += len; }//未写的缓冲区的首地址向后走

  void reset() { cur_ = data_; }//自己实现的reset(重置的意思)
  void bzero() { memset(data_, 0, sizeof data_); }//初始化为0的函数。


private:
  const char* end() const { return data_ + sizeof data_; }//获取缓冲区的最后一个地址。

  char data_[SIZE];//缓冲区
  char* cur_;//
};



class LogStream : noncopyable
{
public:
  typedef FixedBuffer<kSmallBuffer> Buffer;//这个kSmallBuffer就是充当着FixedBuffer类里的模板参数SIZE 所以这个是一个缓冲。

  LogStream& operator<<(bool v)//重载运算符<<，这个是自己实现<<，而不是只用iostream里的“<<”
  //比如cout 后面的“<<”，这个可以看https://blog.csdn.net/wang13342322203/article/details/80522663

  {
  buffer_.append(v ? "1" : "0", 1);//v是真是假，为假则是append参数的第一个参数是0，否则为1。第二个参数是1.一次写一个。
  return *this;//返回调用这个函数的对象，即LogStream对象,
  }

  //下面是对不同的类型输出的重写。
  LogStream& operator<<(short);//这个是输出自己short类型的
  LogStream& operator<<(unsigned short);//unsigned short类型的
  LogStream& operator<<(int);
  LogStream& operator<<(unsigned int);
  LogStream& operator<<(long);
  LogStream& operator<<(unsigned long);
  LogStream& operator<<(long long);
  LogStream& operator<<(unsigned long long);

  LogStream& operator<<(const void*);

  LogStream& operator<<(float v)
  {
  *this << static_cast<double>(v);
  return *this;
  }
  LogStream& operator<<(double);
  LogStream& operator<<(long double);

  LogStream& operator<<(char v)
  {
  buffer_.append(&v, 1);
  return *this;
  }

  LogStream& operator<<(const char* str)
  {
  if (str)
  buffer_.append(str, strlen(str));
  else
  buffer_.append("(null)", 6);
  return *this;
  }

  LogStream& operator<<(const unsigned char* str)
  {
  return operator<<(reinterpret_cast<const char*>(str));
  }

  LogStream& operator<<(const std::string& v)
  {
  buffer_.append(v.c_str(), v.size());
  return *this;
  }

  void append(const char* data, int len) { buffer_.append(data, len); }
  const Buffer& buffer() const { return buffer_; }
  void resetBuffer() { buffer_.reset(); }

private:
  void staticCheck();

  template<typename T>
  void formatInteger(T);

  Buffer buffer_;

  static const int kMaxNumericSize = 32;
};

//主要是自己实现LogStream类，而不用iostream类，主要是出于性能。
//设计这个LogStream类，让它如同C++的标准输出流对象cout，能用<<符号接收输入，cout是直接输出到终端
//而LogStream类是把输出保存自己内部的缓冲区，可以让外部程序把缓冲区的内容重定向输出到不同的
//目标，如文件、终端、socket。
