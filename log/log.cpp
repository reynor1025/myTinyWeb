#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "log.h"
#include <pthread.h>
using namespace std;

log::log(){
    m_log_index = 0;
    is_syn_signal = false;
}

log::~log(){
    if(m_fp!=NULL){
        fclose(m_fp);
    }
}

log::init(const char* mlogname,int mclose_log, int log_buf_size = 8192,int split_lines = 5000000, int max_queue_size = 0){
    //如果设置max_queue_size不为0,说明是异步调用
    if(max_queue_size>=0){
        is_syn_signal = true;
        //设置队列大小
        m_log_queue = new block_queue<string>(max_queue_size);
        //创建子线程实现
        pthread td;
        pthread_create(&td,NULL,async_write_log,NULL);
    }

    close_log = mclose_log;

    m_log_buff_size = log_buf_size;

    m_split_lines = split_lines;

    buff = new char[log_buf_size];
    memset(buff, '\0', m_log_buff_size);

    //获取时间
    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    //获取文件名称
    const char *p = strrchr(mlogname, '/');
    char log_full_name[256] = {0};
    if (p==NULL)
    {
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, mlogname);
        /* code */
    }
    else
    {
        strcpy(logname, p + 1);
        strncpy(pathname, mlogname, p - mlogname + 1);
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", pathname, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, logname);
    }

    day_count = my_tm.tm_mday;
    
    m_fp = fopen(log_full_name, "a");
    if (m_fp == NULL)
    {
        return false;
    }

    return true;
}

log::log_write(int level, const char *format, ...){

    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    char s[16] = {0};

    switch (level)
    {
    case level==0:
        strcpy(s, "[debug]:");
        break;
    case level==1:
        strcpy(s, "[info]:");
        break;
    case level==2:
        strcpy(s, "[warn]:");
        break;
    case level==3:
        strcpy(s, "[error]:");
        break;
    default:
        strcpy(s, "[info]:");
        break;
    }
    
    //获取锁
    log_lock.lock();
    day_count++;
    if(day_count!=my_tm.tm_mday||m_log_index%m_log_index==0){
    
        char newlog[256] = {0};
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
        if (day_count != my_tm.tm_mday)
        {
            snprintf(new_log, 255, "%s%s%s", pathname, tail, logname);
            day_count = my_tm.tm_mday;
            day_count = 0;
        }
        else
        {
            snprintf(new_log, 255, "%s%s%s.%lld", pathname, tail, logname, m_log_index / m_log_index);
        }
        m_fp = fopen(new_log, "a");
    }
    log_lock.unlock();

    va_list valst;
    va_start(valst, format);
    string log_str;
    log_lock.lock();

    //写入的具体时间内容格式
    int n = snprintf(buff, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    
    int m = vsnprintf(buff + n, m_log_buff_size - n - 1, format, valst);
    buff[n + m] = '\n';
    buff[n + m + 1] = '\0';
    log_str = buff;

    log_lock.unlock();

    if (is_syn_signal && !m_log_queue->full())
    {
        m_log_queue->push(log_str);
    }
    else
    {
        log_lock.lock();
        fputs(log_str.c_str(), m_fp);
        log_lock.unlock();
    }

    va_end(valst);

}

log::flush(void){
    log_lock.lock();
    //强制刷新写入流缓冲区
    fflush(m_fp);
    log_lock.unlock();

}