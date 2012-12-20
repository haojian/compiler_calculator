#include <stdlib.h>
#include <iostream>
#include "globals.h"

/*  Grammer Definition:
 *  factor ::= ID | NUM | - factor | (expr);
 *  term   :: = factor | factor mulop term;
 *  expr   :: = term | term addop expr | inputexp;
 *
 */

/* Syntax Analyzer for the calculator language */

static TokenType token; /* holds current token */
int total_addr_index = 0;
int ifExpressioninPAREN = 0;  /* if the expression is in parens, then SEMI is not required. */
TreeNode *bexp_general();
int inExpr = 0; // 0 for boolean, 1 for comparison, 2 for expression
int ifInIteration = 0;
TreeNode *getFuncCall(TreeNode * funcNode);
/* Function newExpNode creates a new expression 
 * node for syntax tree construction
 */

TreeNode * newNodeWithOp(int op) {
  TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
	t->type = DATATYPE_UNKOWN;
  if (t==NULL) {
    cerr << "Out of memory error at line %d\n";
    exit(1);
  }
  else {
    for (i=0;i < MAXCHILDREN;i++) t->child[i] = NULL;
    t->op = op;
  }
  return t;
}

TreeNode * newNode(TokenType tType) {
  TreeNode * t = (TreeNode *) malloc(sizeof(TreeNode));
  int i;
t->type = DATATYPE_UNKOWN;
  if (t==NULL) {
    cerr << "Out of memory error at line %d\n";
    exit(1);
  }
  else {
    for (i=0;i < MAXCHILDREN;i++) t->child[i] = NULL;
    t->op = tType.TokenClass;
  }
  return t;
}


static void advance(int expected) {
  if (token.TokenClass == expected) {
	token = getToken();
  }
  else {
    cerr << "unexpected token -> " << token.TokenString << endl; 
	exit(1);
  }
}

TreeNode * exp(void);
TreeNode *block(void);
TreeNode *comparison(void);
TreeNode *statement(void);
TreeNode *assign_stmt(void);
TreeNode *selection(void);

int relop(){
	int op = OPEQ;
	switch (token.TokenClass)  {
		case OPLESS: 
			op = OPLESS;
			advance(OPLESS); 
			break;
		case OPLESSEQ: 
			op = OPLESSEQ;
			advance(OPLESSEQ); 
			break;
		case OPGREATER: 
			op = OPGREATER;
			advance(OPGREATER); 
			break;
		case OPGREATEREQ: 
			op = OPGREATEREQ;
			advance(OPGREATEREQ);
			break;
		case OPEQ: 
			op = OPEQ;
			advance(OPEQ); 
			break;
		case OPNOTEQ:
			op = OPNOTEQ;
			advance(OPNOTEQ); 
			break;
		default:
			op = -1;
			cerr << "relop error" << endl;
			
	}
	return op;
}


TreeNode * factor(void) {
  TreeNode * t = NULL;
  TokenType tmp;
  switch (token.TokenClass) {
  case NUM :
    t = newNode(token);
    if ((t!=NULL) && (token.TokenClass==NUM))
      t->val = atoi(token.TokenString.c_str());
    advance(NUM);
    break;
  case ID :
    t = newNode(token);
    if ((t!=NULL) && (token.TokenClass==ID)){
      strcpy(t->id, token.TokenString.c_str());
	}
    advance(ID);
	//deal with call
	if (token.TokenClass == LPAREN){
		t = getFuncCall(t);
	}
    break;
  case LPAREN :
    advance(LPAREN);
    t = exp();
    advance(RPAREN);
    break;
  case SUB:
	t = newNode(token);
	advance(SUB);
	tmp.TokenClass = NUM;
	tmp.TokenString = "0";
	t->child[0] = newNode(tmp);
	t->child[1] = factor();
	break;
  case INPUT:
	t= newNode(token);
	if((t!= NULL) && token.TokenClass== INPUT){
		t->op = INPUT;
	}
	advance(INPUT);
	if(token.TokenClass != LPAREN){
		cerr << "unexpected token after input -> " << token.TokenString << endl;
	      exit(1);
	}
	advance(LPAREN);
    if(token.TokenClass != RPAREN) {
      cerr << "unexpected token after input -> " << token.TokenString << endl;
      exit(1);
    }
    advance(RPAREN);
	break;
  default:
    cerr << "unexpected token -> " << token.TokenString << endl;
    exit(1);
    break;
  }
  return t;
}

