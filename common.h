#include <stdio.h>
#include <stdlib.h>

#define SUCCESS 0
#define ERROR (-1) 

/*******************
 * QUEUE/STACK
 ***********************/
typedef struct {
	void * head;
	void * tail;
} Queue;

typedef struct {
  void * head;
} Stack;


/**********************
 * SYNCHRONIZATION PRIMITIVES
 **********************/

typedef struct {
  int *PCB;
  Cvar_Node* next;
  int *lock;
} Cvar_Node;

typedef struct {
  unsigned int state;
  Queue *lock_queue;
  PCB *pcb;
} Lock;

typedef struct {
  char *buffer;
  int length_buffer;
  int identifier;
} Pipe;

	
/*********************
 *  PCB
 **********************/
typedef struct {
  unsigned int state;
  unsigned int exit_state;
  unsigned int pid;
  unsigned int ppid;
  
  UserContext *user_context;
  KernelContext *kernel_context;

  Stack *child_queue;
  // Page Table
} PCB;

typedef struct {
  unsigned int exit_status;
  unsigned int pid;
} Z_CB; // Zombie Control Block
