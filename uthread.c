#include "uthread.h"
//by Li Weiheng for first Proj
THREAD_MUTEX_T mutex[MUTEX_MAX];

int initial=0;                  //initial times
int TotalthreadID = 0;
int signal_num = 0;          //condition variable
int threadID=0;                 //threadID

static ucontext_t scheduler_context;
static int first = 1;
static int numthread = 0;       //number of threads

struct sigaction preemption_handler;
uthread *initialThread, *mainThread, *newThread, *TempThread, *currentThread;

//set the function of preemption
static void signal_handler(int sig_nr)
{
//    printf("signal_handler working!\n");
    struct itimerval time;
    
    time.it_interval.tv_sec = 0;
    time.it_interval.tv_usec = TIME_INTERVAL;
    time.it_value.tv_sec = 0;
    time.it_value.tv_usec = TIME_INTERVAL;
    if (sig_nr == SIGPROF)
    {
        swapcontext(&currentThread->context,&scheduler_context);
        THREAD_YIELD();
    }
    else return;
        

}

/*This function manages mainthread*/
static void main_manager()
{
 //   printf("main_manager working!\n");
	mainThread->status = 1;
	setcontext(&mainThread->context);
    if(mainThread->exited != 1)
    {
        mainThread->finished = 1;
    }
	setcontext(&scheduler_context);
	return;
}

/*This function manages current thread and finish exited thread*/
static void thread_manager(void *func(), void *arg)
{
//    printf("thread_manager working!\n");
	currentThread->status = 1;
	currentThread->retval = func(arg);
	if(currentThread->exited!=1)
    {
        currentThread->finished=1;
    }
	setcontext(&scheduler_context);
	return;
}

/*The function to schedule next thread to run*/
static void scheduler()
{
 //   printf("scheduler working!\n");
	while(numthread != 0)
    {
	if(currentThread->finished != 1 && currentThread->exited != 1)
        {
        swapcontext(&scheduler_context,&currentThread->context);
	}
	if(numthread == 1 && mainThread->exited == 1)
        {
	exit(0);
        }
	if(currentThread->next != NULL)     /*schedule another thread unless there is more than one thread in the runnableList*/
	{	currentThread=currentThread->next;	}
     }
}

/*This function initialize the each value in a thread structure*/
uthread* initialer(uthread *initThread)
{
 //   printf("initialer working!\n");
    
    initThread = (uthread *)malloc(sizeof(uthread));
    getcontext(&initThread->context);
    
    initThread->threadID = (unsigned long int)TotalthreadID;
    initThread->status = 0;
    initThread->finished = 0;
    initThread->exited = 0;

    
    initThread->context.uc_link=&scheduler_context;
    initThread->context.uc_stack.ss_sp = malloc(STACK_SIZE);
    initThread->context.uc_stack.ss_size = STACK_SIZE;
    initThread->context.uc_stack.ss_flags = 0;
    initThread->next = NULL;
    return initThread;
}

//This function initialize a thread structure and set the signal and timer
void THREAD_INIT(long period)
{
  //  printf("THREAD_INIT working!\n");
    
    struct itimerval time;
  

	time.it_value.tv_usec = (long) period;
	time.it_interval = time.it_value;
        
	
	preemption_handler.sa_handler = signal_handler;
	preemption_handler.sa_flags = SA_RESTART | SA_SIGINFO;
	sigemptyset(&preemption_handler.sa_mask);

	sigaction(SIGPROF, &preemption_handler, NULL);
        
    mainThread=initialer(initialThread);
    setitimer(ITIMER_PROF, &time, NULL);

    currentThread = mainThread;
	makecontext(&mainThread->context, (void(*)(void)) main_manager, 0, NULL, NULL);
    TotalthreadID++;
	numthread++;

}

/*The function creates a new thread for the user and allocate the stack*/
int THREAD_CREATE(THREAD_ID *thread, void *(*func)(void *), void *arg)
{
 //   printf("THREAD_CREATE working!\n");
        
    if(initial==0) THREAD_INIT(1000);
    initial++;

	*thread = (unsigned long int)TotalthreadID;
    
    //when first called, create the main thread and scheduler thread
	if(TotalthreadID == 1)
    {//printf("enter the totalthreadID line!\n");
		
		*thread = (unsigned long int)TotalthreadID;
        
        newThread=initialer(initialThread);
		
		newThread->next = mainThread;
		mainThread->next = newThread;
		makecontext(&newThread->context, (void(*)(void)) thread_manager, 2, func, arg);
		currentThread = newThread;
		TotalthreadID++;
		numthread++;
        
		getcontext(&mainThread->context);
        
        //jump to the scheduler but only for once
		if(first){
		first=0;
		scheduler();
		}
		return 0;
        
	}
    else
    {
		uthread *tempThread = newThread;
		newThread = (uthread *)malloc(sizeof(uthread));
		getcontext(&newThread->context);
		newThread->threadID = (unsigned long int)TotalthreadID;
		newThread->status = 0;
		newThread->finished = 0;
		newThread->exited = 0;
		newThread->context.uc_link=&scheduler_context;
		newThread->context.uc_stack.ss_sp = malloc(STACK_SIZE);
		newThread->context.uc_stack.ss_size = STACK_SIZE;
		newThread->context.uc_stack.ss_flags = 0;
		tempThread->next = newThread;
		newThread->next = mainThread;
        
		makecontext(&newThread->context, (void(*)(void)) thread_manager, 2, func, arg);
        TotalthreadID++;
		numthread++;
		swapcontext(&currentThread->context,&scheduler_context);
		return 0;
	}
}

