#ifndef _H_COMMON_H_
#define _H_COMMON_H_ 

//for access
#include <unistd.h>

#include <fcntl.h>
#include <string>
#include <stdlib.h>
//for rlimit
#include <sys/resource.h>

//for mkfifo
#include <sys/types.h>
#include <sys/stat.h>

//for epoll
#include <sys/socket.h>
#include <sys/epoll.h>

//for htons
#include <arpa/inet.h>

//for bzeor
#include <string.h>

//for time
#include <time.h>

//EAGAIN
#include <errno.h>

using namespace std;

/*
 * 公共库
 */
class MCommonTool
{
    public:
        //创建命名管道
        static int32_t makeFIFO(const char* fifo_file);
        
        //设置进程允许打开的最大文件数
        static int32_t setMaxOpenFiles(bool flag, int limit); 
};

/*
 * 网络相关
 *
 */
class MNetTool
{
    public:
        static int32_t setNonBlocking(int socket);
};

/*
 * 时间相关
 */
class MTimeTool
{
    public:
        static time_t getCurrentTime();
};

#endif
