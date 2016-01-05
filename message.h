#ifndef _H_MESSAGE_h_
#define _H_MESSAGE_h_

#include <string>
using namespace std;

/*
 * @brief 包头
 *
 */
struct MZMessageHead
{
    int len;
    int version;
};

/*
 * @brief 包体
 *
 */
struct MZMessageBody
{
    char name[20];
    char desc[20];
};

/*
 * @brief 请求结构体
 *
 */
struct MZMessage
{
    MZMessageHead head;
    MZMessageBody body;
};

#endif
