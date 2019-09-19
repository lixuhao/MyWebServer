/*
 * @Author lixuhao
 * @Email xxx@qq.com
 */
#pragma once
#include "FileUtil.h"
#include "MutexLock.h"
#include "noncopyable.h"
#include <memory>
#include <string>

// TODO 提供自动归档功能
class LogFile : noncopyable
{
public:
  // 每被append flushEveryN次，flush一下，会往文件写，只不过，文件也是带缓冲区的
  LogFile(const std::string& basename, int flushEveryN = 1024);
  ~LogFile();

  void append(const char* logline, int len);
  void flush();
  bool rollFile();

private:
  void append_unlocked(const char* logline, int len);//这个是自己实现的append_unlocked。

  const std::string basename_;//日志文件名
  const int flushEveryN_;//日志写入的间隔时间

  int count_;//计数器，检测是否需要换新文件

  //用unique_ptr声明,肯定是用了unique_ptr的特点。unique_ptr不可拷贝和赋值，在某一时刻，只能有
  //一个unique_ptr指向特定的对象，当unique_ptr被销毁时，它所指向的对象也会被销毁。因此不允许
  //多个unique_ptr指向同一个对象，所以不允许拷贝与赋值。
  std::unique_ptr<MutexLock> mutex_;//加锁的，用unique_ptr来表示说明是
  std::unique_ptr<AppendFile> file_;//文件智能指针,AppendFile这个类在FileUtil里
};
