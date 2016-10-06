
#ifndef YAKK_H_
#define YAKK_H_


// Global Variables shared with application code
extern uint32_t YKCtxSwCount; //Global variable tracking context switches
extern uint32_t YKIdleCount;  //Global variable used by idle task
extern uint32_t YKTickNum;    //Global variable incremented by tick handler 

// Kernel functions
void YKInitialize();
void YKNewTask(void(*task)(void),void *taskStack, unsigned char priority);
void YKRun();	
void YKDelayTask();
void YKEnterMutex();
void YKExitMutex();
void YKEnterISR();
void YKExitISR();
void YKScheduler();
void YKDispatcher();
void YKTickHandler();


