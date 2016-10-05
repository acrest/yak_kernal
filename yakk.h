
#ifndef YAKK_H_
#define YAKK_H_


//Global Variables
uint32_t YKCtxSwCount; //Global variable tracking context switches
uint32_t YKIdleCount;//Global variable used by idle task
uint32_t YKTickNum; //Global variable incremented by tick handler 

//Functions
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


