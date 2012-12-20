#ifndef _GLOBALS_H
#define _GLOBALS_H

#define ENDFILE 256
#define ERROR   257
#define NUM     258
#define PLUS    259
#define TIMES   260
#define LPAREN  261
#define RPAREN  262
#define SUB     263
#define DIV     264
#define ID      265
#define ASSIGN  266
#define OUT     267
#define SEMI    268
#define INPUT   269

#define VIRTUALNODE   270
#define BLOCKNODE   271
#define DECNODE   272
#define IFSELECTION   273
#define ITERATION   274
#define BEXPR   275
#define LETNODE   276
#define FUNCNODE   277

#define OPLESSEQ 280
#define OPGREATEREQ 281
#define OPEQ  282
#define OPLESS 283
#define OPGREATER 284
#define OPNOTEQ 285
#define OPAND  286
#define OPNOT  287
#define OPRETURN  288

#define LBRKT  290
#define RBRKT  291

#define KEYWORD_INT   300
#define KEYWORD_BOOL   301
#define KEYWORD_IF   302
#define KEYWORD_ELSE   303
#define KEYWORD_WHILE   304
#define KEYWORD_BREAK   305
#define KEYWORD_TRUE   306
#define KEYWORD_FALSE   307
#define KEYWORD_FUNDEF   308
#define KEYWORD_LET   309
#define KEYWORD_IN   310
#define KEYWORD_END   311
#define KEYWORD_COMMA   312
#define KEYWORD_FN   313

#define DATATYPE_INT 320
#define DATATYPE_BOOL 321
#define DATATYPE_FUNC 322
#define DATATYPE_UNKOWN 323

#define MAX_REG 3
#define REG_SP 6

#include <string.h>
#include <iostream>
#include <vector>


#define MAXIDLENGTH 256
#define HASH_TABLE_SIZE 100
#define SHIFT 4

#define MAXSCOPE 50

using namespace std;

typedef struct {
  int TokenClass;  /* one of the above */
  string TokenString;
} TokenType;

#define SELFTEXT "_self"


#define MAXCHILDREN 3

typedef struct treeNode {
    struct treeNode * child[MAXCHILDREN];
	struct treeNode * sibling;
    int op;
    int val;
    char id[MAXIDLENGTH];
	int type;
	int st_hash_index;
	char comment[500];
	int isCopy;
	int isCall;
} TreeNode;


typedef struct varType{
	int dataType;
    char varid[MAXIDLENGTH];
	int isCall;
}VarType;


typedef struct functionType{
	std::vector<VarType *> parameterstype;
 	VarType *returnvaltype;
} FunctionType;

#define ST_SIZE 26

#define CODESIZE 500

typedef enum {RO,RM} OpCodeType;

typedef struct {
  string opcode;
  OpCodeType ctype;
  int rand1;
  int rand2;
  int rand3;
} CodeType;

typedef struct _bucket{
	char name[MAXIDLENGTH];
	int addr;
	struct _bucket *next;
	int type;
	struct functionType *infer_type;
	} bucket;

typedef struct _STLinkedNode{
	bucket ST_HASH[HASH_TABLE_SIZE];
	struct _STLinkedNode *next;
	} STLinkedNode;

/* function getToken returns the 
 * next token in source file
 */
TokenType getToken(void);
//TreeNode * statements(void);
TreeNode *program(void);

void analyze(TreeNode*);
void codeGenProgram(TreeNode*);
void emit(string,OpCodeType,int,int,int);
void printCode(void);
int hash(char *);
bucket *lookup(char *, bucket **);
void insert(char *, int, bucket **);
int getAddrByid(char *);
int getTypeByid(char *);
std::string getblockname( int);
TreeNode * newNodeWithOp(int);

#endif
