// JNI内部消息传递实体
// Created by jz on 19-8-10.
//

#ifndef MYPLAYER_JNIMESSAGE_H
#define MYPLAYER_JNIMESSAGE_H

#include <stdint.h>
#include "Mutex.h"
#include "Condition.h"
#include "AndroidLog.h"
extern "C" {
#include <libavutil/mem.h>
};

typedef struct Msg{
    int what;
    char* message;
    Msg* next;
}JNIMessage;

class MessageQueue{
    public:
        MessageQueue();
        ~MessageQueue();
        enum TYPE_READ{
            BLOCK = 0,
            UNBLOCK = 1
        };

        int getMessage(JNIMessage* msg);                  //非阻塞式方式读取，没有数据就立即返回
        int getMessage(JNIMessage* msg, TYPE_READ block);       //阻塞方式读取
        void flush();

        int addMessage(int what, const char* str);


private:
    JNIMessage* first;     //队首取 队尾添加数据
    JNIMessage* last;
    Mutex mMutex;
    Condition mCond;
    int size;
};


#endif //MYPLAYER_JNIMESSAGE_H
