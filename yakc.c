/* yakc.c : c code for kernel implementation */

/* QUESTIONS
 * 1) How should we handle interrupts in the context of kernel functions.
 *    For example, do we want to disable interrupts when we create a new
 *    task? It seems like once we start creating a new task, we would want
 *    to make sure it gets completely created and placed in the TCB without
 *    something else interrupting it. Right? Cause if we did service an
 *    interrupt, the interrupt would call the scheduler, which would call
 *    the dispatcher, which would cause some task to execute. In that case
 *    our new task might never get added. And with that in mind, would 
 *    we want to make sure that the scheduler and dispatcher can't be 
 *    interrupted either? Are there certain kernel functions, or chains of
 *    kernel functions, that should be completely enclosed in a mutex? Are
 *    there kernel functions that we don't care whether they get interrupted?
		-So far in this lab we don't have to worry about this
 * 
 * 2) What does it mean for a task to be suspended? How does a task become
 *    suspended? How does a task become unsuspended, and what state should
 *    it be in when it does?
		- It should be suspended when another task is called before the 
		current task is finished.  It will restore to the state that it was
		when it was suspended and it should get that by going to the stack.
 *
 * 3) What's the correct/best way to call assembly functions from c code?
		- We've got it called correctly, when it's combined in the bin file
		It will be able to call it from the .s file
 *
 * 4) Do we want all of our YK prototype functions in yakk.h, or only the 
 *    ones we want the task code to be able to call?
 *
 * 5) Is SP always supposed to be pointing to the top of the current stack?
 *
 * 6) Will the scheduler ever be called from an ISR? Or can we assume that
 *    the ISR will always return
 */

#include "yakk.h"
#include "yaku.h"


// ******** DEFINITIONS ******** //
#define NULL 0
#define IDLE_PRIORITY 100
#define TASK_STATE_RUNNING   1
#define TASK_STATE_READY     2
#define TASK_STATE_DELAYED   3
#define TASK_STATE_SUSPENDED 4


typedef struct taskblock* Tptr;
/* the TCB struct definition */
typedef struct taskblock {				
    void *stackptr;	// pointer to current top of stack
    int state;			// current state
    int priority;		// current priority
    int delay;			// #ticks yet to wait
    Tptr next;      // Next task in list
} task;


// Assembly functions, defined in yaks.s
void YKEnterMutex(void);
void YKExitMutex(void);
void YKDispatcher(char blocking);

// ------------------------------------------------------ //

// ******** GLOBAL VARIABLES ******** //
// These will be used by application code as well
unsigned YKCtxSwCount; // Global variable tracking context switches
unsigned YKIdleCount;  // Global variable used by idle task
unsigned YKTickNum;    // Global variable incremented by tick handler 

// FLAGS
// Should be set by a blocking function and unset by the dispatcher only
char kernel_running_flag=0;

// Number of nested ISRs
unsigned char isrDepth;

// Stack frame for the idle task
int YKIdleTaskStack[STACK_SIZE];
int taskCount;
// TCB: array of task structs. This array is declared in order to allocate
// space for each task struct. In usage, each struct will be treated like a
// node in a linked list.
task TCBArray[MAX_NUM_TASKS + 1];
Tptr readyLstHead;
Tptr suspLstHead;
unsigned int nextTaskIndex = 0;
char called_from_blocking_function_flag;

// Pointer to task that was the last to run
Tptr lastRunningTask = NULL;

// ------------------------------------------------------ //

// ******* KERNEL FUNCTIONS ******** //
void YKInitialize() {
  // Here we disable interrupts. They will get turned on again after the 
  // dispatcher runs for the first time.
  YKEnterMutex();
  YKCtxSwCount = 0; // Context switch count
  YKIdleCount  = 0; // Iterations of the idle task
  YKTickNum    = 0; // Number of tick interrupts
  taskCount = 0;
  readyLstHead = NULL;
 	suspLstHead  = NULL;
 	called_from_blocking_function_flag = 0;
	//lastRunningTask = &TCBArray[0];
  YKNewTask(YKIdleTask, &YKIdleTaskStack[STACK_SIZE], IDLE_PRIORITY);
	readyLstHead->state = TASK_STATE_RUNNING;
}