TreeNode *term_prime(TreeNode * left){
	if ((token.TokenClass==TIMES) 
	      || (token.TokenClass==DIV)) {
			TreeNode *p = newNode(token);
			advance(token.TokenClass);
			if(p != NULL){
				p->child[0] = left;
				p->child[1] = factor();
				//p->child[1] = term_prime(factor());
			}
			//return p;
			return term_prime(p);
	}
	else
		return left;
}

TreeNode *term(){
	TreeNode * t = factor();
	return term_prime(t);
}

TreeNode * exp_prime(TreeNode * left) {
  if ((token.TokenClass==PLUS) 
      || (token.TokenClass==SUB)) {
    TreeNode * p = newNode(token);
    
    advance(token.TokenClass);
    if (p!=NULL) {
      p->child[0] = left;
	  p->child[1] = term();
	  return exp_prime(p);
    }
  }
  else return left;
}


TreeNode * exp(void) {
	TreeNode * t = term();
    TreeNode *res = exp_prime(t);
    return res;
}


//comparison need refine.
TreeNode *comparison(){
	TreeNode *current_node =  newNodeWithOp(OPEQ);
	if(current_node != NULL){
		current_node->child[0] = exp();
	}
	int op = relop();
	if (current_node != NULL) {
		current_node->op = op; 
		current_node->child[1] = exp();
	}
	return current_node;
}


TreeNode * bexpr(bool);

TreeNode * bexpr_prime(TreeNode * left, bool inParen) {
  TreeNode * p = NULL;

  if (token.TokenClass==OPAND) {
    inExpr = 0;
    p = newNode(token);
    
    advance(token.TokenClass);
    if (p!=NULL) {
      p->child[0] = left;
      p->child[1] = bexpr(inParen);
      return bexpr_prime(p,false);
    }
  }
  else return left;
}

TreeNode *getParameters(){
	TreeNode * t = NULL;
	t = bexpr(true);
	TreeNode * tmp = t;
	while(token.TokenClass == KEYWORD_COMMA){
		advance(token.TokenClass);
		tmp->sibling = bexpr(true);
		tmp = tmp->sibling;
	}
	return t;
}

TreeNode *getFuncDef(){
	TreeNode * t = newNodeWithOp(FUNCNODE);
	//t->type = 2;
	advance(KEYWORD_FN); 
	advance(LPAREN);
	t->child[0] = getParameters();
	advance(RPAREN);
	advance(OPRETURN);
	t->child[1] = bexpr(true);
	
	return t;
}

TreeNode *getLet(){
	TreeNode * current_node = newNodeWithOp(LETNODE);
	advance(KEYWORD_LET);
	current_node->child[0] = assign_stmt();
	advance(KEYWORD_IN);
	current_node->child[1] = bexpr(true);
	advance(KEYWORD_END);
	return current_node;
}


TreeNode *getFuncCall(TreeNode * funcNode){
	funcNode->type = DATATYPE_FUNC;
	advance(LPAREN);
	funcNode->child[0] = getParameters();
	advance(RPAREN);
	funcNode->isCall = 1;
	return funcNode;
}


