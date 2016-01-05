#ifndef _H_MZQUEUE_H_
#define _H_MZQUEUE_H_

#include <deque>
#include <cstring>
#include "message.h"
#include "share_mem.h"
#include <iostream>
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

/*
 * @brief 共享内存元信息, 
 *        共享内存队列由元信息段+数据段组成
 *
 */
class ShmHead
{
    public:
        //队头相对于队列数据区首地址的偏移量, 采用相对偏移, 方便扩展队列头部
        int32_t _head;

        //队尾相对于队列数据区首地址的偏移量, 采用相对偏移, 方便扩展队列头部
        int32_t _tail;

        //队列当前包含的元素数量
        int32_t _item_num;

        //队列最大允许包含的元素数量
        int32_t _max_item_num;

        //队列当前占用的空间大小
        int32_t _space_size;

        //队列允许占用的最大空间大小
        int32_t _max_space_size;

        //显示状态
        void status()
        {
            cout<<"shm head status:"<<endl;
            cout<<"head="<<_head<<endl;
            cout<<"tail="<<_tail<<endl;
            cout<<"item_num="<<_item_num<<endl;
            cout<<"space_size="<<_space_size<<endl;
        }
};

/*
 * 共享内存队列
 *
 */
template <class T>
class ShmQueue : public ShareMem
{
    public:
        //循环队列的大小
        ShmQueue(int32_t num, int32_t size);

        /*
         * @brief 队头出数据
         * @return T 
         */
        T pop_front();

        /*
         * @brief 队尾入数据
         * @return 队尾指针
         */
        int32_t push_back(const T& item);

        //获取队头索引
        int32_t getHeadPos() const
        {
            return _shm_head._head;
        }

        //获取队尾索引
        int32_t getTailPos() const
        {
            return _shm_head._tail;
        }

        /*
         * @brief 队头等于队尾，表示队列为空, 否则非空 
         * @return true-空 
         */
        bool isEmpty()
        {
            return _shm_head._head == _shm_head._tail;
        }

        /*
         * @brief 是否过载
         * @return true-过载
         */
        bool isOverLoad(int32_t pkgLen)
        {
            return _shm_head._space_size + pkgLen >= _shm_head._max_space_size;
        }
    
        /*
         * @brief 连续空间足够完整拷贝一个元素
         * @return true - 足够
         */
        bool isSpaceEnough(int32_t pkgLen)
        {
            return _shm_head._tail + pkgLen <= _shm_head._max_space_size;
        }

        /*
         * @brief 初始化共享内存,写头部数据到共享内存中
         *
         */
        int32_t initMem()
        {
            int ret = attach();
            if(0 == ret)
            {
                setShmHead(_shm_head);
            }

            return ret;
        }

        /*
         * @breif 更新共享内存头部
         *
         */
        void setShmHead(const ShmHead& head)
        {
            memcpy(_shm_buf, &head, sizeof(ShmHead)); 
        }

        void setHeadPos(int32_t pos)
        {
            memcpy(_shm_buf, &pos, sizeof(int32_t));
        }

        void setTailPos(int32_t pos)
        {
            memcpy(_shm_buf + sizeof(int32_t) * 1, &pos, sizeof(int32_t));
        }

        void setCurItemNum(int32_t num)
        {
            memcpy(_shm_buf + sizeof(int32_t) * 2, &num, sizeof(int32_t));
        }

        void setSpace(int32_t space)
        {
            memcpy(_shm_buf + sizeof(int32_t) * 4, &space, sizeof(int32_t)); 
        }

        /*
         * @brief 统计共享内存状态
         *
         */
        void status()
        {
            _shm_head.status();
            ShareMem::status();
        }

        /*
         * @brief 获取共享内存头部信息
         */
        ShmHead getShmHead() const
        {
            return _shm_head;
        }

    private:
        ShmHead _shm_head;
};

template <class T>
ShmQueue<T>::ShmQueue(int32_t num, int32_t size) : ShareMem(num, size)  
{
    cout<<"shm queue init"<<endl;
    _shm_head._tail = _shm_head._head = sizeof(ShmHead);
    _shm_head._item_num = 0;
    _shm_head._max_item_num = num;
    _shm_head._space_size = sizeof(ShmHead);
    _shm_head._max_space_size = size;
}

template <class T>
T ShmQueue<T>::pop_front()
{
    //头部解析
    MZMessageHead reqHead;
    void* buff = _shm_buf + _shm_head._head;
    memcpy(&reqHead, buff, sizeof(MZMessageHead));
    int32_t pkgLen = reqHead.len;
    
    //读取包内容
    T tmp;
    memcpy(&tmp, buff, pkgLen);

    //更新共享内存头部信息
    _shm_head._head += pkgLen;
    if(_shm_head._head >= _shm_head._max_space_size)
    {
        _shm_head._head = _shm_head._head % _shm_head._max_space_size + sizeof(ShmHead);
    }
    
    //更新已占用空间容量
    _shm_head._space_size -= pkgLen;
    _shm_head._item_num -= 1;

    //返回请求包
    return tmp;
}

template <class T>
int32_t ShmQueue<T>::push_back(const T& item)
{
    int pkgLen = item.head.len;
    if(isOverLoad(pkgLen))
    {
        //发生过载
        cout<<"over load"<<endl;
        return -1;
    }

    //连续空间足够完整容纳包时，直接拷贝
    if(isSpaceEnough(pkgLen))
    {
        cout<<"space is enough"<<endl;
        cout<<"pkglen="<<pkgLen<<endl; 
        cout<<"shm_buf="<<_shm_buf<<endl;
        void* buff = static_cast<char*>(_shm_buf) + _shm_head._tail;
        memcpy(buff, &item, pkgLen);     
        _shm_head._tail += pkgLen;
        _shm_head._space_size += pkgLen;
    }
    else
    {
        void* buff = static_cast<char*>(_shm_buf) + _shm_head._tail;
        int tmpLen = _shm_head._max_space_size - 1 - _shm_head._tail;
        memcpy(buff, &item, tmpLen);
        memcpy((char*)buff+tmpLen, (char*)&item + tmpLen, pkgLen - tmpLen);
        _shm_head._tail += pkgLen;
        _shm_head._tail = _shm_head._tail % _shm_head._max_space_size + sizeof(ShmHead); 
        _shm_head._space_size += pkgLen;
    }

    _shm_head._item_num += 1;

    return 0;
}

typedef ShmQueue<MZMessage> ShmSendQueue;
typedef ShmQueue<MZMessage>* ShmSendQueuePtr;

typedef ShmQueue<MZMessage> ShmRecvQueue;
typedef ShmQueue<MZMessage>* ShmRecvQueuePtr;

#endif
