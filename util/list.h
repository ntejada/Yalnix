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
extern List * listAppend(List *, void *);
extern List * listAppendInPlace(List *, void *);
extern List * listPrepend(List *, void *);
extern List * listLast(List *);
extern List * listRemove(List *, void *);

#endif /*!_list_h*/
