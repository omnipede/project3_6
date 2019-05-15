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

	return;
}

/* Function typeError prints
 * type error of input source code. 
 */
static void typeError (int lineno, char* msg) {

	Error = TRUE;
	fprintf(listing, "Type error at line %d: %s\n", lineno, msg);
}

/* Function symbolError prints 
 * symbolic error of input source code.
 */
static void symbolError (int lineno, char* msg) {

	Error = TRUE;
	fprintf(listing, "Symbolic error at line %d: %s\n", lineno, msg);
}


/* Procedure insertNode inserts ID stored in t
 * into the symbol table. 
 */
static void insertNode (TreeNode* t) {

	if (!t)
		return;
	static int scope_cont = FALSE;
	switch(t->nodekind) {
		case StmtK: 
			switch(t->kind.stmt) {
				case CompoundK:
					if (scope_cont == FALSE) 
						scope_push(scope_new());
					else 
						scope_cont = FALSE;
					break;
				default:
					break;
			}
			break;
		case ExpK:
			switch(t->kind.exp) {
				case IdK:
					/* If not declared yet, */
					if (st_lookup (t->attr.name) == NULL)
						symbolError(t->lineno, "Undeclared symbol.");
					/* If declared, append line number. */
					else 
						st_insert_global (t->attr.name, t->lineno);
					break;
				case CallK:
					/* If not declared yet, */
					if (st_lookup (t->attr.name) == NULL)
						symbolError(t->lineno, "Undeclared symbol.");
					/* If declared, append line number. */
					else
						st_insert_global (t->attr.name, t->lineno);
					break;
				default:
					break;
			}
			break;
		case DeclK:
			switch(t->kind.decl) {
				case VarK:
					/* First declared. */
					if (st_lookup_local(t->attr.name) == NULL)
						st_insert(t->attr.name, t->lineno, location, 
								'V', t->child[0]->type, t->child[0]->len);
					/* Duplicate declaration. */
					else 
						symbolError(t->lineno, "Duplicate var declaration.");
					break;
				case FunK:
					/* If first declared */
					if (st_lookup(t->attr.name) == NULL) {
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
					if (st_lookup_local(t->attr.name) == NULL) 
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

/* Function postInsertNode pops current scope
 * when face end of compound statement.
 */
static void postInsertNode (TreeNode* t) {
	if (t->nodekind == StmtK 
			&& t->kind.stmt == CompoundK)
		scope_pop();
	else
		return;
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