// false means only bexpr, true if can also be expr
TreeNode * bexpr(bool inParen) { // initial value of inParen is false

  TreeNode * t = NULL, * o = NULL;

  switch (token.TokenClass) {
  case INPUT:
	t = exp();
	return t;
	break;
  case SUB:
	t = exp();
	if (token.TokenClass == OPLESSEQ 
		|| token.TokenClass == OPGREATEREQ
		|| token.TokenClass == OPEQ
		|| token.TokenClass == OPLESS
		|| token.TokenClass == OPGREATER
		|| token.TokenClass == OPNOTEQ
		)
	{
      inExpr = 1;
      o = newNode(token);
      o->child[0] = t;
      advance(token.TokenClass);
      o->child[1] = exp();
	  o = bexpr_prime(o,false);
    }
	else
		return t;
	break;
  case KEYWORD_TRUE: 
  case KEYWORD_FALSE:
    t = newNode(token);
    if (t!=NULL)
      t->val = atoi(token.TokenString.c_str());
    advance(token.TokenClass);
    inExpr = 0;
    t = bexpr_prime(t,false);
    return t;
  case OPNOT:
		t = newNode(token);
		advance(OPNOT);
		t->child[0] = bexpr(false);
		return t;
  case NUM :
    inExpr = 2; 
    t = newNode(token);
    if ((t!=NULL) && (token.TokenClass==NUM))
      t->val = atoi(token.TokenString.c_str());
    advance(NUM);
    t = exp_prime(t);

    if (token.TokenClass == OPLESSEQ 
		|| token.TokenClass == OPGREATEREQ
		|| token.TokenClass == OPEQ
		|| token.TokenClass == OPLESS
		|| token.TokenClass == OPGREATER
		|| token.TokenClass == OPNOTEQ
		) {
      inExpr = 1; // comparison
      o = newNode(token);
      o->child[0] = t;
      advance(token.TokenClass);
      o->child[1] = exp();
    }
    else if (! inParen) {
      cerr << "num context incorrect" << endl;
      exit(1);
    }
    else o = t; // inExpr remains 2

    if (inExpr > 1 && token.TokenClass == OPAND) {
      cerr << "bexpr' context incorrect" << endl;
      exit(1);
    }
    o = bexpr_prime(o,false);
    break;
  case ID : 
    t = newNode(token);
    if ((t!=NULL) && (token.TokenClass==ID))
		strcpy(t->id , token.TokenString.c_str());
    advance(ID);

	//deal with call
	if (token.TokenClass == LPAREN){
		inExpr = 2;
		t = getFuncCall(t);
	}

	if(token.TokenClass == DIV || token.TokenClass == TIMES){
		inExpr = 2;
		t = term_prime(t);
	}

    if (token.TokenClass == PLUS || token.TokenClass == SUB
		) {
      inExpr = 2;
      t = exp_prime(t);
    }

    if (token.TokenClass == OPLESSEQ 
		|| token.TokenClass == OPGREATEREQ
		|| token.TokenClass == OPEQ
		|| token.TokenClass == OPLESS
		|| token.TokenClass == OPGREATER
		|| token.TokenClass == OPNOTEQ
		)
	{
      inExpr = 1;
      o = newNode(token);
      o->child[0] = t;
      advance(token.TokenClass);
      o->child[1] = exp();
    }
    else if (! inParen && t -> op != ID) {  // a stand alone expression must be 
                                            // inside parenthesis, except for ID
      cerr << "ID context incorrect" << endl;
      exit(1);
    }
    else o = t; // okay, inExpr remains 2 (or 0 if a single ID)

    if (inExpr > 1 && token.TokenClass == OPAND) {
      cerr << "bexpr' context incorrect" << endl;
      exit(1);
    }

    o = bexpr_prime(o,false);
    break;
  case KEYWORD_LET:
	t = getLet();
	return t;
	break;
  case KEYWORD_FN:
	t = getFuncDef();
	return t;
	break;
  case KEYWORD_IF:
	t = selection(); 
	return t;
	break;
  case LPAREN :
    advance(LPAREN);
    t = bexpr(true);
    advance(RPAREN);

    // for debugging: 
    // cout << token.TokenClass << " coming out with " << inExpr << endl;

    if ((token.TokenClass==PLUS) || (token.TokenClass==SUB)) {
	inExpr = 2;
	t = exp_prime(t);
    }

    if ((inExpr > 1 || t -> op == ID) && 
		(token.TokenClass == OPLESSEQ 
			|| token.TokenClass == OPGREATEREQ
			|| token.TokenClass == OPEQ
			|| token.TokenClass == OPLESS
			|| token.TokenClass == OPGREATER
			|| token.TokenClass == OPNOTEQ
			)) { 
                 // LE can only happen if we had arithmetic expression previously
      inExpr = 1;
      o = newNode(token);
      o->child[0] = t;
      advance(token.TokenClass);
      o->child[1] = exp();
    }
    else if (inExpr > 1 && ! inParen) {  // no comparison okay only if we are in parenthesis
      cerr << "paren 1 context incorrect" << endl;
      exit(1);
    }
    else o = t; // okay, inExpr remains 2 or 0

    if (inExpr > 1 && token.TokenClass == OPAND) {
      cerr << "paren bexpr' context incorrect" << endl;
      exit(1);
    }

    o = bexpr_prime(o,false);
    break;

  }
  return o;
}

TreeNode *selection_else(void){
	TreeNode *cur_root = NULL;
	if(token.TokenClass == KEYWORD_ELSE){
		advance(KEYWORD_ELSE);
		cur_root = statement();
	}
	return cur_root;
}

