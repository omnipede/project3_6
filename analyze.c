#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations. */
static int location = 0;

/* Procedure traverse is a generic recursive 
 * syntax tree traversal routine;
 * it applies preProc in preorder and postProc 
 * in postorder to tree pointed to by t
 */
static void traverse (TreeNode* t, 
				void (* preProc) (TreeNode *), 
				void (* postProc) (TreeNode *) ) 
{
	if (t != NULL) {

		preProc(t);
		int i;
		for (i = 0; i < MAXCHILDREN; i++)
			traverse(t->child[i], preProc, postProc);
		postProc(t);
		traverse(t->sibling, preProc, postProc);
	}
}
