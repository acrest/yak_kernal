/* yakc.c : c code for kernel implementation */

#include "yakk.h"
#include "yaku.h"

// ******** DEFINITIONS ******** //
#define NULL 0
#define IDLE_PRIORITY 255
#define TASK_STATE_RUNNING   1
#define TASK_STATE_READY     2
#define TASK_STATE_DELAYED   3
#define TASK_STATE_SUSPENDED 4

typedef struct taskblock
{				/* the TCB struct definition */
    void *stackptr;	// pointer to current top of stack
    int state;			// current state
    int priority;		// current priority
    int delay;			// #ticks yet to wait
    TCB* next;  		// forward ptr for linked list
} task;

// ------------------------------------------------------ //

// ******** GLOBAL VARIABLES ******** //
// These will be used by application code as well
unsigned YKCtxSwCount; // Global variable tracking context switches
unsigned YKIdleCount;  // Global variable used by idle task
unsigned YKTickNum;    // Global variable incremented by tick handler 

// FLAGS
// Should be set by a blocking function and unset by the dispatcher only
int called_from_blocking_function_flag;

// Stack frame for the idle task
int YKIdleTaskStack[STACK_SIZE];

// TCB: array of task structs. This array is declared in order to allocate
// space for each task struct. In usage, each struct will be treated like
// node in a linked list.
task TCBArray[MAX_NUM_TASKS + 1];
task* TCBHead;
unsigned int nextTaskIndex = 0;


// ******* KERNEL FUNCTIONS ******** //
void YKInitialize() {
  // QUESTION: should interrupts be off or on during initialization?
	YKCtxSwCount = 0; // Context switch count
	YKIdleCount  = 0; // Iterations of the idle task
	YKTickNum    = 0; // Number of tick interrupts
  TCBHead      = NULL;
  called_from_blocking_function_flag = 0;
	YKNewTask(&YKIdleTask, &YKIdleTaskStack[STACK_SIZE], IDLE_PRIORITY);
}

void YKNewTask(void(*taskCode)(void), void *taskStack, unsigned char priority) {
  // NOTE: this function can never be called from an interrupt handler. That
  // implies to me that we don't have to worry about the TCBHead pointer
  // getting mixed up. Am I right?

  // NOTE: when a task is first created, we need to somehow indicate at what
  // instruction it is going to start executing. I see two ways to do that.
  // First, we could store values for CS and IP in the TCB. Second, we could
  // push them on at the top of the stack frame. I'm thinking we should push
  // them on the stack frame, since that's how it is going to be in all other
  // contexts

  unsigned int index;
  // Read and update the nextTaskIndex atomically. Probably no other code
  // is going to use nextTaskIndex, but just in case...
	YKEnterMutex();
  index = nextTaskIndex;
  nextTaskIndex++;
  YKExitMutex();

  task* t = &TCBArray[index];
  // Here we set up the initial stack frame. We're simulating pushing the
  // the flags, CS, and IP onto the stack, so that later they can be restored
  // using the iret instruction.
  int* sp = (int*)taskStack;
  sp -= 3;
  *(sp+2) = 0x200;
  *(sp+1) = 0x0;
  *sp     = taskCode;
  // Set up the task struct for this task in the TCB
  t->stackptr = (void*)sp;
  t->state    = TASK_STATE_READY;
  t->priority = priority;
  t->delay    = 0;
  t_next      = NULL;
  // Handle the insertion and maintanance of the linked list. Tasks are
  // inserted into the linked list based on priority level; the list should
  // always be sorted from highest to lowest priority. Note that smaller
  // numbered priorities are "higher" priority.
  if (TCBHead == NULL)
    TCBHead = t;
  else if (TCBHead->priority > t->priority) {
    t->next = TCBHead;
    TCBHead = t;
  }
  else {
    task* t_iter = TCBHead;
    while (t_iter->next)
      if (t_iter->next->priority > t->priority)
        break;
      else
        t_iter = t_iter->next;
    t->next = t_iter->next;
    t-iter->next = t;
  }

  // Call the scheduler
  YKScheduler();
}

void YKRun() {
}

void YKDelayTask() {
}

void YKEnterMutex() {
  // This should be an assembly function
  asm("cli"); // This will inline the assembly to disable interrupts
}

void YKExitMutex() {
  // This should be an assembly function
  asm("sti"); // This will inline the assembly to enable interrupts
}

void YKEnterISR() {
}

void YKExitISR() {
}

void YKScheduler() {
}

void YKDispatcher() {
}

void YKTickHandler() {
}

void YKIdleTask(void) {
  // There's something about this function needing to take a certain
  // number of instructions per loop iteration: need to look into that
	while(1);
}
