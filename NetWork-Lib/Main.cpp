// @Author Lixuhao
// @Email xxx@qq.com


#include "EventLoop.h"
#include "Server.h"
#include "base/Logging.h"
#include <getopt.h>//在这个代码里主要用到这个头文件里的getopt函数
#include <string>

int main(int argc, char *argv[])
{
    int threadNum = 4;
    int port = 80;
    std::string logPath = "./WebServer.log";

    // parse args
    int opt;
    const char *str = "t:l:p:";
    while ((opt = getopt(argc, argv, str))!= -1)
    {
        switch (opt)
        {
            case 't':
            {
                threadNum = atoi(optarg);//这个optarg是getopt函数带来的，optarg——指向当前选项参数（如果有）的指针（optarg不需要定义，在getopt.h中已经有定义,由于 optarg 都是字符串类型的，
                //所以当我们想要整型的输入参数时，会经常用到 atio() 这个方法atoi (表示 ascii to integer) 是把字符串转换成整型数的一个函数，包含在 <stdlib.h> 头文件中。
                break;
            }
            case 'l':
            {
                logPath = optarg;//类对象的对象名是地址
                if (logPath.size() < 2 || optarg[0] != '/')
                {
                    printf("logPath should start with \"/\"\n");
                    abort();
                }
                break;
            }
            case 'p':
            {
                port = atoi(optarg);
                break;
            }
            default: break;
        }
    }
    Logger::setLogFileName(logPath);
    // STL库在多线程上应用
    #ifndef _PTHREADS
        LOG << "_PTHREADS is not defined !";
    #endif
    EventLoop mainLoop;
    Server myHTTPServer(&mainLoop, threadNum, port);
    myHTTPServer.start();
    mainLoop.loop();
    return 0;
}
