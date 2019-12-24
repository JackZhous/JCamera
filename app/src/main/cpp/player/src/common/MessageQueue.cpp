//
// Created by jz on 19-8-10.
//

#include "MessageQueue.h"

MessageQueue::MessageQueue() {
    first = NULL;
    last = NULL;
    size = 0;
}

MessageQueue::~MessageQueue() {
    flush();
    first = NULL;
    last = NULL;
}

int MessageQueue::getMessage(JNIMessage *msg) {
    return getMessage(msg, BLOCK);
}

int MessageQueue::getMessage(JNIMessage *msg, MessageQueue::TYPE_READ block) {
    if(msg == NULL){
        LOGE("MessageQueue method getMessage paras 1 is NULL");
        return -1;
    }
    JNIMessage *temp;
    mMutex.lock();

    for(;;){
        if(first == NULL){
            if(block == BLOCK){
                mCond.wait(mMutex);      //等待100ms
                continue;
            } else{
                return -1;
            }
        }

        *msg = *first;
        temp = first;
        first = first->next;
        av_free(temp);
        //这里更新尾巴节点
        if(first == NULL){
            last = NULL;
        }
        break;
    }
    size--;
    mMutex.unlock();
    return 0;
}

int MessageQueue::addMessage(int what, const char* str) {
    AutoMutex autoMutex(mMutex);

    JNIMessage* message = (JNIMessage*)av_malloc(sizeof(JNIMessage));
    if(message == NULL){
        return -1;
    }
    message->what = what;
    message->message = strdup(str);
    if(last == NULL){
        last = message;
        first = last;
    } else{
        last->next = message;
        last = message;
    }
    last->next = NULL;
    size++;
    mCond.signal();
    return 0;
}


void MessageQueue::flush() {
    JNIMessage* temp = NULL;
    while (first != NULL){
         temp = first;
         first = first->next;
         free(temp->message);
         av_free(temp);
    }
}