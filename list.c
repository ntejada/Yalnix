#include "list.h"
#include "std.h"

List *
listAllocate() {
  // TODO need to use kernel malloc here
  return (List *) malloc(sizeof(List));
}

List *
listAppend(List *list,
           void *data) {
  List *newList, *last;
  newList = listAllocate();
  newList->data = data;
  newList->next = NULL;

  if (list) {
    last = listLast(list);
    last->next = newList;
    newList->prev = last;
    return list;
  } else {
    newList->prev = NULL;
    return newList;
  }
}

List *
listAppendInPlace(List *list,
		  void *data) {
  List *newList, *start;
  start = list;

  newList = listAllocate();
  newList->data = data;
  newList->next = NULL;

  if (list) {

    while (list->next != NULL && ((DelayData *)newList->next->data)->clock_count > ((DelayData *)list->next->data)->clock_count)
      list = list->next;

    if (list->next == NULL) { // Append at end of list.
      newList->prev = list;
      list->next = newList;
      return start;
    } else if (list->prev == NULL) { // Append at beginning.
      newList->next = list;
      list->prev = newList;
      newList->prev = NULL;
      return newList;
    } else { // Append in middle.
      newList->next = list;
      list->prev->next = newList;
      newList->prev = list->prev;
      list->prev = newList;
      return start;
    }
  } else {
    newList->prev = NULL;
    return newList;
  }
}


List *
listPrepend(List *list,
            void *data) {
  List *newList;
  newList = listAllocate();
  newList->data = data;
  newList->next = NULL;

  if (list) {
    newList->prev = list->prev;
    if (list->prev)
      list->prev->next = newList;
    list->prev = newList;
  } else {
    newList->prev = NULL;
  }

  return newList;
}

List *
listLast(List *list) {
  if (list) {
    while (list->next)
      list = list->next;
  }
  return list;
}

List *
listDelayUpdate(List *list) {
  List *start = list;
  while (list) {
    ((DelayData *) list->data)->clock_count--;
    list = list->next;
  }
  list = start;
  while (list != NULL && ((DelayData *) list->data)->clock_count == 0) {
    if (list->next) {
      list = list->next;
      list->prev->next = NULL;
      // Place list->prev->data->pid on ready queue.
      list->prev = NULL;
      return list;
    } else {
      // Place list->data->pid on ready queue.
      return NULL;
    }
  }
  return list;
}
