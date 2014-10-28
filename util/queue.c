
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
    queue->tail = listAppend(queue->tail, data);
    if (queue->tail->next)
        queue->tail = queue->tail->next;
    else
        queue->head = queue->tail;
    queue->length++;
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

//void
//queueRemove(Queue *queue, 
//	        void *data) {
//   queue->head = listRemove(queue->head, data);
//}

void
queueRemove(Queue *queue, 
	        void *data) {
    List *list = queue->head;
    
    // Move to where the data is.
    while (list && list->data != data) {
        list = list->next;
    }

    // If the node actually exists, update the queue and free.
    if (list) {
        if (list == queue->head) {
            queue->head = list->next;
            if (queue->head) {
                queue->head->prev = NULL;
            }
        } else if (list == queue->tail) {
            queue->tail = list->prev;
            if (queue->tail) {
                queue->tail->next = NULL;
            }
        } else {
            // Somewhere in the middle
            list->prev->next = list->next;
            list->next->prev = list->prev;
        }
        queue->length--;
        free(list);
    }
}

int
queueIsEmpty(Queue *queue) {
    return (queue->head == NULL);
}
