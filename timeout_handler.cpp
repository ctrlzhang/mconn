#include "timeout_handler.h"
#include <iostream>
using namespace std;

void TimeoutHandler::checkTimeout()
{
    time_t current = MTimeTool::getCurrentTime();
    map<time_t, int32_t>::iterator it = _timeout_map.begin();
    while(it != _timeout_map.end())
    {
        if(it->first > current)
        {
            break;
        }
        
        handle(it->second);        
    }
}

void TimeoutHandler::handle(int32_t conn_fd)
{
    cout<<"handle "<<conn_fd<<endl;    
}

void TimeoutHandler::addConn(time_t timestamp, int32_t conn_fd)
{
    _timeout_map.insert(pair<time_t, int32_t>(timestamp, conn_fd));
}

void TimeoutHandler::delConn(time_t timestamp)
{
    _timeout_map.erase(timestamp); 
}
