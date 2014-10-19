#include "common.h"


// Create generic cvar node.
Node *createNode(PCB *pcb,  int lock_id) {
  Node *node = malloc(sizeof(Node));
  
  if (cv == NULL) {
    TracePrintf(1, "Malloc Error, createNode\n");
    return NULL;
  }

  memset(node, 0, sizeof(Node));
  node->pcb = pcb;
  // If initialized with -1, then not a lock node.
  if (-1 == lock_id)
    node->lock_id = lock_id;

  // Otherwise, check if lock even exists. 

  return node;
}


/*******************************
 * Common queue functions.
 ****************************/

Queue *initQueue() {
  Queue *q_t = malloc(sizeof(Queue));

  if (q_t == NULL) {
    TracePrintf(1, "Malloc Error, initQueue\n");
    return NULL;
  }

  memset(q_t, 0, sizeof(Queue));
  q_t->head = NULL;
  q_t->tail = NULL;

  return q_t;
}


  
int isQueueEmpty(Queue *q_t) {
  if (q_t == NULL) {
    TracePrintf(1, "NULL Queue input, isEmpty\n");
    return ERROR;
  }

  if (q_t->head == NULL && q_t->tail == NULL) 
    return 1;
  return 0;
}



int enqueue(Queue *q_t, Node *node) {
  if (q_t == NULL || node == NULL) { 
    TracePrintf(1, "Arguments NULL, enqueue\n");
    return ERROR;
  }
  int rc = isQueueEmpty(q_t);
  
  if (ERROR == rc) {
    TracePrintf(1, "isEmpty Error, enqueue\n");
    return ERROR;
  }
  else if (rc) {
    q_t->head = node;
    q_t->tail = node;
  } 
  else {
    q_t->tail->next = node;
    q_t->tail = node;
  }

  return SUCCESS;
}

void *dequeue(Queue *q_t) {
  
  int rc = isQueueEmpty(q_t);
  if (ERROR == rc) {
    TracePrintf(1, "isEmpty Error, dequeue\n");
    return NULL;
  }
  else if (rc) {
    return NULL;
  }

  
  Node *node = q_t->head;
  if (q_t->head->next == NULL) { // One element left in queue.
    q_t->head = NULL;
    q_t->tail = NULL;
  } 
  else {
    q_t->head = q_t->head->next;
  }

  node->next = NULL;
  return node;
}

// Used for Broadcast
// Will place each Cvar_Node onto corresponding lock queue after being dequeued from cvar queue
int dequeueALL(Queue *q_t) {
  
  if (q_t == NULL) {
    TracePrintf(1, "NULL Queue input, dequeueAll\n");
    return ERROR;
  }
  
  int rc = isQueueEmpty(q_t);
  if (ERROR == rc) {
    TracePrintf(1, "isEmpty Error, enqueue\n");
    return ERROR;
  }
  else if (rc) {
    return SUCCESS;
  }


  Node *node = q_t->head;
  Node *temp;
  while (node != NULL) {
    temp = node;
    node = node->next;
    temp->next = NULL;

    Queue *lock_q_t = getLock(node->lock_id); // Need to create function to find lock with corresponding lock id
    if (lock_q_t == NULL) {
      TracePrintf(1, "No Lock Queue Initialized, dequeueAll\n");
      return ERROR;
    }

    if (ERROR == enqueue(lock_q_t, temp)) {
      TracePrintf(1, "Error enqueueing onto LockQueue, dequeueAll\n");
      return ERROR;
    }
  }
  return SUCCESS;
}
    
    
/**********************************
 * Common stack functions
 ********************************/
  
Stack *initStack() {
  Stack *s_t = malloc(sizeof(Stack));

  if (s_t == NULL) {
    TracePrintf(1, "Malloc Error, initStack\n");
    return NULL;
  }

  memset(s_t, 0, sizeof(Queue));
  s_t->head = NULL;

  return s_t;
}


int isStackEmpty(Stack *s_t) {
  if (s_t == NULL) {
    TracePrintf(1, "NULL Queue input, isEmpty\n");
    return ERROR;
  }

  if (s_t->head == NULL) 
    return 1;
  return 0;
}




int push(Stack *s_t, Node *node) {
  if (s_t == NULL || node == NULL) {
    TracePrintf(1, "Arguments NULL, push\n");
    return ERROR;
  }

  int rc = isStackEmpty(s_t);
  if (ERROR == rc) {
    TracePrintf(1, "isEmpty Error, push\n");
    return ERROR;
  }
  else if (rc) {
    s_t->head = node;
  }
 else {
   node->next = s_t->head; 
   s_t->head = node;
  }

  return SUCCESS;
}


void *pop(Stack *s_t) {

  int rc = isStackEmpty(s_t);
  if (ERROR == rc) {
    TracePrintf(1, "isEmpty Error, pop\n");
    return NULL;
  }
  else if (rc) {
    return NULL;
  }

  Node *node = s_t->head;

  s_t->head = s_t->head->next;

  node->next = NULL;
  return node;
}



