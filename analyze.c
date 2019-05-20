#include "globals.h"
#include "symtab.h"
#include "analyze.h"

/* counter for variable memory locations. */
static int location = 0;

/* Saved function name. */
static char* function_name = NULL;

/* proto */
void mainCheck (TreeNode*);

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

/* Function calcLoc calculates
 * memory location of symbol. 
 */
static int calcLoc (TreeNode* t) {
	int ret = 0;
	if (scope_top() == NULL)
		return 0;
	switch(t->nodekind) {
		case DeclK:
			switch(t->kind.decl) {
				case VarK:
					if (t->child[0] == NULL)
						break;
					if (scope_top()->level == 0) 
						ret = (t->child[0]->type == Array) 
							? (scope_top()->varLoc += 4 * (t->child[0]->len))
							: (scope_top()->varLoc += 4);
					else
						ret = (t->child[0]->type == Array)
							? (scope_top()->varLoc -= 4 * (t->child[0]->len))
							: (scope_top()->varLoc -= 4);
					break;
				case FunK:
					ret = scope_top()->funcLoc;
					scope_top()->funcLoc += 1;
					break;
				case ParamK:
					ret = scope_top()->paramLoc;
					scope_top()->paramLoc += 4;
					break;
			}
			break;
		default:
			;
	}
	return ret;
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
					location = calcLoc(t);
					/* First declared. */
					if (st_lookup_local(t->attr.name) == NULL)
						st_insert(t->attr.name, t->lineno, location, 
								'V', t->child[0]->type, t->child[0]->len, NULL);
					/* Duplicate declaration. */
					else 
						symbolError(t->lineno, "Duplicate var declaration.");
					break;
				case FunK:
					location = calcLoc(t);
					/* If first declared */
					if (st_lookup(t->attr.name) == NULL) {
						st_insert(t->attr.name, t->lineno, location, 
								'F', t->child[0]->type, t->child[0]->len, t->child[1]);
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
						st_insert (t->attr.name, t->lineno, 0, 
								'P', t->child[0]->type, t->child[0]->len, NULL);
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
	else if (t->nodekind == DeclK && t->kind.decl == ParamK) {
		/* directly adjust memory location. */
		BucketList l = st_lookup(t->attr.name);
		if (l) l->memloc = calcLoc(t);
	}
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
	scope_pop();
	return;
}

/* Procedure preCheckNode performs pre type checking
 * at a single tree node. 
 */
static void preCheckNode (TreeNode* t) {

	static int scope_cont = FALSE;
	static int i = 1;
	switch(t->nodekind) {
		case StmtK:
			switch(t->kind.stmt){
				case CompoundK:
					if (scope_cont == FALSE)
						scope_push(scope[i++]);
					else
						scope_cont = FALSE;
					break;
				default: ;
			}
			break;
		case DeclK:
			switch(t->kind.decl) {
				case FunK:
					/* Save function name. */
					function_name = t->attr.name;
					if (st_lookup(t->attr.name) == NULL) {
						scope_cont = TRUE;
						scope_push(scope[i++]);
					}
					break;
				default: ;
			}
			break;
		default: ;
	}
	return;
}

/* Function paramCheck checks 
 * whether arguments are valid or not(-1).
 * Returns -1 if number of params and args are different, 
 * and returns 0 if type of params and args are different,
 * and returns 1 if no error.
 */
int paramCheck(TreeNode* params, TreeNode* args) {
	int ret = 1;
	TreeNode *p, *a;
	int param_cnt = 0, arg_cnt = 0;
	BucketList p_entry = NULL;

	for (p = params; p; p = p->sibling)
		param_cnt++;
	for (a = args; a; a = a->sibling)
		arg_cnt++;

	if (param_cnt != arg_cnt)
		ret = -1;

	for (p = params, a = args; p && a;
			p = p->sibling, a = a->sibling) {
		p_entry = st_lookup(p->attr.name);
		if (p_entry && (p_entry->type != a->type)) {
			ret = 0;
			break;
		}
	}
	return ret;
}

/* Procedure checkNode performs type checking
 * at a single tree node.
 */
static void checkNode (TreeNode* t) {

	BucketList entry = NULL;
	TreeNode* left = NULL, *right = NULL;
	switch(t->nodekind) {
		case StmtK:
			switch(t->kind.stmt) {
				case CompoundK:
					scope_pop();
					break;
				case ReturnK:
					entry = st_lookup(function_name);
					/* Check return type. */
					if (entry && t->child[0] && (entry->type != t->child[0]->type))
						printError(t->lineno, "Wrong return type.");
					/* Check whether void type have return statement. */
					if (entry && entry->type == Void)
						printError(t->lineno, "Void function can't have return statement.");
					break;
				case WhileK:
					if (t->child[0] && t->child[0]->type != Integer)
						printError(t->lineno, "Condition statement should be int type.");
					break;
				default: ;
			}
			break;
		case ExpK:
			switch(t->kind.exp) {
				case OpK:
					left = t->child[0];
					right = t->child[1];
					/* Operand type check. */
					if (t->attr.op == ASSIGN) {
						if (left->type == Array)
							printError(t->lineno, "Can't assign to array itself.");
					}
					if (left && right && (left->type != right->type))
						printError(t->lineno, "Type of operands are different.");
					/* Set type. */
					t->type = left->type;
					break;
				case ConstK:
					t->type = Integer;
					break;
				case IdK:
					/* Set type. */
					entry = st_lookup (t->attr.name);
					if (!entry) break;
					t->type = entry->type;
					/* If array index is not integer */
					if (t->child[0] && t->child[0]->type != Integer) 
						printError(t->lineno, "Array index should be integer.");
					/* If using non-array id as array. */
					if (t->child[0] && t->type != Array)
						printError(t->lineno, "Not array.");
					/* Change type to int if have array subscription. */
					if (t->type == Array && t->child[0] != NULL)
						t->type = Integer;
					break;
				case CallK:
					/* Set type. */
					entry = st_lookup (t->attr.name);
					if (!entry) break;
					t->type = entry->type;
					/* If call somthing which is not function. */
					if (entry->VPF != 'F') 
						printError(t->lineno, "Not function.");
					/* Check parameter number & types. */
					else {
						switch(paramCheck(entry->params, t->child[0])){
							case -1: printError(t->lineno, "The number of parameters is invalid."); break;
							case  0: printError(t->lineno, "Invalid type of argument."); break;
							case  1: /* No error */; break;
						}
					}
					break;
				default: ;
			}
			break;
		case DeclK:
			switch(t->kind.decl) {
				case VarK:
					if (t->child[0] && t->child[0]->type == Void)
						printError(t->lineno, "Can't declare void type variable.");
					break;
				case ParamK:
					if (t->child[0] && t->child[0]->type == Void)
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

	mainCheck(syntaxTree);
	scope_push(scope[0]);
	traverse(syntaxTree, preCheckNode, checkNode);
	scope_pop();
	return;
}

/* determine whether main function has valid type 
 */
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
		if (t->child[0] && t->child[0]->type != Void) {
			printError(t->lineno, "Return type of main function should be void type.");
		}
		if (t->child[1] != NULL) {
			printError(t->lineno, "Parameter of main function should be void type.");
		}
	}
	else {
		printError (t->lineno, "There does not exist main function.");
	}

	return;
}
