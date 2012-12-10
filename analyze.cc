#include <iostream>
#include "globals.h"
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>


#define cur_scope scope[scope.size()-1] // use the end element as current scope.

std::vector <string> scope;
std::map <string, int> scopeMap;
/*       < scope_name, scope_index >    */

/* Semantic Analysis: checks for undefind variables */

extern bucket *ST_SCOPE_MAP[MAXSCOPE][HASH_TABLE_SIZE];
//bucket *ST_HASH[HASH_TABLE_SIZE];

int scope_index_counter = 0;
int tmpAddr = 0;
int block_index = 0;


/*
static bool ST[ST_SIZE] = {
    false, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false, false, false, false, false,
    false, false 
};
*/

void analyze_s_stmt(TreeNode*);
std::string get_scopename(char *id) {
	string scope_id;
	for(int i = scope.size()-1; i>= 0; i--){
		scope_id = scope[i];
		int index = scopeMap[scope_id];
		if(lookup(id, ST_SCOPE_MAP[index]) != NULL){
			return scope_id;
		}
	}
	return "";
}

void constant_folding(TreeNode* icode) {
  int op,val;
  op = icode->op;
  if ( (op==PLUS) || (op==SUB) || icode->op==TIMES || icode->op==DIV ) {
    if ( icode->child[0]->op != NUM ) {
      constant_folding( icode->child[0] );
    }
    if ( icode->child[1]->op != NUM ) {
      constant_folding( icode->child[1] );
    }
    if ( (icode->child[0]->op == NUM) && (icode->child[1]->op == NUM) ) {
      switch (op) {
        case PLUS:
          val=  icode->child[0]->val + icode->child[1]->val;
          break;
        case SUB:
          val=  icode->child[0]->val - icode->child[1]->val;
          break;
        case TIMES:
          val=  icode->child[0]->val * icode->child[1]->val;
          break;
        case DIV:
          if ( icode->child[1]->val == 0 ) {
            cerr << "Error! Division by zero!" << endl;
            exit(1);
          }
          val=  icode->child[0]->val / icode->child[1]->val;
          break;
        default:
          cerr << "error in constant_folding()" << endl;
          exit(1);
          break;
      }
      icode->op=NUM;
      icode->val=val;
      free( icode->child[0] );
      free( icode->child[1] );
      icode->child[0]=NULL;
      icode->child[1]=NULL;
    }
  } else {
    if ( icode->child[0] != NULL )
      constant_folding( icode->child[0] );
    if ( icode->child[1] != NULL )
      constant_folding( icode->child[1] );
  }
}


void analyzeExp(TreeNode* icode) {
    /* look for all occurrences of undefined variables */
    if (icode != NULL) {
	//if (icode -> op == ID && !ST[icode -> id - 'a']) {
	
	if (icode -> op == ID && get_scopename(icode->id) == "") {
	    cerr << "undefined variable: " << icode->id << endl;
	    exit(1);
	}
	else if((icode-> op == PLUS) || (icode->op == SUB) || (icode->op == TIMES)){
		constant_folding(icode);
	}
	else if(icode->op == DIV){
		constant_folding(icode->child[0]);
		constant_folding(icode->child[1]);
		if((icode->child[1]->op == NUM) && (icode->child[1]->val == 0)){
			cerr << "Error! Division by zero!" <<endl;
			exit(1);
		}
		constant_folding(icode);
	}
	else{
		
	}
	
	for (int i = 0; i < MAXCHILDREN; i++)
	    analyzeExp(icode -> child[i]);
  }
}

std::string getblockname(int i) {
	char s[8];
	std::string blockname;
	sprintf(s, "block%d", i);
	blockname = s;
	return blockname;
}


//additional util function.
int getAddrByid(char *id){
	string scope_id;
	for(int i = scope.size()-1; i>= 0; i--){
		scope_id = scope[i];
		int index = scopeMap[scope_id];
		bucket *tmp = lookup(id, ST_SCOPE_MAP[index]);
		if( tmp != NULL){
			return tmp->addr;
		}
	}
	cerr << "query for unknown variable: " << id << endl;
	exit(1);
	return -1;
}

int getTypeByid(char *id){
	string scope_id;
	for(int i = scope.size()-1; i>= 0; i--){
		scope_id = scope[i];
		int index = scopeMap[scope_id];
		bucket *tmp = lookup(id, ST_SCOPE_MAP[index]);
		if( tmp != NULL){
			return tmp->type;
		}
	}
	cerr << "query for unknown variable: " << id << endl;
	exit(1);
	return -1;
}

