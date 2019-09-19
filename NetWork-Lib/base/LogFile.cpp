/*
 *@Author Lixuhao
 *@Email xxx@qq.com
 */
#include "LogFile.h"
#include "FileUtil.h"
#include <assert.h>
#include <stdio.h>
#include <time.h>

using namespace std;


LogFile::LogFile(const string& basename, int flushEveryN)
  : basename_(basename),
  flushEveryN_(flushEveryN),
  count_(0),
  mutex_(new MutexLock)
{
  //assert(basename.find('/') >= 0);
  file_.reset(new AppendFile(basename));
}

LogFile::~LogFile()//
{ }//默认的析构函数本来是不用写的，不过写出来更清晰明了

void LogFile::append(const char* logline, int len)
{
  MutexLockGuard lock(*mutex_);//锁的临界区
  append_unlocked(logline, len);//无锁添加
}

void LogFile::flush()
{
  MutexLockGuard lock(*mutex_);
  file_->flush();
}

void LogFile::append_unlocked(const char* logline, int len)
{
  file_->append(logline, len);
  ++count_;
  if (count_ >= flushEveryN_)
  {
  count_ = 0;
  file_->flush();//从缓冲区将数据写入内核。
  }
}