TreeNode *selection(void){	
	TreeNode *current_node =  newNodeWithOp(IFSELECTION);
	TreeNode *statement_parent = NULL; //to deal with &&
	advance(KEYWORD_IF);
	advance(LPAREN);
	if(current_node != NULL){
		current_node->child[0] = bexpr(false);
	}
	advance(RPAREN);
	if(current_node != NULL){
		current_node->child[1] = bexpr(true);
		advance(KEYWORD_ELSE);
		current_node->child[2] = bexpr(true);
	}
	return current_node;
}

TreeNode *iteration(void){
	TreeNode *current_node =  newNodeWithOp(ITERATION);
	TreeNode *statement_parent = NULL;
	advance(KEYWORD_WHILE); 
	advance(LPAREN);
	if(current_node != NULL){
		current_node->child[0] = bexpr(false);
	}
	advance(RPAREN); 
	if(current_node != NULL){
		ifInIteration++;
		current_node->child[1] = statement();
		ifInIteration--;
	}
	return current_node;
}

TreeNode *assign_stmt(){
	TreeNode * current_node = newNode(token);
	current_node->op = ASSIGN;
	strcpy(current_node->id, token.TokenString.c_str());
	advance(token.TokenClass);
	if (token.TokenClass != ASSIGN) {
	cerr << "Invalid assignment statement " 
	     << current_node->id << " " << token.TokenString << endl;
	exit(1);
	  }
	advance(ASSIGN);
	current_node->child[0] = bexpr(true);
	return current_node;
}

TreeNode *output_stmt(){
	TreeNode * current_node = newNode(token);
	if(token.TokenClass==OUT) {
      advance(token.TokenClass);
      current_node->child[0] = bexpr(false);
    }
	return current_node;
}

TreeNode *statement(){
	TreeNode *current_node = NULL;
	switch(token.TokenClass){
		case ID:
			current_node = assign_stmt();
			advance(SEMI);
			break;
		case OUT:
			current_node = output_stmt(); 
			advance(SEMI);
			break;
		default:
			break;
	}
	return current_node;
}

TreeNode *statements(){
	TreeNode *current_node = NULL;
	switch(token.TokenClass){
		case SEMI: // for null 
			advance(SEMI);
			current_node = statements();
			break;
		case ID:   // for assignment
		case OUT:  // for output
			current_node = statement();
			if(current_node != NULL){
				current_node->sibling = statements();
			}
			break;
			
	}
	return current_node;
}

TreeNode *local_vars(){
	TreeNode *current_node = NULL;
	switch (token.TokenClass){
		case KEYWORD_INT:
			advance(KEYWORD_INT);
			current_node = newNodeWithOp(DECNODE);
			strcpy(current_node->id, token.TokenString.c_str());
			advance(ID);
			advance(SEMI);
			
			if(current_node != NULL){
				current_node->type = DATATYPE_INT;
				current_node->sibling = local_vars();
			}
			break;
		case KEYWORD_BOOL:
			advance(KEYWORD_BOOL);
			current_node = newNodeWithOp(DECNODE);
			strcpy(current_node->id, token.TokenString.c_str());
			advance(ID);
			advance(SEMI);
			
			if(current_node != NULL){
				current_node->type = DATATYPE_BOOL;
				current_node->sibling = local_vars();
			}
			break;
		case KEYWORD_FUNDEF:
			advance(KEYWORD_FUNDEF);
			current_node = newNodeWithOp(DECNODE);
			strcpy(current_node->id, token.TokenString.c_str());
			advance(ID);
			advance(SEMI);
			
			if(current_node != NULL){
				current_node->type = DATATYPE_FUNC;
				current_node->sibling = local_vars();
			}
			break;
	}
	return current_node;
}

TreeNode *block() {
	TreeNode * current_node = newNodeWithOp(BLOCKNODE);
	advance(LBRKT); 
	if (current_node != NULL) {
		current_node->child[0] = local_vars(); 
		current_node->child[1] = statements(); 
	}
	advance(RBRKT);
	return current_node;
}

TreeNode *global_block() {
	TreeNode * current_node = newNodeWithOp(BLOCKNODE);
	if (current_node != NULL) {
		current_node->child[0] = local_vars(); 
		current_node->child[1] = statements(); 
	}
	return current_node;
}


TreeNode *program(){
	token = getToken();
	return global_block();
}

