
#ifndef YAKK_H_
#define YAKK_H_


// Global Variables shared with application code
extern unsigned YKCtxSwCount; //Global variable tracking context switches
extern unsigned YKIdleCount;  //Global variable used by idle task
extern unsigned YKTickNum;    //Global variable incremented by tick handler 

// Kernel functions
void YKInitialize();
void YKNewTask(void(*task)(void),void *taskStack, unsigned char priority);
void YKRun();	
void YKDelayTask();
void YKEnterMutex();
void YKExitMutex();
void YKEnterISR();
void YKExitISR();
void YKScheduler(char blocking);
void YKTickHandler();
void YKIdleTask(void);

#endif
