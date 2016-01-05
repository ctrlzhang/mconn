#include "common.h"

/*
 * @brief 创建有名管道 
 * @return <=0 - fail  >0 - fifo_fd
 */
int32_t MCommonTool::makeFIFO(const char* fifo_file)
{
    int ret = access(fifo_file, F_OK);
    if( 0 == ret || (-1 == ret && 0 == mkfifo(fifo_file, 0666)))
    {
        return open(fifo_file, O_RDWR); 
    }

    return -1;
}

/*
 * @param flag - 开关
 * @return <0 - fail
 */
int32_t MCommonTool::setMaxOpenFiles(bool flag, int limit)
{
    if(flag)
    {
        struct rlimit rt;
        rt.rlim_max = rt.rlim_cur = limit;
        if(0 > setrlimit(RLIMIT_NOFILE, &rt))
        {
            return -1;
        }
    }

    return 0;
}

/*
 * @brief 设置socket为非阻塞
 * @return =0 - success  <0 - fail
 */
int32_t MNetTool::setNonBlocking(int socket)
{
    int32_t flags = fcntl(socket, F_GETFL, 0);
    
    if(fcntl(socket, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        return -1;
    }

    return 0;
}

/*
 * @brief 获取当前时间
 *
 */
time_t MTimeTool::getCurrentTime()
{
    return time(NULL); 
}
