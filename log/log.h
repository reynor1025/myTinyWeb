#ifdef LOG_H
#define LOG_H
#include<./block_queue.h>
#include<../locker/locker.h>
class log
{
private:
    /* data */
    //路径名称
    char pathname[128];

    //日志名称
    char logname[128];

    //日志最大行数
    int m_split_lines;

    //日志的行数
    long long m_log_index;

    //记录天数
    int day_count;

    //文件指针
    File *m_fp;

    //数据缓存
    char * buff;

    //缓存区大小
    int m_log_buff_size;

    //锁机制

    locker log_lock;

    //阻塞队列
    block_queue<string> * m_log_queue;

    //同步信号
    bool is_syn_signal;

    //是否关闭日志
    int close_log;

     void *async_write_log()
    {
        string single_log;
        //从阻塞队列中取出一个日志string，写入文件
        while (m_log_queue->pop(single_log))
        {
            log_lock.lock();
            fputs(single_log.c_str(), m_fp);
            log_lock.unlock();
        }
    }
    log(/* args */);
    ~log();


public:
    
    static log *get_instance(){
        static log log_instance;
        return &log_instance;
    }

    void init(const char* mlogname,int mclose_log, int log_buf_size = 8192,int split_lines = 5000000, int max_queue_size = 0);
    void log_write(int level, const char *format, ...);
    void flush(void);

    
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif