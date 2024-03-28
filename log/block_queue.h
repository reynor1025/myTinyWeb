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
    cond con_t;
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

    //队列是否为空
    bool is_empty(){
        m_mutex.lock();
        bool res = cer_size==0?true:false;
        m_mutex.unlock();
        return res;
    }
    //返回队首元素
    bool get_front(T &res){
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return false;
        }
        res = b_array[front];
        m_mutex.unlock();
        return true;
    }
    //返回队尾元素
    bool get_tail(T &res){
        m_mutex.lock();
        if (0 == m_size)
        {
            m_mutex.unlock();
            return false;
        }
        res = b_array[tail];
        m_mutex.unlock();
        return true;
    }

    //返回此时队列大小
    int get_size(){
        int temp = 0;
        m_mutex.lock();
        temp = cur_size;
        m_mutex.unlock();
        return temp;
    }

    int get_max_size(){
        int tmp = 0;

        m_mutex.lock();
        tmp = max_size;

        m_mutex.unlock();
        return tmp;
    }

    //当队列没有满的时候，向队列添加
    bool push(const T& val){
        m_mutex.lock();
        if (cur_size>=max_size)
        {
            //将所有使用队列的全唤醒
            con_t.broadcast();
            m_mutex.unlock();
            return false;
            /* code */
        }
        tail = (tail+1)%max_size;
        b_array[tail] = val;
        cur_size++;
        con_t.broadcast();
        m_mutex.unlock();
        return true;
    }

    //取出队列的元素
    bool pop(T& val){
        m_mutex.lock();
        while(cur_size<=0)
        {
            //如果con_t.wait()返回为false说明 没有
            //进程向队列中方东西，等待失败
            if (！con_t.wait(m_mutex.get()))
            {
                m_mutex.unlock()
                return false;
            }
            
        }
        //这里返回的是那一个元素呢？
        //1.返回当前队列的队首元素
        //val = b_array[front];
        front = (front+1)%max_size;
        //2.返回pop之后的队首,原作者给出的是第二种写法
        val = b_array[front];
        cur_size--;
        m_mutex.unlock();
        return true;
    }
    //加入时间限制的取操作
    bool pop(T&val, int ms_timeout){
        //如果有一个名为 timespec 的结构体类型，
        // 要在函数参数列表中声明一个指向该类型的指针参数，
        // 你需要写成 struct timespec *，其中 struct timespec 
        // 表示 timespec 结构体类型，* 表示指针类型。
        // 在C++中，这种限制不是必须的，
        // 因为C++允许在定义结构体时省略 struct 关键字，
        // 所以你可以直接写 timespec * 而不需要 struct timespec *。
        // 但在C语言中，由于历史原因，需要使用 struct timespec * 
        // 来表示指向 timespec 结构体的指针类型。
        

        //测试上诉描述对不对
        timespec tmsp = {0,0};
        timeval now = {0,0};
        gettimeofday(now,NULL);
        m_mutex.lock();
        if (cur_size<=0)
        {
            tmsp.tv_sec = now.tv_sec+ms_timeout/1000;
            tmsp.tv_nsec = (ms_timeout%1000)*1000;
            if(!con_t.timewait(m_mutex.get(),&tmsp)){
                m_mutex.unlock();
                return false;
            }
        }
        if (cur_size <= 0)
        {
            m_mutex.unlock();
            return false;
        }

        //这里返回的是那一个元素呢？
        //1.返回当前队列的队首元素
        //val = b_array[front];
        front = (front+1)%max_size;
        //2.返回pop之后的队首,原作者给出的是第二种写法
        val = b_array[front];
        cur_size--;
        m_mutex.unlock();
        return true;


    }


}




#endif