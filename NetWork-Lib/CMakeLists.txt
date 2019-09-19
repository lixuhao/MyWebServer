set(SRCS
    Channel.cpp
    Epoll.cpp
    EventLoop.cpp
    EventLoopThread.cpp
    EventLoopThreadPool.cpp
    HttpData.cpp
    Main.cpp
    Server.cpp
    #ThreadPool.cpp
    Timer.cpp
    Util.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/base)


add_executable(WebServer ${SRCS})#使用给定的源文件，为工程引入一个可执行文件，而add_executable括号里的东西是生成的可执行文件的名字
target_link_libraries(WebServer libserver_base)
#上面这个指令的作用为将目标文件与库文件进行链接,而这个库文件是由源文件生成的可链接文件，这个可以在base目录下看有个add_library(libserver_base ${SRC})


add_subdirectory(base)
add_subdirectory(tests)

#add_executable的意思是使用给定的源文件，为工程引入一个可执行文件

