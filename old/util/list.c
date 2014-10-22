#include <stdlib.h>
#include "list.h"

List *
listAllocate() {
    // TODO need to use kernel malloc here
    return malloc(sizeof(List));
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
