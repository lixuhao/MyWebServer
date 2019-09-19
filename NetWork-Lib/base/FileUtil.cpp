/*
 *@Author lixuhao
 *Email xxx@qq.com
 */


#include "FileUtil.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

AppendFile::AppendFile(string filename):fp_(fopen(filename.c_str(),"ae"))//以ae的方式打开filename.c_str文件,a表示增补，表示文件不存在就创建一个。e表示O_CLOEXEC 
  //调用open函数O_CLOEXEC模式打开的文件描述符在执行exec调用新程序中关闭，且为原子操作。exec是用新的进程去代替原来的进程，但进程的PID保持不变。因此，可以这样认为，exec系统调用并没有创建
  //新的进程，只是替换了原来进程上下文的内容。原进程的代码段，数据段，堆栈段被新的进程所代替。
{
//用户提供缓冲区
setbuffer(fp_, buffer_, sizeof buffer_);//fp是文件流，buffer_是缓冲区的起始地址，sizeof buffer是缓冲区的大小
}

AppendFile::~AppendFile()
{
  fclose(fp_);    
}

void AppendFile::append(const char* logline, const size_t len)
{
 size_t n = this->write(logline, len);//logline应该指的是类似文件的(现在看来就是要写入的缓冲区的起始地址)，len是写入的长度
size_t remain = len - n;//remain代表还需写入的数
while (remain > 0)//
{
size_t x = this->write(logline + n, remain);//缓冲区向后走n位，remain表示要写的数,
if(0==x)//返回为0，说明写入失败，为0，说明是fwrite的第二个或者第三个为0，在这儿的第二个参数是1，则这儿返回值为0,则第三个参数为0,反正为0，说明没写入到缓冲区，
{
int err=ferror(fp_);//检查这个文件流的返回值，如果是0，表示未出错，返回一个非零值，表示出错。对同一个文件 每一次调用输入输出函数，均产生一个新的ferror函 数值，
if(err)
  fprintf(stderr,"AppendFile::append() failed ! \n");
break;
}
n += x;//这个说明n写入正确,那么变化n 让n往后走，这个因为fwrite的第二个参数是1，第三个参数是要写入的总字节数，所以n+x就相当于从原来的n的位置向后再走写入的字节数的大小，
remain = len - n;//这个是记录要写入的长度还有多少要写。
}
}

void AppendFile::flush()
{
  fflush(fp_);
}

size_t AppendFile::write(const char* logline, size_t len)
{
  return fwrite_unlocked(logline, 1, len, fp_);//以unlocked为后缀的是非锁定的标准输入输出函数
//与它没有 `_unlocked` 后缀的对应版本行为一致，但是它们不使用锁定 (它们不自行设置锁定，也不判断
//  是否有其他函数设置的锁定) ，因此是非线程安全的。参见 flockfile(3) 
 // 就是说写入时是不加锁的。
 //
 //为了快速，使用unlocked(无锁)的fwrite函数。平时我们使用的C语言IO函数，都是线程安全的，
 ////而这个线程安全，是在函数的内部加锁。这会拖慢速度。而对于这个类。因为文件是以原子方式打开的，所以可以保证，从
 ////始到终只有一个线程能访问，所以无需进行加锁操作
 //这个函数的参数
 //第一个是参数是写入的缓冲区的起始地址，
 //第二个是要写入的单个元素的字节大小。
 //第三个是要写入的元素的总字节数
 //第四个是文件流。
 //
 //返回值是：1、fwrite调用不成功，返回0；
 //2、没有写完你要求的，返回所写的count值。
}




