#include "delay.h"
void DelayUpdate() {

	List *list = delay_queue->head;	

	// Allocated list from append to list outside memory that this process can access

	while (list) {
		((PCB *) list->data)->clock_count--;
		list = list->next;
	}
}

void DelayAdd(PCB *pcb) {
	List *list = delay_queue->head;
	while (list && ((PCB *)list->data)->clock_count <= pcb->clock_count) {
		list = list->next;
	}

	if (!list) {
		queuePush(delay_queue, pcb);
		TracePrintf(1, "Pushed onto delay queue\n");
		return;
	} else {

		List *newList = listAllocate();
		newList->data = pcb;
		if (list->prev == NULL) { // Append at beginning.
			newList->next = delay_queue->head;
			newList->prev = NULL;
			delay_queue->head->prev = newList;
			delay_queue->head = newList;
		} else { // Append in middle.
			newList->next = list;
			list->prev->next = newList;
			newList->prev = list->prev;
			list->prev = newList;
		}
	}	
}

PCB * DelayPop() {
	while (delay_queue->head  && ((PCB *) delay_queue->head->data)->clock_count == 0) {
		TracePrintf(1, "Popping off from queue pid: %d with delay count: %d\n", ((PCB *) delay_queue->head->data)->id, ((PCB *) delay_queue->head->data)->clock_count);
		queuePush(ready_queue, queuePop(delay_queue));
	}
}
