#include "share_mem.h"
#include <iostream>
#include <errno.h>
#include <string.h>
using namespace std;

ShareMem::ShareMem(int32_t num, int32_t size) : _shmid(0), _max_item_num(num), _size(size), _shm_buf(NULL)
{}

int32_t ShareMem::get(const char* key_file)
{
    key_t shm_key = ftok(key_file, 1);
    int32_t mem_size = _size; 
    _shmid = shmget(shm_key, mem_size, IPC_CREAT|0666);

    if(_shmid < 0)
    {
        return -1;
    }

    return 0;
}

int32_t ShareMem::destroy()
{
    if( _shmid > 0)
    {
        return shmctl(_shmid, IPC_RMID, 0); 
    }

    return 0;
}

int32_t ShareMem::attach()
{
    _shm_buf = shmat(_shmid, 0, 0);
    cout<<"shm_buf="<<_shm_buf<<endl;
    cout<<strerror(errno)<<endl;

    if(_shm_buf < (char*)0)
    {
        return -1;
    }

    return 0;
}

int32_t ShareMem::dettach()
{
    return shmdt(_shm_buf);
}

int32_t ShareMem::ctrl()
{
    return 0;
}

void ShareMem::status()
{
    cout<<"share mem status:"<<endl;
    cout<<"shmid="<<_shmid<<endl;
    cout<<"shmbuf="<<_shm_buf<<endl;
    cout<<"size"<<_size<<endl;
    cout<<"item="<<_max_item_num<<endl;
}
