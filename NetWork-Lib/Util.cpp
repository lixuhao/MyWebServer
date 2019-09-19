// @Author Lixuhao
// @Email xxx@qq.com
#include "Util.h"
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>////包含了SOL_TCP等宏的定义

const int MAX_BUFF = 4096;
ssize_t readn(int fd, void *buff, size_t n)
{
    size_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    char *ptr = (char*)buff;
    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)//read系统调用，读fd这个文件,从ptr开始，读nleft个字节。返回值是读到的字节数，读到的文件尾是0，出错返回-1
        {
            if (errno == EINTR)//read发生错误后read自身会返回一个-1，还会根据错误的原因返回一个errno值。EINTR是打断的信号，就是read的时候则系统会生成一个errno
                nread = 0;
            else if (errno == EAGAIN)////如果返回的错误是再来一次。
            {
                return readSum;////返回读了的总数目
            }
            else
            {
                return -1;////其他情况，返回-1。
            }  
        }
        else if (nread == 0)//就是调用read函数读到文件尾,
            break;//那就跳出本次循环。
        readSum += nread;//读了的总数目就是加读了的总数目。
        nleft -= nread;//这个就是看所要读的字节数还剩几个。
        ptr += nread;//这个是将未读区域的首地址向后走。
    }
    return readSum;////返回就是读了多少。
}

ssize_t readn(int fd, std::string &inBuffer, bool &zero)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true)
    {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
            {
                return readSum;
            }  
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
        {
            //printf("redsum = %d\n", readSum);
            zero = true;
            break;
        }
        //printf("before inBuffer.size() = %d\n", inBuffer.size());
        //printf("nread = %d\n", nread);
        readSum += nread;
        //buff += nread;
        inBuffer += std::string(buff, buff + nread);
        //printf("after inBuffer.size() = %d\n", inBuffer.size());
    }
    return readSum;
}


ssize_t readn(int fd, std::string &inBuffer)
{
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true)
    {
        char buff[MAX_BUFF];
        if ((nread = read(fd, buff, MAX_BUFF)) < 0)
        {
            if (errno == EINTR)
                continue;
            else if (errno == EAGAIN)
            {
                return readSum;
            }  
            else
            {
                perror("read error");
                return -1;
            }
        }
        else if (nread == 0)
        {
            //printf("redsum = %d\n", readSum);
            break;
        }
        //printf("before inBuffer.size() = %d\n", inBuffer.size());
        //printf("nread = %d\n", nread);
        readSum += nread;
        //buff += nread;
        inBuffer += std::string(buff, buff + nread);
        //printf("after inBuffer.size() = %d\n", inBuffer.size());
    }
    return readSum;
}


ssize_t writen(int fd, void *buff, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    char *ptr = (char*)buff;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                {
                    return writeSum;
                }
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    return writeSum;
}

ssize_t writen(int fd, std::string &sbuff)
{
    size_t nleft = sbuff.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char *ptr = sbuff.c_str();
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0)
            {
                if (errno == EINTR)
                {
                    nwritten = 0;
                    continue;
                }
                else if (errno == EAGAIN)
                    break;
                else
                    return -1;
            }
        }
        writeSum += nwritten;
        nleft -= nwritten;
        ptr += nwritten;
    }
    if (writeSum == static_cast<int>(sbuff.size()))
        sbuff.clear();
    else
        sbuff = sbuff.substr(writeSum);
    return writeSum;
}

void handle_for_sigpipe()
{
    struct sigaction sa;///这个结构体是信号机制的结构体。
    memset(&sa, '\0', sizeof(sa));////这个是将结构体清零
    sa.sa_handler = SIG_IGN;////这个是将处理方式处理为忽视。结构体sigaction的第一个参数 
    sa.sa_flags = 0;////结构体sigaction的第三个参数，设置为0 如果不需要重置该给定信号的处理函数为缺省值；并且不需要阻塞该给定信号(无须设置sa_flags标志)，那么必须将sa_flags清零，否则运行
    //将会产生段错误,选自：http://blog.chinaunix.net/uid-1877180-id-3011232.html
    if(sigaction(SIGPIPE, &sa, NULL))//sigaction的函数的返回值成功返回0，失败返回-1。这个是如果非0，即是-1，则return。sigaction的第一个参数是要处理的信号，第二个参数是针对信号SIGPIPE的处
    //理动作。第三个参数是针对SIGPIPE的当前操作。
        return;
}

int setSocketNonBlocking(int fd)
{
    int flag = fcntl(fd, F_GETFL, 0);//获取文件的fd值
    if(flag == -1)
        return -1;

    flag |= O_NONBLOCK;//设置为非阻塞。flag=flag|O_NONBLOCK,“|”是管道的意思
    if(fcntl(fd, F_SETFL, flag) == -1)//设置文件为非阻塞
        return -1;
    return 0;
}

void setSocketNodelay(int fd) 
{
    int enable = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable));//第一个参数是要设置的socket的fd，第二个参数是协议层，第三个参数是需要访问的第二个参数的选项名。这里
    //TCP_NODELAY就是IPPROTO_TCP这个协议层的选项。第4个参数是指向包含新选项值的缓冲区。第5个参数是选项的长度。
    //而TCP_NODELAY是关闭TCP的Nagle算法。
}

void setSocketNoLinger(int fd) 
{
    struct linger linger_;//linger也是一个结构体，包含l.onoff和l.linger
    linger_.l_onoff = 1;
    linger_.l_linger = 30;
    setsockopt(fd, SOL_SOCKET, SO_LINGER,(const char *) &linger_, sizeof(linger_));
}

void shutDownWR(int fd)
{
    shutdown(fd, SHUT_WR);//第一个参数是要shutdown的文件描述符，第二个参数是关闭的方式，SHUT_WR：值为1，关闭连接的写这一半
    //printf("shutdown\n");
}

int socket_bind_listen(int port)
{
    // 检查port值，取正确区间范围，端口范围就是在这两个数字之间
    if (port < 0 || port > 65535)
        return -1;

    // 创建socket(IPv4 + TCP)，返回监听描述符
    int listen_fd = 0;
    if((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        return -1;

    // 消除bind时"Address already in use"错误
    int optval = 1;
    if(setsockopt(listen_fd, SOL_SOCKET,  SO_REUSEADDR, &optval, sizeof(optval)) == -1)
        return -1;

    // 设置服务器IP和Port，和监听描述副绑定
    struct sockaddr_in server_addr;
    bzero((char*)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons((unsigned short)port);
    if(bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
        return -1;

    // 开始监听，最大等待队列长为LISTENQ
    if(listen(listen_fd, 2048) == -1)//listen函数的第二个参数是就是backlog，也就是最大并发连接数。
        return -1;

    // 无效监听描述符
    if(listen_fd == -1)
    {
        close(listen_fd);
        return -1;
    }
    return listen_fd;
}
