#ifndef LOCKER_H
#define LOCKER_H

#include<exception>
#include<pthread.h>
#include<semaphore.h>
//semaphore.h 是信号量的类包

//类sem是同步线程类
class sem{
public:
	//空构造函数，类初始化的时候调用
	sem(){
	//int sem_init __P ((sem_t *__sem, int __pshared, unsigned int __value));　
	//__pshared为0 的时候，意味这该信号量只能被该线程共享
	//__value给信号量赋值
		if(sem_init(&m_sem,0,0) != 0){
			throw std::exception();
		}
	}
	
	//带参的构造函数，给信号量赋初始值
	sem(int num){
		if(sem_init(&m_sem,0,num) != 0){
			throw std::exception();
		}
	}
	
	~sem(){
		sem_destroy(&m_sem);
	}

	bool wait(){
		//sem_wait函数也是一个原子操作，它的作用是从信号量的值减去一个“1”，
		//但它永远会先等待该信号量为一个非零值才开始做减法。
		return sem_wait(&m_sem) == 0;
	}
	bool post(){
		return sem_post(&m_sem) == 0;
	}
private:

//在这里定义一个信号量
sem_t m_sem;//sem_t 是信号量类型

};
class cond{
	public:
	cond(){
		if(pthread_cond_init(&pth_t,NULL)!=0){
			throw std::exception();
		}
	}
	~cond(){
		pthread_cond_destroy(&pth_t);
	}
	//释放占有信号量
	bool signal(){
		return pthread_cond_signal(&pth_t);
	}

	//等待信号量
	bool wait(pthread_mutex_t *m_mutex){

		int ret = pthread_cond_wait(&pth_t，m_mutex);
		return ret == 0;
	}

	//释放所有信号量                
	bool broadcast(){
		return pthread_cond_broadcast(&pth_t);
	}

	//等待
	bool timewait(pthread_mutex_t *m_mutex,struct timespec *abstime){
		//int pthread_cond_timedwait(pthread_cond_t *cv,
		//    pthread_mutex_t *mp, const struct timespec * abstime);
		int ret = pthread_cond_timedwait(&th_t,m_mutex,&abstime);
		return ret ==0;

	}


	private:
	pthread_cond_t pth_t;
}

class locker
{
private:
	/* data */
	pthread_mutex_t phm_t;
	
public:
	locker(){
		if (pthread_mutex_init(&phm_t,NULL)!=0)
		{
			throw std::exception();
			/* code */
		}
		
	}
	~locker(){

		pthread_mutex_destroy(&phm_t);
	}

	bool lock(){
		
		return pthread_mutex_lock(&phm_t)==0;
	}

	bool unlock(){
		return pthread_mutex_unlock(&phm_t)==0;
	}

	pthread_mutex_t *get_m(){
		return &phm_t;
	}

};

#endif
