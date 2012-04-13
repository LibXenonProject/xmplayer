#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <errno.h>
//#include "pthread.h"

#include <threads/threads.h>
#include <threads/mutex.h>
#include <threads/cond.h>

//typedef COND* pthread_cond_t;
typedef MUTEX* pthread_cond_t;
typedef MUTEX* pthread_mutex_t;
typedef PTHREAD pthread_t;

#define NB_THREAD 6

#define DBG_TR TR

int pthread_mutex_destroy(pthread_mutex_t * mutex){
	mutex_destroy(mutex[0]);
	return 0;
};
int pthread_mutex_init(pthread_mutex_t * mutex, void * u){
	mutex[0] = mutex_create(1);
	return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mutex){
	mutex_acquire(mutex[0],INFINITE);
	return 0;
};
int pthread_mutex_trylock(pthread_mutex_t *mutex){
	// check timeout
	if(mutex_acquire(mutex[0],0)==0){
		return EBUSY;
	}
	return 0;
};
int pthread_mutex_unlock(pthread_mutex_t *mutex){
	mutex_release(mutex[0]);
	return 0;
};

// cond
int pthread_cond_broadcast(pthread_cond_t *cond){
	cond_broadcast(cond[0]);
	//mutex_release(cond[0]);
	return 0;
};
int pthread_cond_destroy(pthread_cond_t * cond){
	cond_delete(cond[0]);
	//mutex_destroy(cond[0]);
	return 0;
};
int pthread_cond_init(pthread_cond_t * cond, void * u){
	cond[0] = cond_create();
	//mutex_acquire(cond[0],INFINITE);
	//cond[0] = mutex_create(1);
	return 0;
};
int pthread_cond_signal(pthread_cond_t * cond){
	cond_signal(cond[0]);
	//mutex_release(cond[0]);
	return 0;
};

int pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex){
	TR
	cond_wait(cond[0],mutex[0]);
	// HACK !!!
/*
	mutex_release(mutex[0]);
	mutex_acquire(cond[0],INFINITE);
	mutex_release(cond[0]);
	mutex_acquire(mutex[0],INFINITE);
*/
	return 0;
};

int pthread_create(pthread_t *thread, void *u,
    void *(*start_routine)(void*), void *arg){
	
	static int last_thread_id = 0;
	if(last_thread_id>=NB_THREAD){
		last_thread_id = 0;
	}
	
	thread[0] = thread_create(start_routine,0,arg,THREAD_FLAG_CREATE_SUSPENDED);
	thread_set_processor(thread[0],last_thread_id);
	thread_resume(thread[0]);
	
	
	last_thread_id++;
		
	return 0;
}

int pthread_join(pthread_t thread, void **value_ptr){
	while(!thread->ThreadTerminated);
	return 0;
}

