/*
 * @Author Lixuhao
 * @Email xx@qq.com
 */ 
 

#pragma once
#include "LogStream.h"
#include <pthread.h>
#include <string.h>
#include <string>
#include <stdio.h>

class AsyncLogging;//


class Logger
{
public:
  Logger(const char *fileName, int line);
  ~Logger();

  LogStream& stream() { return impl_.stream_; }//定义一个日志流

  static void setLogFileName(std::string fileName)//设置日志文件名
  {
  logFileName_ = fileName;
  }
  static std::string getLogFileName()//获取日志文件名。
  {
  return logFileName_;
  }

private:
  class Impl
  {
  public:
  Impl(const char *fileName, int line);
  void formatTime();

  LogStream stream_;
  int line_;
  std::string basename_;
  };
  Impl impl_;
  static std::string logFileName_;
};

#define LOG Logger(__FILE__, __LINE__).stream()//这个是将LOG定义为logger类的stream方法
