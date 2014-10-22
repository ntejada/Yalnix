/*
 * Conditional variables for Yalnix
 *
 * Author: szheng
 * created 10/10/14
 */

#ifndef _cvar_h
#define _cvar_h

#include "util/queue.h"

#define CVAR 2

typedef struct cvar_t Cvar;

struct cvar_t {
    int id;         /* Cvar identifier */
    Queue *waiting; /* List of processes waiting on this cvar */
};

extern void doCvarInit(UserContext *);
extern void doCvarSignal(UserContext *);
extern void doCvarBroadcast(UserContext *);
extern void doCvarWait(UserContext *);
extern Cvar * getCvar(int);
extern int destroyCvar(int);

#endif /*!_cvar_h*/