void YKNewTask(void(*taskCode)(void), void *taskStack, unsigned char priority) {
  // NOTE: this function can never be called from an interrupt handler. That
  // implies to me that we don't have to worry about the list pointers
  // getting mixed up?

  // NOTE: when a task is first created, we need to somehow indicate at what
  // instruction it is going to start executing. I see two ways to do that.
  // First, we could store values for CS and IP in the TCB. Second, we could
  // push them on at the top of the stack frame. I'm thinking we should push
  // them on the stack frame, since that's how it is going to be in all other
  // contexts
  //taskCount++;
  unsigned int index;
  Tptr t;
  int* sp;
  Tptr t_iter;
  // We disable interrupts here. WE DON'T WANT THEM ON UNTIL THE DISPATCHER
  // IS DONE. They will get re-enabled when the iret instruction restores
  // the flags for a task, or by an ISR.
	YKEnterMutex();
  // Read and update the nextTaskIndex
  index = nextTaskIndex;
  nextTaskIndex++;

  t = &TCBArray[index];
  // Here we set up the initial stack frame. We're simulating pushing the
  // the flags, CS, and IP onto the stack, so that later they can be restored
  // using the iret instruction. We also put default values for all the
  // registers, so they can be popped off as well.
  sp = (int*)taskStack;
  sp -= 12;
  *(sp+11) = 0x200; // flags
  *(sp+10) = 0x0;   // CS
  *(sp+9)  = (int)taskCode; // IP
  *(sp+8)  = 0x0;   // AX
  *(sp+7)  = 0x0;   // BX
  *(sp+6)  = 0x0;   // CX
  *(sp+5)  = 0x0;   // DX
  *(sp+4)  = 0x0;   // SI
  *(sp+3)  = 0x0;   // DI
  *(sp+2)  = (int)((int*)taskStack -1);   // BP
  *(sp+1)  = 0x0;   // DS
  *(sp+0)  = 0x0;   // ES

  // Set up the task struct for this task in the TCB
  t->stackptr = (void*)sp;
  t->state    = TASK_STATE_READY;
  t->priority = priority;
  t->delay    = 0;       // default
  t->next     = NULL;    // default

  // Handle the insertion and maintanance of the linked list. Tasks are
  // inserted into the linked list based on priority level; the list should
  // always be sorted from highest to lowest priority. Note that smaller
  // numbered priorities are "higher" priority.
  if (readyLstHead == NULL)
    readyLstHead = t;
  else if (readyLstHead->priority > t->priority) {
    t->next = readyLstHead;
    readyLstHead = t;
  }
  else {
    t_iter = readyLstHead;
    while (t_iter->next)
      if (t_iter->next->priority > t->priority)
        break;
      else
        t_iter = t_iter->next;
    t->next = t_iter->next;
    t_iter->next = t;
  }
  taskCount++;

//printString("the new task has priority ");
//printInt(t->priority);
//printNewLine();

  // Call the scheduler
  YKScheduler(1);
}

void YKRun() {
  // At least one user-defined task needs to be created before this is called.	
  kernel_running_flag = 1;	
  YKScheduler(0);
}

void YKScheduler(char blocking) {
//printString(" the last task has priority ");
//printInt(lastRunningTask->priority);
//printNewLine();
//printString(" the current head has priority ");
//printInt(readyLstHead->priority);
//printNewLine();
  // If YKRun has not been called yet, return.
 	if (!kernel_running_flag){ 
		//printString("exited 1\n");
		return;
		}

  // If the currently executing task is the highest priority ready task, return.
	
	
	if (readyLstHead->priority == lastRunningTask->priority){
		//printString("exited 2\n");
 	  return;
	}
	else{
		//printString("updating Last Running Task\n");
		YKCtxSwCount++;
		*lastRunningTask = *readyLstHead;
		YKDispatcher(blocking);
	}
  // If the highest priority ready task is not running:
  // 1) Set the lastRunningTask state to READY, if it was still RUNNING
  // 2) Set the HPRT state to RUNNING
  // 3) Call the dispatcher, passing it the address of the first task in the ready list
 // if (lastRunningTask->state == TASK_STATE_RUNNING)
 //   lastRunningTask->state = TASK_STATE_READY;
 // readyLstHead->state = TASK_STATE_RUNNING;
 // lastRunningTask = readyLstHead;
 // YKDispatcher(blocking);
}

void YKIdleTask(void) {
  // The loop in this function should be exactly 4 instructions, and should
  // increment YKIdleCount atomically.
	while(1) {
    asm("cli");
    YKIdleCount++;
    asm("sti");
  }
}


// NOT THIS LAB

void YKTickHandler() {
  // I think we want this function to operate inside a mutex: when a tick
  // occurs, I feel like we want to make sue that all tasks are updated before
  // the scheduler gets a chance to run.
}

void YKDelayTask(unsigned count) {
  // This will update the state of a task to DELAYED, set the delay counter,
  // and move the task to the suspended list. Then it calls the scheduler.
}

void YKEnterISR() {
  // This function is called toward the beginning of each ISR, before interrupts
  // are re-enabled.
  isrDepth++;
}

void YKExitISR() {
  // This function is called toward the end of each ISR while interrupts
  // are disabled.
  isrDepth--;
  if (isrDepth == 0)
    YKScheduler(1);
}
