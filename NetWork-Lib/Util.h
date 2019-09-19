// @Author Lixuhao
// @Email xxx@qq.com
#pragma once
#include <cstdlib>
#include <string>

//ssize_t是signed size_t的意思
ssize_t readn(int fd, void *buff, size_t n);
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
ssize_t readn(int fd, std::string &inBuffer);

//为啥有3种readn,两种writen。
ssize_t writen(int fd, void *buff, size_t n);
ssize_t writen(int fd, std::string &sbuff);
void handle_for_sigpipe();//出现信号sigpipe的时候的处理方式

int setSocketNonBlocking(int fd);
void setSocketNodelay(int fd);
void setSocketNoLinger(int fd);
void shutDownWR(int fd);
int socket_bind_listen(int port);
