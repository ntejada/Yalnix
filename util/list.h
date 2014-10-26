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

extern List * listAllocate();
extern List * listAppend(List *list, void *data);
extern List * listAppendInPlace(List *list, void *data);
extern List * listPrepend(List *list, void *data);
extern List * listLast(List *list);
extern List * listRemove(List *list);

#endif /*!_list_h*/
