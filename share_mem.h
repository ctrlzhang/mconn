#ifndef _SHARE_MEM_H_
#define _SHARE_MEM_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

/*
 * @brief 共享内存类
 *
 */
class ShareMem
{
    public:
        //待分配的共享内存大小 
        ShareMem(int32_t num, int32_t item_size);

        virtual ~ShareMem()
        {
            if(_shmid > 0)
            {
                dettach();
                destroy();
            }
        }

    public:
        //创建共享内存
        int32_t get(const char* key_file);

        //删除共享内存
        int32_t destroy();

        //映射共享内存到进程空间
        int32_t attach();

        //分离共享内存
        int32_t dettach();

        //操作
        int32_t ctrl();

        //显示共享内存的信息
        void status();

    protected: 
        //共享内存id
        int32_t _shmid;

        //共享内存上限
        int32_t _max_item_num;

        //共享内存大小
        int32_t _size;

        //共享内存在进程中的地址
        void* _shm_buf;
};

#endif
