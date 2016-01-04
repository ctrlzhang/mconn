#ifndef _H_MZQUEUE_H_
#define _H_MZQUEUE_H_

#include <deque>
using namespace std;

/*
 * 连接队列
 *
 */
template <class T>
class MZQueue
{
    public:
        MZQueue(){}
    private:
        deque<T> _queue;
};

#endif
