/* 
	File: main.c 
	Main program for TINY compiler
*/

#include "globals.h"

#define NO_PARSE FALSE
#define NO_ANALYZE FALSE
#define NO_CODE TRUE

#include "util.h"

#if NO_PARSE
#include "scan.h"
#else
#include "parse.h"

#if !NO_ANALYZE
#include "analyze.h"

#if !NO_CODE
#include "cgen.h"

#endif
#endif
#endif

int lineno = 0;
FILE* source;
FILE* listing;
FILE* code;

int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = TRUE;
int TraceCode = FALSE;

int Error = FALSE;

int main (int argc, char* argv[]) {
	TreeNode* syntaxTree;
	char pgm[120]; /* file name. */
	if (argc != 2) {
		fprintf(stderr, "usage: %s <filename>\n", argv[0]);
		exit(1);
	}

	strcpy(pgm, argv[1]);
	if (strchr (pgm,'.') == NULL)
		strcat(pgm,".tny");
	source = fopen(pgm, "r");
	if (source == NULL) {
		fprintf(stderr, "File %s not found\n", pgm);
		exit(1);
	}
	listing = stdout;
#if NO_PARSE
	fprintf(listing, "   line number\t\t token\t\tlexeme\n");
	fprintf(listing, "-------------------------------------------------\n");
	while (getToken() != ENDFILE);
#else 
	syntaxTree = parse();
	if (TraceParse) {
		fprintf(listing, "\nSyntax tree: \n");
		printTree(syntaxTree);
	}

#if !NO_ANALYZE
	if (! Error) {	
		if (TraceAnalyze) fprintf(listing, "\nBuilding Synbol Table ...\n");
		buildSymtab(syntaxTree);
		if (TraceAnalyze) fprintf(listing, "\nChecking Types ...\n");
		typeCheck(syntaxTree);
		if (TraceAnalyze) fprintf(listing, "\nType Checking Finished\n");
	}

#if !NO_CODE
	if (! Error) {
		char* codefile;
		int fnlen = strcspn(pgm, ".");
		codefile = (char*) calloc(fnlen+4, sizeof(char));
		strncpy(codefile, pgm, fnlen);
		strcat(codefile, ".tm");
		code = fopen(codefile, "w");
		if(code == NULL) {
			printf("Unable to open %s\n", codefile);
			exit(1);
		}
		codeGen(syntaxTree, codefile);
		fclose(code);
	}
#endif
#endif
#endif
	fclose(source);
	return 0;
}