// to block the current thread, waiting for thread to exit
int  THREAD_JOIN(THREAD_ID thread, void **status)
{
//	printf("THREAD_JOIN working!\n");
	uthread *tempThread = mainThread;
	do
	{
	if(tempThread != NULL && tempThread->threadID == thread)  //test if thread exists
	break;
	tempThread = tempThread->next;
	}while(tempThread != mainThread && tempThread != NULL);
	
	if(tempThread != NULL)             ///*test if thread has already exited*/
    {
		if((tempThread->exited == 1 || tempThread->finished == 1))
        {
            if(status != NULL)
                *status = (void *)tempThread->retval;
            return 0;
        }
		while(tempThread->exited != 1 && tempThread->finished != 1){}
	}
	return -1;
}

//exit a thread
void THREAD_EXIT(void *arg)
{
 //   printf("THREAD_EXIT working!\n");
	currentThread->retval = arg;
	currentThread->exited=1;
	if(currentThread->threadID != mainThread->threadID)
	setcontext(&scheduler_context);
}

//to yield CPU to other threads
void THREAD_YIELD(void)
{
//printf("THREAD_YIELD working!\n");
    int flag = 0;
    getcontext(&currentThread->context);
    if(flag == 0)
    {
        flag = 1;
        setcontext(&scheduler_context);
    }
    return;
}

//to return the id for the current thread
int THREAD_SELF(void)
{
  //  printf("THREAD_SELF working!\n");
	return currentThread->threadID;
}

/*Set the initial value for the lock*/
int  THREAD_MUTEX_INIT(THREAD_MUTEX_T *mutex)
{
 //   printf("THREAD_MUTEX_INIT working!\n");
    if(mutex->lock == 1)return -1;
	mutex->lock = 0;
	mutex->owner = -1;
	return 0;
}

//to lock on mutex. 
int  THREAD_MUTEX_LOCK(THREAD_MUTEX_T *mutex)
{
//    printf("THREAD_MUTEX_LOCK working!\n");
	if((mutex->owner) == currentThread->threadID)
    {
        return -1;
    }
    while(mutex->lock !=0 && mutex->owner != currentThread->threadID)
    THREAD_YIELD();
    mutex->owner = currentThread->threadID;
    mutex->lock = 1;
    return 0;
}

//to release the lock
int  THREAD_MUTEX_UNLOCK(THREAD_MUTEX_T *mutex)
{
  //  printf("THREAD_MUTEX_UNLOCK working!\n");
	if(mutex->lock == 1 && mutex->owner == currentThread->threadID)
    {
        mutex->lock = 0;
        mutex->owner = -1;
        THREAD_YIELD();
        return 0;
    }

	return -1;
}

//to unblock a thread waiting on the condition variable
int  THREAD_COND_SIGNAL (THREAD_COND_T* cond)
{
   // printf("THREAD_COND_SIGNAL working!\n");
    signal_num = signal_num+1;
    TempThread->status=1;
    cond->status=1;
    THREAD_YIELD();

}

//to put the calling thread into a blocked status and wait for the signal
int  THREAD_COND_WAIT (THREAD_COND_T* cond, THREAD_MUTEX_T* mutex)
{
   // printf("THREAD_COND_WAIT working!\n");
    TempThread = currentThread;
    currentThread->status=0;
    while(1)
    {
        if(signal_num==1)
        {
            cond->status=1;
            mutex->lock=0;
            THREAD_MUTEX_LOCK(mutex);
            break;
        }
        else
        {
            cond->status=0;
            THREAD_YIELD();
            THREAD_MUTEX_UNLOCK(mutex);
        }
    }

}

//to unblock all threads waiting on the condition variable
int UTHREAD_COND_BROADCAST (THREAD_COND_T* cond)
{
  //  printf("UTHREAD_COND_BROADCAST working!\n");

    signal_num = signal_num+1;
    cond->all=1;
    THREAD_YIELD();
}

