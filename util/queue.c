
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
    TracePrintf(1, "queue head: %p\n", queue->head);
	if (queue->head) {
        
    TracePrintf(1, "queue head data: %p\n", queue->head->data);
    TracePrintf(1, "queue head next next: %p\n", queue->head->next->next);
	List *node = queue->head;
        void *data = node->data;

        queue->head = node->next;
        if (queue->head)
            queue->head->prev = NULL;
        else
            queue->tail = NULL;
        free(node);
        queue->length--;
	TracePrintf(1, "queuePop data returned: %p\n", data);
        return data;
    }
    
    return NULL;
}

int
queueIsEmpty(Queue *queue) {
    return (queue->head == NULL);
}
