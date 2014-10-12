#include <stdio.h>
#include <stdlib.h>

typedef struct {
	void * head;
	void * tail;
	int id;
} queue;

typedef struct Cvar_Node{
	int *PCB;
	Cvar_Node* next;
	int *lock;
} Cvar_Node;

typedef struct PCB{
	
