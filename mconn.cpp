#include "mconn.h"

MZConnSvr::MZConnSvr(const string& ip, int32_t port) :
_ip(ip),
_port(port),
_timeout(0),
_max_conn_num(0),
_listen_fd(0),
_recv_queue(ShmRecvQueue(SHARE_MEM_QUEUE_ITEM, SHARE_MEM_QUEUE_SIZE)),
_send_queue(ShmSendQueue(SHARE_MEM_QUEUE_ITEM, SHARE_MEM_QUEUE_SIZE)),
_fifo_fd(0)
{
}

int32_t MZConnSvr::initSvr()
{
    //设置命名管道
    string fifo_file("fifo");
    setFifo(MCommonTool::makeFIFO(fifo_file.c_str()));
   
    //设置单个进程允许打开的最大文件数
    MCommonTool::setMaxOpenFiles(false, MAX_EPOLL_EVENT_SIZE);    

    //初始化接收队列和发送队列
    //需指定不同的key
    _recv_queue.get("/tmp/recv_queue");
    int ret = _recv_queue.initMem();
    if(ret < 0)
    {
        cout<<"attach fail"<<endl;
        return -1;
    }

    _send_queue.get("/tmp/send_queue");
    ret = _send_queue.initMem();
    if(ret < 0)
    {
        cout<<"attach fail"<<endl;
        return -1;
    }

    return 0;              
}

int32_t MZConnSvr::run()
{
    initSvr();

    struct sockaddr_in svr_addr;
    socklen_t socklen = sizeof(struct sockaddr_in);
    
    struct epoll_event ev;
    struct epoll_event events[MAX_EPOLL_EVENT_SIZE];

    bzero(&svr_addr, sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    svr_addr.sin_port = htons(getPort());

    int32_t listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_fd < 0)
    {
        //throw exception
        return -1;
    }

    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    MNetTool::setNonBlocking(listen_fd); 

    if(0 > bind(listen_fd, (struct sockaddr *)&svr_addr, sizeof(struct sockaddr)))
    {
        return -1;
    }

    if(0 > listen(listen_fd, LISTEN_BACKLOG))
    {
        return -1;
    }

    int32_t epoll_fd = epoll_create(MAX_EPOLL_EVENT_SIZE);
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_fd;

    if(0 > epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &ev))
    {
        return -1;
    }

    //监听有名管道
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = _fifo_fd;
    if(0 > epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _fifo_fd, &ev))
    {
        return -1;
    }

    int conn_fd = 0;
    int curr_conn_num = 0;

    while(1)
    {
        //非阻塞
        int num_events = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENT_SIZE, 10000000);
        
        cout<<num_events<<endl;

        if(num_events <= 0)
        {
            cout<<"no events"<<endl;
            usleep(100000);
            continue;
        }

        int i = 0;
        for(; i< num_events; i++)
        {
            if(events[i].data.fd == listen_fd)
            {
                //建立连接,ET模式下需要循环accept，直到返回-1
                do
                {
                    conn_fd = accept(listen_fd, (struct sockaddr *)&svr_addr, &socklen);
                    if(conn_fd <= 0)
                    {
                        break;
                    }

                    LOG("accept success");
                    LOG(conn_fd);
          
                    //过载断掉连接
                    if(curr_conn_num >= MAX_EPOLL_EVENT_SIZE)
                    {
                        LOG("> max connection");
                        close(conn_fd);
                        continue;
                    }
           
                    //设置非阻塞失败时，则直接断掉连接
                    if(0 > MNetTool::setNonBlocking(conn_fd))
                    {
                        LOG("set non blocking fail");
                        close(conn_fd);
                        continue;
                    }
               
                    //注册到epoll失败时，则直接断掉连接
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = conn_fd;
                    if(0 > epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev))
                    {
                        LOG("epoll ctl fail");
                        close(conn_fd);
                        continue;
                    }

                    //连接计数加1
                    curr_conn_num++;
                    LOG("curr conn num");
                    LOG(curr_conn_num);
                }
                while(conn_fd > 0);
            }
            else if(events[i].data.fd == _fifo_fd)
            {
                LOG("notify from fifo");
                LOG("send out");
            }
            else
            {
                LOG("message come");
                handleInput(events[i].data.fd);
            }
        }
    }
}

int32_t MZConnSvr::handleInput(int32_t fd)
{
    //非阻塞的socket
    //接收数据
    char msg_buff[8192] = {0};
    char buff[1024] = {0};
    int flag = 1;
    int recv_len = 0;
    int pos = 0;

    while(flag)
    {
        recv_len = recv(fd, buff, sizeof(buff), 0);
        
        //异常或读完了
        if(recv_len < 0)
        {
            if(errno != EAGAIN)
            {
                //关闭连接
                close(fd);
                LOG("errno != EAGAIN, close socket");
            }

            break;
        }
        
        //对端关闭
        if(recv_len == 0)
        {
            close(fd);
            LOG("client close socket");
            
            break;
        }
        
        //正常读到了
        //拷贝数据到msg_buff中, 重置接收缓冲区
        cout<<"recv len="<<recv_len<<endl;

        memcpy(msg_buff + pos, buff, recv_len);
        pos += recv_len;
        memset(buff, 0, sizeof(buff));
       
        //没数据可以读了    
        if((unsigned long)recv_len < sizeof(buff))
        {
            flag = 0;
        }
    }
 
    if(pos > 0)
    {
        MZMessage msg;
        memcpy(&msg, msg_buff, pos);
        cout<<msg.head.len<<" "<<msg.head.version<<" "<<msg.body.name<<" "<<msg.body.desc<<endl;
    
        //将请求放入共享内存队列中
        _recv_queue.push_back(msg);  
        _recv_queue.status();
    }

    return 0;
}

int main()
{
    string ip("127.0.0.1");
    MZConnSvr server(ip, 12345);
    server.run();
    return 0;
}
