
#include "list.h"
#include "queue.h"
#include "std.h"

Queue *
queueNew() {
    Queue *queue = malloc(sizeof(Queue));
        queue->head = queue->tail = NULL;
    queue->length = 0;
    return queue;
}

void
queuePush(Queue *queue,
          void *data) {
    TracePrintf(2, "queuePush\n");
    queue->tail = listAppend(queue->tail, data);
    if (queue->tail->next)
        queue->tail = queue->tail->next;
    else
        queue->head = queue->tail;
    queue->length++;
    TracePrintf(2, "queuePush: length queue after push: %d\n", queue->length);
}

void *
queuePop(Queue *queue) {

    if (queue->head) {
        


	List *node = queue->head;
        void *data = node->data;

        queue->head = node->next;
        if (queue->head)
            queue->head->prev = NULL;
        else
            queue->tail = NULL;
        free(node);
        queue->length--;
       TracePrintf(1, "QueuePop: Data returned: %p\n", data);
        return data;
    }
    
    return NULL;
}

void
queueRemove(Queue *queue, 
	        void *data) {
    queue->head = listRemove(queue->head, data);
}


int
queueIsEmpty(Queue *queue) {
    return (queue->head == NULL);
}
