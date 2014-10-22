/* list.h */
/*
 * Doubly linked list structure, drawing inspiration from
 * Glib's Glist.
 *
 * created 10/8/14  szheng
 */

#ifndef _list_h
#define _list_h

typedef struct _List List;

struct _List {
  void *data;
  List *next;
  List *prev;
};

List * listAllocate();
List * listAppend(List *list, void *data);
List * listAppendInPlace(List *list, void *data);
List * listPrepend(List *list, void *data);
List * listLast(List *list);
List * listDelayUpdate(List *list);

#endif /*!_list_h*/
