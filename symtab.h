#ifndef _SYMTAB_H_
#define _SYMTAB_H_

/* Procedure xt_insert inserts line numbers and
 * memory locations into the symbol table
 */
void st_insert (char* name, int lineno, int loc);

/* Function st_lookup returns the memory location
 * of a variable or -1 if not found
 */

int st_lookup (char* name);

/* Procedure printSymTab prints a formatted listing 
 * of the symbol table contents
 * to the listing file
 */
void printSymTab (FILE* listing);

/* the list of line numbers of the source code
 * in which a variable is referenced
 */
typedef struct LineListRec {

	int lineno;
	struct LineListRec* next;
}* LineList;

/* The record in the bucket lists for
 * each variable, including name, assigned memory loc, 
 * and the list of line numbers 
 * in which it appears in the source code. 
 */
typedef struct BucketListRec {

	char* name;
	LineList lines;
	int memloc;
	struct BucketListRec* next;
}* BucketList;


#endif
