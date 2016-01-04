#include "mconn.h"

MZConnSvr::MZConnSvr(const string& ip, int32_t port) :
_ip(ip),
_port(port)
{}    

int32_t MZConnSvr::initSvr()
{
    //设置命名管道
    string fifo_file("fifo");
    setFifo(MCommonTool::makeFIFO(fifo_file.c_str()));
   
    //设置单个进程允许打开的最大文件数
    MCommonTool::setMaxOpenFiles(false, MAX_EPOLL_EVENT_SIZE);    

    return 0;              
}

int32_t MZConnSvr::run()
{
    struct sockaddr_in svr_addr, cli_addr;
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

    int conn_fd = 0;
    int curr_conn_num = 0;

    while(1)
    {
        //非阻塞
        int num_events = epoll_wait(epoll_fd, events, MAX_EPOLL_EVENT_SIZE, 0);
        
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
                }while(conn_fd);
          
                //正常结束或者发生异常时，默认不处理
                if(conn_fd < 0)
                {
                    continue;
                }
           
                //过载断掉连接
                if(curr_conn_num >= MAX_EPOLL_EVENT_SIZE)
                {
                    close(conn_fd);
                    continue;
                }
           
                //设置非阻塞失败时，则直接断掉连接
                if(0 > MNetTool::setNonBlocking(conn_fd))
                {
                    close(conn_fd);
                    continue;
                }
               
                //注册到epoll失败时，则直接断掉连接
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = conn_fd;
                if(0 > epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev))
                {
                    close(conn_fd);
                    continue;
                }

                //连接计数加1
                curr_conn_num++;
            }
            else
            {
                int task = 0;

                if(events[i].events & EPOLLIN)
                {
                    task += handleInput(events[i].data.fd);
                }
                else
                {
                    task += handleOutput(events[i].data.fd);
                }
            }
        }
    }
}

int main()
{
    string ip("127.0.0.1");
    MZConnSvr server(ip, 12345);
    server.run();
    return 0;
}
