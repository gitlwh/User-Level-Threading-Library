#include "uthread.h"
//by Li Weiheng for first Proj


void* testfunc1(void*);
int main()
{

    THREAD_ID tid[32];
struct timespec time1 ={0,0};
struct timespec time2 ={0,0};
struct timespec time3 ={0,0};
struct timespec time4 ={0,0};
clock_gettime(CLOCK_REALTIME,&time1);
    // Test #1: how long does it take for one thread to run?
    
    THREAD_CREATE(&tid[0], testfunc1, NULL);
    THREAD_JOIN(tid[0], NULL);
    clock_gettime(CLOCK_REALTIME,&time2);
    printf("time for one thread: %d, %d\n",(int)(((time2.tv_nsec-time1.tv_nsec)<0)?(time2.tv_sec-time1.tv_sec-1):(time2.tv_sec-time1.tv_sec)), (int)(((time2.tv_nsec-time1.tv_nsec)<0)?(time2.tv_nsec-time1.tv_nsec+1000000000):(time2.tv_nsec-time1.tv_nsec)));

    // Test #2: how long does it take for four threads to run?

    int i;
    for (i = 0; i < 4; ++i) {
        THREAD_CREATE(&tid[i], testfunc1, NULL);
    }
    for (i = 0; i < 4; ++i) {
        THREAD_JOIN(tid[i], NULL);
    }
    clock_gettime(CLOCK_REALTIME,&time3);
    printf("time for four thread: %d, %d\n",(int)(((time3.tv_nsec-time2.tv_nsec)<0)?(time3.tv_sec-time2.tv_sec-1):(time3.tv_sec-time2.tv_sec)), (int)(((time3.tv_nsec-time2.tv_nsec)<0)?(time3.tv_nsec-time2.tv_nsec+1000000000):(time3.tv_nsec-time2.tv_nsec)));
}

/**
 *  Increment a counter that is in a cache line on my stack
 */
void* testfunc1(void* params)
{
    int pad1[64];
    volatile int counter = 0;
    int pad2[64];
    int i;
    for (i = 0; i < 1024*32; ++i)
        ++counter;
}
