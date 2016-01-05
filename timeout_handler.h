#ifndef _TIMEOUT_HANDLER_H_
#define _TIMEOUT_HANDLER_H_

#include "common.h"
#include <map>
using namespace std;

/*
 * @brief 超时管理类
 * 
 */
class TimeoutHandler
{
    public:
        //超时检查
        void checkTimeout();
        
        //添加
        void addConn(time_t timestamp, int32_t conn_fd);

        //删除
        void delConn(time_t timestamp);

        //处理超时
        void handle(int32_t conn_fd);

    private:
        //key - timestamp, value - connfd
        multimap<time_t, int32_t> _timeout_map;
};

#endif
