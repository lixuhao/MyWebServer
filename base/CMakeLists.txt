set(LIB_SRC
  AsyncLogging.cpp
  CountDownLatch.cpp
  FileUtil.cpp
  LogFile.cpp
  Logging.cpp
  LogStream.cpp
  Thread.cpp
)
#set命令是用来显示的定义变量,LIB_SRC代表后面的字符串

add_library(libserver_base ${LIB_SRC})
#该指令的主要作用就是将指定的源文件生成链接文件，然后添加到工程中去，这一句的意思是用LIB_SRC
#这些的cpp文件生成libserver_base可链接文件
target_link_libraries(libserver_base pthread rt)
#<libserver_base>是指通过add_executable()和add_library()指令生成已经创建的目标文件。这个命令
#是添加需要的共享库，这里的第二个参数是需要的共享库。这个命令是看
#了https://blog.csdn.net/arackethis/article/details/43488177

set_target_properties(libserver_base PROPERTIES OUTPUT_NAME "server_base")
# set_target_properties在这儿是设置输出的名称

add_subdirectory(tests)
#这个指令用于向当前工程添加存放源文件的子目录，并可以指定中间二进制和目标二进制存放的位置。
