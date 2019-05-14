#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE is the size of the hash table. */
#define SIZE 211

/* SHIFT is the power of two used as multiplier */
#define SHIFT 4

/* Wrapping structure of BucketList. */
typedef struct ScopeListRec {
	/* hash table */
	BucketList hashTable[SIZE];
	/* scope level */
	int level;
	/* Parent ptr. */
	struct ScopeListRec* parent;
}* ScopeList;

/* Maximum scope level. */
#define MAX_SCOPE 200

/* index of scope list. */
int scope_index = 0;
/* List of scopes. */
ScopeList scope[MAX_SCOPE];

/* Top of stack. */
static int top = 0;
/* Stack of scopes. */
ScopeList scope_stack[MAX_SCOPE];

/* Function scope_top returns
 * current scope record. 
 */
struct ScopeListRec* scope_top (void) {

	if (top > 0)
		return scope_stack[top - 1];
	else
		return NULL;
}

/* Procedure scope_push pushes
 * current scope record to stack
 */
void scope_push (struct ScopeListRec* sc) {
	
	if (top < MAX_SCOPE)
		scope_stack[top++] = sc;	
	return;
}

void scope_pop (void) {

	top -=  1;
	return;
}

struct ScopeListRec* scope_new (void) {

	struct ScopeListRec* t
		= (struct ScopeListRec*) malloc (sizeof(struct ScopeListRec));

	t->level = top;
	t->parent = scope_top();

	scope[scope_index] = t;
	scope_index += 1;
	return t;
}


/* the hash function */
static int hash (char* key) {

	int temp = 0;
	int i = 0;
	while(key[i] != '\0') {
		temp = ((temp << SHIFT) + key[i]) % SIZE;
		++i;
	}
	return temp;
}
