#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

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

	if (top > 0)
		top -=  1;
	return;
}

struct ScopeListRec* scope_new (void) {

	/* If space allows, */
	if (scope_index < MAX_SCOPE) {
		struct ScopeListRec* t
			= (struct ScopeListRec*) malloc (sizeof(struct ScopeListRec));

		t->level = top;
		t->parent = scope_top();

		scope[scope_index] = t;
		scope_index += 1;
		return t;
	}
	else
		return NULL;
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

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 */
void st_insert (char* name, int lineno, int loc) {

	int h = hash(name);
	BucketList l = scope_top()->hashTable[h];

	while ( l && (strcmp(name, l->name) != 0))
		l = l->next;
	if (l == NULL) { /* variable not yet in table */
		l = (BucketList) malloc (sizeof(struct BucketListRec));
		l->name = name;
		l->lines = (LineList) malloc (sizeof(struct LineListRec));
		l->lines->lineno = lineno;
		l->lines->next = NULL;
		l->memloc = loc;
		l->next = scope_top()->hashTable[h];
		scope_top()->hashTable[h] = l;
	}
	else { /* found in table, so just add line number */
		LineList t = l->lines;
		while(t->next) t = t->next;
		t->next = (LineList) malloc (sizeof(struct LineListRec));
		t->next->lineno = lineno;
		t->next->next = NULL;
	}
}

/* Funcion st_lookup returns the memory location
 * of a variable or -1 if not found.
 */
int st_lookup (char* name) {

	int h = hash(name);
	struct ScopeListRec* sc = scope_top();
	BucketList l = NULL;
	while (sc) {
		l = sc->hashTable[h];
		while (l && (strcmp(name, l->name) != 0) )
			l = l->next;

		if (l == NULL)
			sc = sc->parent;
		else
			return l->memloc;
	}
	return -1;
}

/* Function st_lookup_local returns the memory location
 * of a variable in local scope, if not found, returns -1
 */
int st_lookup_local (char* name) {

	int h = hash(name);
	BucketList l = scope_top()->hashTable[h];
	while( l && (strcmp(name, l->name) != 0) )
		l = l->next;

	if (l == NULL)
		return -1;
	else
		return l->memloc;
}

/* Procedure printSymTab prints a formatted listing
 * of the symbol table contents
 * to the listing file.
 */
void printSymTab(FILE* listing) {

	int i, j;
	for (i = 0; i < scope_index; i++) {
		for (j=0; j < SIZE; j++) {

			BucketList l = scope[i]->hashTable[j];
			while(l != NULL) {
				fprintf(listing, "%s\t", l->name);
				fprintf(listing, "%d\t", l->memloc);
				
				LineList t = l->lines;
				while(t) {
					fprintf(listing, "%d ", t->lineno);
					t = t->next;
				}
				fprintf(listing, "\n");
				l = l->next;
			}
		}
	}
}
