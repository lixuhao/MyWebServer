/*
 *@Athor lixuhao
 *@Email xxx@qq.com
 */
 
#pragma once

#include <stdint.h>

namespace CurrentThread
{
// internal
extern __thread int t_cachedTid;//__thread是一种关键字，这个关键字可以修饰线程内的数据，而且这个关键字只能修饰POD类型的数据，__thread是GCC内置的线程局部
//存储设施,_thread变量每一个线程有一份独立实体，各个线程有各自的数据，各个线程的值互不干扰。可以用来修饰那些带有全局性且值可能变，但是又不值得用全局变量保护的变量。
//还有t_cachedTid表示线程的真实id POSIX的thread库提供了pthread_self获取当前线程的标识符，类型为pthread_t，这是一个结构体，用起来不便。线程的真实id是一个整数，可以通过系统调用
//syscall(SYS_gettid)获得，在muduo中封装为gettid()函数。调用系统调用开销比较大，因此可以使用一个变量t_cachedTid来存储，在线程第一次使用tid时通过系统调用获得，存储在
//t_cacheTid中，以后使用时不再需要系统调用了。
extern __thread char t_tidString[32];//t_tidString[32],用string类型表示tid，便于输出日志。
extern __thread int t_tidStringLength;//t_tidStringLength string类型tid的长度
extern __thread const char* t_threadName;//t_threadName 线程的名字。

void cacheTid();

inline int tid()
{
if (__builtin_expect(t_cachedTid == 0, 0))//__builtin_expect，GCC为CPU的分支预测提供的内置函数
{
cacheTid();
}
return t_cachedTid;
}

 inline const char* tidString() // for logging
 {
return t_tidString;
 }

 inline int tidStringLength() // for logging
 {
 return t_tidStringLength;
 }

 inline const char* name()
 {
 return t_threadName;
 }
}
