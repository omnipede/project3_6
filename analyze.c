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

/* nullProc is a do-nothing procedure to
 * generate preorder-only or postorder-only
 * traversals from traers
 */
static void nullProc(TreeNode* t) {

	if (t == NULL) return;
	return;
}

/* Function symbolError prints 
 * symbolic error of input source code.
 */
static void symbolError (int lineno, char* msg) {

	fprintf(listing, "Symbol error at line %d: %s\n", lineno, msg);
	Error = TRUE;
}


/* Procedure insertNode inserts ID stored in t
 * into the symbol table. 
 */
static void insertNode (TreeNode* t) {

	static int scope_cont = FALSE;
	switch(t->nodekind) {
		case StmtK: 
			switch(t->kind.stmt) {
				case CompoundK:
					if (scope_cont) 
						scope_cont = FALSE;
					else
						scope_push(scope_new());
					break;
				default:
					break;
			}
			break;
		case ExpK:
			break;
		case DeclK:
			switch(t->kind.decl) {
				case VarK:
					/* First declared. */
					if (st_lookup_local(t->attr.name) == -1)
						st_insert(t->attr.name, t->lineno, location, 
								'V', t->child[0]->type, t->child[0]->len);
					/* Duplicate declaration. */
					else 
						symbolError(t->lineno, "Duplicate var declaration.");
					break;
				case FunK:
					/* If first declared */
					if (st_lookup(t->attr.name) == -1) {
						st_insert(t->attr.name, t->lineno, location, 
								'F', t->child[0]->type, t->child[0]->len);
						scope_cont = TRUE;
						scope_push(scope_new());
					}
					/* Duplicate declaration. */
					else {
						symbolError(t->lineno, "Duplicate function declaration.");	
					}
					break;
				case ParamK:
					/* If first declared. */
					if (st_lookup_local(t->attr.name) == -1) 
						st_insert (t->attr.name, t->lineno, location, 
								'P', t->child[0]->type, t->child[0]->len);
					/* Duplicate declared. */
					else 
						symbolError(t->lineno, "Duplcate paramater declaration.");
					break;
			}
			break;
		case TypeK:
			break;	
	}
}

static void postInsertNode (TreeNode* t) {
	switch(t->nodekind) {
		case StmtK:
			switch(t->kind.stmt) {
				case CompoundK:
					scope_pop();
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
}

/* Function buildSymtab constructs the
 * symbol table by preorder traversal of the syntax tree
 */
void buildSymtab (TreeNode *syntaxTree) {

	/* Push global scope */
	scope_push(scope_new());

	traverse(syntaxTree, insertNode, postInsertNode);

	if (TraceAnalyze) {

		fprintf(listing, "\nSymbol table:\n\n");
		printSymTab(listing);
	}
	return;
}