void analyzeBlock(TreeNode* icode){
	TreeNode* p= icode->child[0];
	std::string blockname;
	blockname = getblockname(block_index++);
	scope.push_back(blockname);
  	scopeMap[blockname] = scope_index_counter;
	while(p != NULL){
		if(p->op != DECNODE){
			cerr << "error in declaration check: " << icode->id << endl;
			exit(1);
		}
		char * tmpstr;
		tmpstr = new char[MAXIDLENGTH];
		strcpy(tmpstr, p->id);
		if(lookup(p->id, ST_SCOPE_MAP[scope_index_counter]) != NULL){
			cerr << "multiply declaration: " << p->id << endl;
			exit(1);
		}
		insert(p->id, tmpAddr, ST_SCOPE_MAP[scope_index_counter]);
		lookup(p->id, ST_SCOPE_MAP[scope_index_counter])->type = p->type;
		tmpAddr++;
		p = p->sibling;
	}
	scope_index_counter++;
	analyze_s_stmt( icode->child[1] );
	scope.pop_back();
}

void analyzeIter(TreeNode* icode) {
  analyzeExp( icode->child[0] );
  analyze_s_stmt( icode->child[1] );
}

void analyzeIFS(TreeNode* icode) {
  analyzeExp( icode->child[0] );
  analyze_s_stmt( icode->child[1] );
  analyze_s_stmt( icode->child[2] );
}


#define OPLESSEQ 280
#define OPGREATEREQ 281
#define OPEQ  282
#define OPLESS 283
#define OPGREATER 284
#define OPNOTEQ 285
#define OPAND  286
#define OPNOT  287
#define KEYWORD_TRUE   306
#define KEYWORD_FALSE   307

int isBexprSubtree(TreeNode *icode){
	if(icode->op == OPLESSEQ 
	|| icode->op == OPGREATEREQ 
	|| icode->op == OPEQ 
	|| icode->op == OPLESS
	|| icode->op == OPGREATER 
	|| icode->op == OPNOTEQ 
	|| icode->op == OPAND 
	|| icode->op == OPNOT 
	|| icode->op == KEYWORD_TRUE
	|| icode->op == KEYWORD_FALSE ){
		return 1;
	}
	return 0;
}

void analyze_s_stmt(TreeNode* icode) {
	while(icode != NULL){
		switch(icode -> op){
			case ASSIGN:
				if(get_scopename(icode->id) == ""){
					cerr << "undefined variable: " << icode->id << endl;
					exit(1);
				}
				if((getTypeByid(icode->id) == DATATYPE_BOOL && isBexprSubtree(icode->child[0]) == 1) 
					|| (getTypeByid(icode->id) == DATATYPE_INT && isBexprSubtree(icode->child[0]) == 0)){
						icode->type = getTypeByid(icode->id);
						analyzeExp( icode->child[0] );
					}
				else{
					cerr << "type must be match" <<endl;
					exit(1);
				}
				break;
			case OUT:
				analyzeExp(icode -> child[0]);
				break;
			case BLOCKNODE:
				if(scope_index_counter >= MAXSCOPE){
					cerr << "exceed max number of scopes: " << endl;
			        exit(1);
				}
				analyzeBlock(icode);
			    scope_index_counter++;
				break;
			case IFSELECTION:
				analyzeIFS(icode);
				break;
			case ITERATION:
				analyzeIter(icode);
				break;
			case KEYWORD_BREAK:
				icode->sibling = NULL;
				break; 
			default:
				cerr << "statements grammer error: " << icode->op << endl;
				exit(1);
		}
	
		icode = icode->sibling;
	}
}



void analyze(TreeNode* icode) {
    /* analyze all the statements in the program */
  	
	//scope.push_back( "global" );
	//scopeMap["global"] = scope_index_counter;
	//scope_index_counter++;
	
    while (icode != NULL) {
		switch(icode -> op){
			case ASSIGN:
				if(get_scopename(icode->id) == ""){
					cerr << "undefined variable: " << icode->id << endl;
				}
				analyzeExp(icode -> child[0]);
				break;
			case OUT:
				analyzeExp(icode -> child[0]);
				break;
			case BLOCKNODE:
				if(scope_index_counter >= MAXSCOPE){
					cerr << "exceed max number of scopes: " << endl;
			        exit(1);
				}
				analyzeBlock(icode);
			    scope_index_counter++;
				break;
			case IFSELECTION:
				analyzeIFS(icode);
				break;
			case ITERATION:
				analyzeIter(icode);
				break;
			case KEYWORD_BREAK:
				cerr << "invalid BREAK in scope: " << cur_scope << endl;
			     exit(1);
			default:
				cerr << "statements grammer error: " << icode->op << endl;
				exit(1);
		}
		icode = icode->sibling;
    }
}


