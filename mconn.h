#ifndef _h_MZCONN_H_
#define _h_MZCONN_H_

#include <iostream>
#include <string>
#include "common.h"
#include "mqueue.h"
#include "message.h"

#define MAX_EPOLL_EVENT_SIZE 5000
#define LISTEN_BACKLOG 1024
#define SHARE_MEM_QUEUE_SIZE 1024 * 1024 * 100
#define SHARE_MEM_QUEUE_ITEM 10000

#define LOG(msg) cout<<msg<<endl

using namespace std;

/*
 * 连接管理类
 */
class MZConnSvr
{
    public:        
        explicit MZConnSvr(const string& ip, int32_t port);
   
        int32_t initSvr();

        int32_t run();

        int32_t getTimeout() const
        {
            return _timeout;
        }

        void setTimeout(int32_t timeout)
        {
            _timeout = timeout;
        }

        int32_t getMaxConnNum() const
        {
            return _max_conn_num;
        }

        void setMaxConnNum(int32_t max_conn_num)
        {
            _max_conn_num = max_conn_num;
        }

        int32_t getFifo() const
        {
            return _fifo_fd;
        }

        void setFifo(int fifo_fd)
        {
            _fifo_fd = fifo_fd;
        }

        int32_t getPort() const
        {
            return _port;
        }

        void setPort(int32_t port)
        {
            _port = port;
        }

        string getIp() const
        {
            return _ip;
        }

        void setIp(const string& ip)
        {
            _ip = ip;
        }

        //建立连接
        //请求入队列
        //响应出队列
        
        int32_t handleInput(int32_t fd); 

        int32_t handleOutput(int32_t fd)
        {
            LOG(fd);
            return 0;
        } 

        /*
        void setSendQueue(const ShmSendQueuePtr& queue)
        {
            _send_queue = queue;
        }

        ShmSendQueuePtr getSendQueue() const
        {
            return _send_queue;
        }

        void setRecvQueue(const ShmRecvQueuePtr& queue)
        {
            _recv_queue = queue;
        }

        ShmRecvQueuePtr getRecvQueue() const
        {
            return _recv_queue;
        }
        */

    private:
        //IP
        string _ip;

        //端口
        int32_t _port;

        //请求超时时间
        int32_t _timeout;

        //最大连接数
        int32_t _max_conn_num;

        //监听套接口， 暂时只支持监听一个
        int32_t _listen_fd;

        //接收队列
        ShmSendQueue _recv_queue;

        //发送队列
        ShmRecvQueue _send_queue;

        //有名管道
        int32_t _fifo_fd;
};

#endif
