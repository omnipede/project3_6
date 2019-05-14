#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* SIZE is the size of the hash table. */
#define SIZE 211

/* SHIFT is the power of two used as multiplier */
#define SHIFT 4

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
