#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include "message.h" 
using namespace std;

int main()
{
    char recv_buff[1024] = {0};
    char send_buff[1024] = {0};

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", (struct in_addr *)&server_addr.sin_addr);
    server_addr.sin_port = htons(12345);

    cout<<"port "<<server_addr.sin_port<<endl;

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket < 0)
    {
        cout<<"client socket create fail"<<endl;
        return -1;
    }

    int ret = connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(ret < 0)
    {
        cout<<"connect fail"<<endl;
        cout<<ret<<endl;
        return -1;
    }

    //send message
    MZMessage msg;
    MZMessageHead head;
    head.version = 1;
    head.len = sizeof(msg);

    MZMessageBody body;
    memset(body.name, 0, 20);
    memset(body.desc, 0, 20);
    strcpy(body.name, "body name test");
    strcpy(body.desc, "body desc test");

    msg.head = head;
    msg.body = body;
    cout<<"len="<<sizeof(msg)<<endl;

    void* ptr = memcpy(send_buff, &msg, sizeof(msg));

    ret = send(client_socket, send_buff, sizeof(msg), 0);
    if(ret < 0)
    {
        cout<<"send fail"<<endl;
        return -1;
    }

    /*
    ret = recv(client_socket, recv_buff, sizeof(recv_buff), 0);
    if(ret < 0)
    {
        cout<<"recv fail"<<endl;
        return -1;
    }
    */

    return 0;
}
