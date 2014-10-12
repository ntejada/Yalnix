#include "common.h"


// Create generic node.
Cvar_Node *createCvarNode(int *PCB,  int *lock_t) {
  Cvar_Node *cv = malloc(sizeof(Cvar_Node));
  
  if (cv == NULL) {
    TracePrintf(1, "Malloc Error, createCvarNode\n");
    return NULL;
  }

  memset(cv, 0, sizeof(Cvar_Node));
  cv->PCB = PCB;
  cv->lock = lock_t;

  return cv;
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


  
int isEmpty(Queue *q_t) {
  if (q_t == NULL) {
    TracePrintf(1, "NULL Queue input, isEmpty\n");
    return ERROR;
  }

  if (q_t->head == NULL && q_t->tail == NULL) 
    return 1;
  return 0;
}



int enqueue(Queue *q_t, void *node) {
  if (q_t == NULL || node == NULL) { 
    TracePrintf(1, "Arguments NULL, enqueue\n");
    return ERROR;
  }
  int rc = isEmpty(q_t);
  
  if (ERROR == rc) {
    TracePrintf(1, "isEmpty Error, enqueue\n");
    return ERROR;
  }
  else if (rc) {
    queue->head = node;
    queue->tail = node;
  } 
  else {
    q_t->tail->next = node;
    q_t->tail = tail->next;
  }

  return SUCCESS;
}

void *dequeue(Queue *q_t) {
  
  int rc = isEmpty(q_t);
  if (ERROR == rc) {
    TracePrintf(1, "isEmpty Error, enqueue\n");
    return ERROR;
  }
  else if (rc) {
    return NULL;
  }

  
  void *node = q_t->head;
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
  
  int rc = isEmpty(q_t);
  if (ERROR == rc) {
    TracePrintf(1, "isEmpty Error, enqueue\n");
    return ERROR;
  }
  else if (rc) {
    return SUCCESS;
  }

  (Cvar_Node *) node = q_t->head;
  Cvar_Node *temp;
  while (node != NULL) {
    temp = node;
    node = node->next;
    temp->next = NULL;

    Queue *lock_q_t = node->lock->lock_queue;
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
    
    

  
