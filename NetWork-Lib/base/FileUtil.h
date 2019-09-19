/*
 *@Author lixuhao
 *@Email xxx@qq.com
 */

#pragma once

#include "noncopyable.h"
#include <string>

class AppendFile:noncopyable
{
  public:
  explicit AppendFile(std::string filename);
  ~AppendFile();
  void append(const char* logline,const size_t len);//向文件中写，这个写按照append的意思来说应该是续写，或者说添加内容。这个在FileUtil.cpp文件中自己实现
  void flush();

  private:
  size_t write(const char *logline, size_t len);//这个才是写
  FILE* fp_;//这个是文件指针，FILE结构体定义在stdio.h头文件中,是C语言定义的标准数据结构，用于文件,fp是文件指针，用于指向File类型的对象。既然这儿的头文件没有stdio.h,那么就在FileUtil.cpp文件里。
  
  char buffer_[64*1024];
};
