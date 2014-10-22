#include <stdlib.h>
#include "list.h"
#include "queue.h"

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

        return data;
    }
    
    return NULL;
}

int
queueIsEmpty(Queue *queue) {
    return (queue->head == NULL);
}
