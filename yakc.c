#include "yakk.h"

#define IDLE_PRIORITY 255

void YKIdleTask(void){
	while(1);
}
int YKIdleTaskStack[STACK_SIZE];
typedef struct taskblock *TCBptr;
typedef struct taskblock
{				/* the TCB struct definition */
    void *stackptr;		/* pointer to current top of stack */
    int state;			/* current state */
    int priority;		/* current priority */
    int delay;			/* #ticks yet to wait */
    TCBptr next;		/* forward ptr for dbl linked list */
    TCBptr prev;		/* backward ptr for dbl linked list */
}  TCB;

void YKInitialize(){
	YKCtxSwCount = 0; //Global variable tracking context switches
	YKIdleCount = 0;//Global variable used by idle task
	YKTickNum = 0; //Global variable incremented by tick handler 
	YKNewTask(&YKIdleTask,&YKIdleTaskStack,IDLE_PRIORITY);
}
void YKNewTask(void(*task)(void),void *taskStack, unsigned char priority){
	
}
void YKRun(){
}
void YKDelayTask(){
}
void YKEnterMutex(){
}
void YKExitMutex(){
}
void YKEnterISR(){
}
void YKExitISR(){
}
void YKScheduler(){
}
void YKDispatcher(){
}
void YKTickHandler(){
}
