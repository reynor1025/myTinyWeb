/*************************************************************
*循环数组实现的阻塞队列，m_back = (m_back + 1) % m_max_size;  
*线程安全，每个操作前都要先加互斥锁，操作完后，再解锁
**************************************************************/

#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H
#include<sys/time.h>
#include<iostream>
#include<../locker/locker.h>
#include<pthread.h>
#include<stdlib.h>

using namespace std;

template<class T>

class block_queue{
    private:
    locker m_mutex;
    pthread_cond_t con_t;
    int max_size;
    int cur_size;
    int front;
    int tail;
    T b_array[];

    public:
    block_queue(int sizes =1000){
        if (sizes<0)
        {
            exit(-1);
            /* code */
        }
        
        this->max_size = sizes;
        front = -1;
        tail = -1;
        cur_size = 0;
        b_array = new T[sizes];
    }
    ~block_queue(){
        m_mutex.lock();
        if (b_array!=NULL)
        {
            delete []b_array;
        }
        m_mutex.unlock();
    }
    //清空队列
    void clear(){
        m_mutex.lock();
        cur_size = 0;
        front = -1;
        tail = -1;
        m_mutex.unlock();
    }

    //队列盘满
    bool is_full(){
        m_mutex.lock();
        bool res = cur_size>=max_size?true:false;
        m_mutex.unlock();
        return res;
    }

    //


}




#endif