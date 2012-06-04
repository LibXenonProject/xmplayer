#include <stdio.h>
#include <stdlib.h>
#include <debug.h>
#include <errno.h>
//#include "pthread.h"

#include <ppc/atomic.h>
#include <xetypes.h>
#include <ppc/register.h>
#include <ppc/xenonsprs.h>

#include <xenon_soc/xenon_power.h>


#define NB_THREAD 6

#define DBG_TR TR

//#define USE_NAT_THREAD

#ifdef USE_NAT_THREAD
#include <threads/threads.h>
#include <threads/mutex.h>
#include <threads/cond.h>
#include <network/network.h>
#include <threads/gdb.h>

typedef PTHREAD pthread_t;

void pthread_init(){
	xenon_make_it_faster(XENON_SPEED_FULL);
	threading_init();
	//network_init_sys();
	//gdb_init();
}
#if 0

typedef MUTEX*  pthread_mutex_t;
int pthread_mutex_destroy(pthread_mutex_t * mutex){
	mutex_destroy(mutex[0]);
	return 0;
};
int pthread_mutex_init(pthread_mutex_t * mutex, void * u){
	mutex[0] = mutex_create(1);
	return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mutex){
	//TR;
	mutex_acquire(mutex[0],INFINITE);
	//TR;
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
	//TR;
	mutex_release(mutex[0]);
	//TR;
	return 0;
};

typedef MUTEX* pthread_cond_t;

// cond
int pthread_cond_broadcast(pthread_cond_t *cond){
	//mutex_release(cond[0]);
	return 0;
};
int pthread_cond_destroy(pthread_cond_t * cond){
	//mutex_destroy(cond[0]);
	return 0;
};
int pthread_cond_init(pthread_cond_t * cond, void * u){
	//cond[0] = mutex_create(1);
	return 0;
};
int pthread_cond_signal(pthread_cond_t * cond){
	//mutex_release(cond[0]);
	return 0;
};

int pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex){
	//TR;
	mutex_release(mutex[0]);
	//TR;
	mutex_acquire(mutex[0],INFINITE);
	//TR;
	return 0;
};
#else

typedef unsigned int __attribute__ ((aligned (128))) pthread_cond_t;
typedef unsigned int __attribute__ ((aligned (128))) pthread_mutex_t;

int pthread_mutex_destroy(pthread_mutex_t * mutex){
	mutex[0]=0;
	return 0;
};
int pthread_mutex_init(pthread_mutex_t * mutex, void * u){
	//mutex= mutex+0x20000000;
	mutex[0]=0;
	return 0;
}

int pthread_mutex_lock(pthread_mutex_t *mutex){
	lock((void*)mutex);
/*
	while(mutex[0]==1);
	mutex[0]=1;
*/
	return 0;
};

int pthread_mutex_unlock(pthread_mutex_t *mutex){
	unlock((void*)mutex);
/*
	mutex[0]=0;
*/
	return 0;
};


// cond
int pthread_cond_destroy(pthread_cond_t * cond){
	//TR;
	cond[0]=0;
	return 0;
};
int pthread_cond_init(pthread_cond_t * cond, void * u){
	//TR;
	cond[0]=0;
	return 0;
};

int pthread_cond_broadcast(pthread_cond_t *cond){
	return 0;
};

int pthread_cond_signal(pthread_cond_t * cond){
	return 0;
};

int pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex){	
	pthread_mutex_unlock(mutex);
	pthread_mutex_lock(mutex);
	
	return 0;
};
#endif

static int thread_n[] = {
	1,2,3,4,5
};

int pthread_create(pthread_t *thread, void *u,
    void *(*start_routine)(void*), void *arg){
	
	PTHREAD curThread = NULL;
	
	static int last_thread_id =0;
	if(last_thread_id>=NB_THREAD){
		last_thread_id = 0;
	}
	
	curThread = thread_create(start_routine,0,arg,THREAD_FLAG_CREATE_SUSPENDED);
	thread_set_processor(curThread,thread_n[last_thread_id]);
	//thread_set_priority(curThread,15);
	thread_resume(curThread);
	
	printf("New thread on %d\r\n",thread_n[last_thread_id]);
	
	last_thread_id++;	
		
	*thread = curThread;
		
	return 0;
}

int pthread_join(pthread_t thread, void **value_ptr){
	while(!thread->ThreadTerminated);
	return 0;
}

#else // CLASSIC THREAD

typedef unsigned int __attribute__ ((aligned (128))) pthread_cond_t;
typedef unsigned int __attribute__ ((aligned (128))) pthread_mutex_t;
typedef unsigned int pthread_t;

void pthread_init(){
	xenon_make_it_faster(XENON_SPEED_FULL);
//	threading_init();
}

int pthread_mutex_destroy(pthread_mutex_t * mutex){
	mutex[0]=0;
	return 0;
};
int pthread_mutex_init(pthread_mutex_t * mutex, void * u){
	//mutex= mutex+0x20000000;
	mutex[0]=0;
	return 0;
}
int pthread_mutex_lock(pthread_mutex_t *mutex){
	lock((void*)mutex);
	return 0;
};

int pthread_mutex_unlock(pthread_mutex_t *mutex){
	unlock((void*)mutex);
	return 0;
};


// cond
int pthread_cond_destroy(pthread_cond_t * cond){
	//TR;
	cond[0]=0;
	return 0;
};
int pthread_cond_init(pthread_cond_t * cond, void * u){
	//TR;
	cond[0]=0;
	return 0;
};

int pthread_cond_broadcast(pthread_cond_t *cond){
	//TR;
	//unlock(cond);
	return 0;
};

int pthread_cond_signal(pthread_cond_t * cond){
	//TR;
	//unlock(cond);
	return 0;
};

int pthread_cond_wait(pthread_cond_t * cond, pthread_mutex_t * mutex){
	//TR;
	//lock(cond);
	
	unlock((void*)mutex);
	lock((void*)mutex);
	
	return 0;
};

typedef void *(*xenon_thread_func)(void*);

static xenon_thread_func func_stack[6];
static void * args_stack[6];
static unsigned char stack[6 * 0x10000];

static void thread_runner(void){
	void *args=args_stack[mfspr(pir)];
	xenon_thread_func func = func_stack[mfspr(pir)];	
	func(args);	
}

//#define STABLE

int pthread_create(pthread_t *thread, void *u,
    void *(*start_routine)(void*), void *arg){
#ifdef STABLE
	static int last_thread_id = 2;
	if(last_thread_id>=NB_THREAD){
		last_thread_id = 2;
	}
#else
	static int last_thread_id = 1;
	if(last_thread_id>=NB_THREAD){
		last_thread_id = 1;
	}
#endif
	printf("New thread on %d\r\n",last_thread_id);
	
	args_stack[last_thread_id]=arg;
	func_stack[last_thread_id]=start_routine;
	
	xenon_run_thread_task(last_thread_id,stack + (last_thread_id * 0x10000) - 0x100,thread_runner);
	
	thread[0]=last_thread_id;
	
#ifdef STABLE
	last_thread_id+=2;
#else
	last_thread_id++;
#endif
	return 0;
}

int pthread_join(pthread_t thread, void **value_ptr){
	while(xenon_is_thread_task_running(thread));
	return 0;
}


#endif