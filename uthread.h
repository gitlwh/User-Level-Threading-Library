#include <sys/time.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <signal.h>
#include <ucontext.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
//by Li Weiheng for first Proj
#define STACK_SIZE 10000
#define MUTEX_MAX 5


typedef int THREAD_ID;
typedef struct uthread{
	THREAD_ID threadID;     //thread id
	ucontext_t context;     //thread context
	struct uthread *next;   //used to link threads
	int status;              //the state of the thread
	int finished;
	int exited;
	void *retval;
} uthread;

void THREAD_INIT(long period);
int  THREAD_CREATE(THREAD_ID *thread, void *(*start_routine)(void *), void *arg);
int  THREAD_JOIN(THREAD_ID thread, void **status);
void THREAD_EXIT(void *retval);
void THREAD_YIELD(void);
int THREAD_SELF(void);


typedef struct THREAD_MUTEX_T{
    long lock;
    THREAD_ID owner;
}THREAD_MUTEX_T;


int  THREAD_MUTEX_INIT(THREAD_MUTEX_T *mutex);
int  THREAD_MUTEX_LOCK(THREAD_MUTEX_T *mutex);
int  THREAD_MUTEX_UNLOCK(THREAD_MUTEX_T *mutex);

#define THREAD_MUTEX_INITIALIZER  {0, -1}

//Define states of condition variable
#define SATISFIED 1
#define FAILURE 2

typedef struct THREAD_COND_T {
	THREAD_MUTEX_T *mutex;
	int status;
    int all;
}THREAD_COND_T;
int  THREAD_COND_SIGNAL (THREAD_COND_T *cond);
int  THREAD_COND_WAIT (THREAD_COND_T *cond, THREAD_MUTEX_T *mutex);
int  UTHREAD_COND_BROADCAST (THREAD_COND_T *cond);

