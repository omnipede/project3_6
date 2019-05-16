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

/* Function printError prints
 * general semantic error of input source code. 
 */
static void printError (int lineno, char* msg) {

	Error = TRUE;
	fprintf(listing, "Semantic error at line %d: %s\n", lineno, msg);
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
					if (scope_cont == FALSE) 
						/* Create new scope */
						scope_push(scope_new());
					else /* Don't create new scope */
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

/* Procedure preCheckNode performs pre type checking
 * at a single tree node. 
 */
static void preCheckNode (TreeNode* t) {

	return;
}

/* Procedure checkNode performs type checking
 * at a single tree node.
 */
static void checkNode (TreeNode* t) {

	switch(t->nodekind) {
		case StmtK:
			break;
		case ExpK:
			switch(t->kind.exp) {
				case OpK:

					break;
			}
			break;
		case DeclK:
			switch(t->kind.decl) {
				case VarK:
					if (t->child[0]->type == Void)
						printError(t->lineno, "Can't declare void type variable.");
					break;
				case ParamK:
					if (t->child[0]->type == Void)
						printError(t->lineno, "Can't declare void type parameter.");
					break;
				default: ;
			}
			break;
		case TypeK:
			break;
		default:
			;
	}
	return;
}

/* Procedure typeCheck performs type checking 
 * by a postorder syntax tree traversal.
 */
void typeCheck (TreeNode* syntaxTree) {

	traverse(syntaxTree, nullProc, checkNode);
	return;
}

/* determine whether main function has valid type 
 */
/*
void mainCheck (TreeNode* t) {

	if (!t) {
		printError (t->lineno, "There does not exist main function.");
		return;
	}
	for (; t->sibling; t = t->sibling) 
		;
	if (t->nodekind == DeclK 
			&& t->kind.decl == FunK 
			&& strcmp(t->attr.name, "main") == 0) {
		if (t->child[0] && t->child[0]->nodekind == TypeK) {
			if (t->child[0]->type != Void) {
				printError(t->lineno, "Return type of main function should be void type.");
				return;
			}
			if (t->child[1] && t->child[1]->type != Void) {
				printError(t->lineno, "Parameter of main function should be void type.");
				return;
			}
		}
		else {
			printError (t->lineno, "There does not exist main function.\n");
			return;
		}
	}
}*/
