#include <iostream>
#include "globals.h"
#include <map>
#include <stdio.h>
#include <stdlib.h>


#define cur_scope scope[scope.size()-1] // use the end element as current scope.

//std::map <string, TypeTreeNode *> fuctionTreeMap;
// store the variables in different function.
std::map <string, std::map<string, bucket *> > variableMap; 


std::vector <string> scope;
std::map <string, int> scopeMap;
/*       < scope_name, scope_index >    */

/* Semantic Analysis: checks for undefind variables */

extern bucket *ST_SCOPE_MAP[MAXSCOPE][HASH_TABLE_SIZE];
//bucket *ST_HASH[HASH_TABLE_SIZE];

int scope_index_counter = 0;
int tmpAddr = 0;
int block_index = 0;


void analyze_s_stmt(TreeNode*);
void analyzeIFS(TreeNode* icode);

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
	else if(icode->op == LETNODE){
		insert(p->id, tmpAddr, ST_SCOPE_MAP[scope_index_counter]);
		lookup(p->id, ST_SCOPE_MAP[scope_index_counter])->type = p->type;
		
		bucket *tmp = getBucketByid(varName);
		std::string variablename(icode->child[0]->id);
		variableMap[functionname][variablename] = new bucket;
		variableMap[functionname][variablename]->type = DATATYPE_UNKOWN;
		analyze_s_stmt(icode->child[0]);
		analyzeExpinFunc(icode->child[1], functionname);
		
		typeTagging(icode->child[1], functionname, DATATYPE_UNKOWN);
		if(icode->child[1]->type != DATATYPE_UNKOWN)
			variableMap[functionname][variablename]->type = icode->child[1]->type;
	}
	else if(icode->op == IFSELECTION){
		analyzeIFS(icode);
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

bucket* getBucketByid(char *id){
	string scope_id;
	for(int i = scope.size()-1; i>= 0; i--){
		scope_id = scope[i];
		int index = scopeMap[scope_id];
		bucket *tmp = lookup(id, ST_SCOPE_MAP[index]);
		if( tmp != NULL){
			return tmp;
		}
	}
	cerr << "query for unknown variable: " << id << endl;
	exit(1);
	return NULL;
}


char *typeEncoding(int input){
	if(input == 320){
		return "INT";
	}else if(input == 321){
		return "BOOL";
	}else 
		return "UNKOWN";
}

std::string funcTypeToString(bucket *func){
	FunctionType *funcType = func->infer_type;
	std::string res = "( ";
	if(funcType != NULL){
		for(int i=0; i<funcType->parameterstype.size(); i++){
			if(funcType->parameterstype[i]->dataType == DATATYPE_INT || funcType->parameterstype[i]->dataType == DATATYPE_BOOL){
				res +=  typeEncoding(funcType->parameterstype[i]->dataType);
			}
			else if(funcType->parameterstype[i]->dataType == DATATYPE_FUNC){
				
			}else if(funcType->parameterstype[i]->dataType == DATATYPE_UNKOWN){
				std::string functioname(func->name);
				std::string variablename(funcType->parameterstype[i]->varid);
				if(variableMap[functioname][variablename]->type == DATATYPE_INT || variableMap[functioname][variablename]->type == DATATYPE_BOOL)
				{
					res +=  typeEncoding(variableMap[functioname][variablename]->type);
				}
			}		
			if(i != (funcType->parameterstype.size() -1))
				res += "X";
		}
	
		res += "=>";
		if(funcType->returnvaltype->dataType == DATATYPE_INT || funcType->returnvaltype->dataType == DATATYPE_BOOL){
			res +=  typeEncoding(funcType->returnvaltype->dataType);
		}
		else if(funcType->returnvaltype->dataType == DATATYPE_FUNC){
			
		}else if(funcType->returnvaltype->dataType == DATATYPE_UNKOWN){

		}
	}
	res += " )";
	return res;
}


void printTypeInference(char *varName){
	bucket *tmp = getBucketByid(varName);
	if(tmp != NULL){
		if(tmp->type == DATATYPE_FUNC){
			cout << varName << ":" << funcTypeToString(tmp) << endl;
		}else if(tmp->type == DATATYPE_INT || tmp->type == DATATYPE_BOOL){
			cout << varName << " : " << typeEncoding(tmp->type) << endl;
		}
	}
}

void output(TreeNode* icode){
	TreeNode *tmpNode = icode->child[0];
	while(tmpNode != NULL){
		printTypeInference(tmpNode->id);
		tmpNode = tmpNode->sibling;
	}
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
	output(icode);
	scope.pop_back();
}

void analyzeIter(TreeNode* icode) {
  analyzeExp( icode->child[0] );
  analyze_s_stmt( icode->child[1] );
}

void analyzeIFS(TreeNode* icode) {
  analyzeExp( icode->child[0] );
  analyzeExp( icode->child[1] );
  analyzeExp( icode->child[2] );
}

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

void setVariableType(char* varName, std::string functionname, int dataType){
	std::string varKey(varName);
	if(variableMap[functionname][varKey] != NULL){
		if(variableMap[functionname][varKey]->type == DATATYPE_UNKOWN || variableMap[functionname][varKey]->type == dataType){
			variableMap[functionname][varKey]->type = dataType;
		}
		else{
			cout << "conflicts in: " << varName <<"\t" <<dataType << "\t" <<variableMap[functionname][varKey]->type<<endl;
		}
	}
	else if(getTypeByid(varName) != -1){
		if(getBucketByid(varName)->type == DATATYPE_UNKOWN || getBucketByid(varName)->type == dataType){
			//variableMap[functionname][varKey]->type = dataType;
		}
		else{
			cout << "conflicts in: " << varName <<"\t" <<dataType << "\t" <<variableMap[functionname][varKey]->type<<endl;
		}
	}
	else{
		cout << "conflicts with the whole variable.: " <<endl;
	}
}

int checkParameterIfMatch(std::vector<VarType *> curparameters, std::vector<VarType *> preparameters){
	if(curparameters.size() != preparameters.size()){
		return -1;
	}
	for(int i=0; i< curparameters.size(); i++){
		if(curparameters[i]->dataType != DATATYPE_UNKOWN && curparameters[i]->dataType != preparameters[i]->dataType && preparameters[i]->dataType != DATATYPE_UNKOWN){
			return -1;
		}
	}
	return 1;
}
void setFunctionVariableType(char* varName, std::string functionname, int dataType, std::vector<VarType *> parameters, int returntype){
	std::string varKey(varName);
	if(variableMap[functionname][varKey] != NULL){
		if(variableMap[functionname][varKey]->infer_type == NULL){
			//set infer_type
			variableMap[functionname][varKey]->infer_type = new FunctionType;
			variableMap[functionname][varKey]->infer_type->parameterstype = parameters;
			variableMap[functionname][varKey]->infer_type->returnvaltype = new VarType;
			variableMap[functionname][varKey]->infer_type->returnvaltype->dataType = returntype;
		}else{
			int status = checkParameterIfMatch(parameters, variableMap[functionname][varKey]->infer_type->parameterstype);
			if(status == -1){
				cout << "function: " << varName <<"parameters doesn't match" << endl;
				exit(1);
			}
		}
	}
	else if(getTypeByid(varName) != -1){
		if(getBucketByid(varName)->infer_type == NULL){
			cout << "function: " << varName <<"undefined" << endl;
		}else{
			int status = checkParameterIfMatch(parameters, getBucketByid(varName)->infer_type->parameterstype);
			if(status == -1){
				cout << "function: " << varName <<"parameters doesn't match" << endl;
				exit(1);
			}
		}
	}
	else{
		cout << "function: " << varName <<" is not existing" << endl;
	}
}

//proposedType = -1 means no proposal, 
void typeTagging(TreeNode *icode, std::string functionname, int proposedType){
	if(icode->op == OPLESSEQ
	|| icode->op == OPGREATEREQ 
	|| icode->op == OPEQ 
	|| icode->op == OPLESS
	|| icode->op == OPGREATER 
	|| icode->op == OPNOTEQ){
		if(proposedType != DATATYPE_UNKOWN && proposedType != DATATYPE_BOOL){
			cout << "type inference error: " << icode->id <<" need to be DATATYPE_BOOL" <<endl;
		}
		icode->type = DATATYPE_BOOL;
		typeTagging(icode->child[0], functionname, DATATYPE_INT);
		typeTagging(icode->child[1], functionname, DATATYPE_INT);
	}
	else if(icode->op == OPNOT){
		if(proposedType != DATATYPE_UNKOWN && proposedType != DATATYPE_BOOL){
			cout << "type inference error: " << icode->id <<" need to be DATATYPE_BOOL" <<endl;
		}
		icode->type = DATATYPE_BOOL;
		typeTagging(icode->child[0], functionname, DATATYPE_BOOL);		
	}
	else if(icode->op == OPAND){
		if(proposedType != DATATYPE_UNKOWN && proposedType != DATATYPE_BOOL){
			cout << "type inference error: " << icode->id <<" need to be DATATYPE_BOOL" <<endl;
		}
		icode->type = DATATYPE_BOOL;
		typeTagging(icode->child[0], functionname, DATATYPE_BOOL);
		typeTagging(icode->child[1], functionname, DATATYPE_BOOL);
	}else if(icode->op == ID){
		if(icode->type == DATATYPE_FUNC){
			//Collect parameters
			std:vector<VarType *> parameters;
			TreeNode *parameterPointer = icode->child[0]->child[0];
			while(parameterPointer != NULL){
				typeTagging(parameterPointer, functionname, DATATYPE_UNKOWN);
				VarType *tmpType = new VarType;
				tmpType->dataType = parameterPointer->type;
				strcpy(tmpType->varid, parameterPointer->id);		
				parameters.push_back(tmpType);
				parameterPointer = parameterPointer->sibling;
			}
			
			setFunctionVariableType(icode->id, functionname, DATATYPE_FUNC, parameters, proposedType);
		}
		else{
			setVariableType(icode->id, functionname, proposedType);
		}
	}else if(icode->op == NUM){
		icode->type = DATATYPE_INT;
	}else if(icode->op == LETNODE){
		TreeNode *assignNode = icode->child[0];
		std::string variableName(assignNode->id);
		typeTagging(assignNode->child[0], functionname, DATATYPE_UNKOWN);
		variableMap[functionname][variableName] = new bucket;
		variableMap[functionname][variableName]->type = icode->child[0]->type;		
	}else if(icode->op == ASSIGN){
		typeTagging(icode->child[0], functionname, DATATYPE_UNKOWN);
		setVariableType(icode->id, functionname, icode->child[0]->type);
	}
	else{
		cout << "unhandled op" << icode->op << endl;
	}
}

void analyzeExpinFunc(TreeNode *icode, std::string functionname){
	/* look for all occurrences of undefined variables */
    if (icode != NULL) {
	//if (icode -> op == ID && !ST[icode -> id - 'a']) {
	std::string variableName(icode->id);
	if (icode -> op == ID && get_scopename(icode->id) == "" && variableMap[functionname][variableName] == NULL) {
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
	}else if(icode->op == LETNODE){
		std::string variablename(icode->child[0]->id);
		variableMap[functionname][variablename] = new bucket;
		variableMap[functionname][variablename]->type = DATATYPE_UNKOWN;
		analyze_s_stmt(icode->child[0]);
		analyzeExpinFunc(icode->child[1], functionname);
		
		typeTagging(icode->child[1], functionname, DATATYPE_UNKOWN);
		if(icode->child[1]->type != DATATYPE_UNKOWN)
			variableMap[functionname][variablename]->type = icode->child[1]->type;
	}	
	else if(icode->op == IFSELECTION){
		analyzeIFS(icode);
	}
	else{
		
	}
	for (int i = 0; i < MAXCHILDREN; i++)
	    analyzeExpinFunc(icode -> child[i], functionname);
  }
}

void analyze_functionContent(TreeNode* icode, std::string functionname){
	char *tmpStr;
	strcpy(tmpStr, functionname.c_str());;
	FunctionType *tmp = getBucketByid(tmpStr)->infer_type;

	//to check if all the variable is legal in context.
	analyzeExpinFunc(icode, functionname);
	//type inference
	typeTagging(icode, functionname, DATATYPE_UNKOWN);

	tmp->returnvaltype = new VarType;
	tmp->returnvaltype->dataType = icode->type;
	strcpy(tmp->returnvaltype->varid, icode->id);		
}


void analyze_parameters(TreeNode* icode, std::string functionname){
	char tmpStr[256];
	strcpy(tmpStr, functionname.c_str());;
	FunctionType *tmp = getBucketByid(tmpStr)->infer_type;
	std:vector<VarType *> parameters;
	while(icode != NULL){
		std::string variableName(icode->id);
		//update the variable table
		variableMap[functionname][variableName] = new bucket;
		variableMap[functionname][variableName]->type = DATATYPE_UNKOWN;
		//update functiontype tree
		VarType *tmpType = new VarType;
		tmpType->dataType = DATATYPE_UNKOWN;
		//tmpType->varid = icode->id;
		strcpy(tmpType->varid, icode->id);		
		parameters.push_back(tmpType);
		
		icode = icode->sibling;
	}
	//tmp->parameterstype = new std::vector;
	for(int i=0; i<parameters.size(); i++){
		tmp->parameterstype.push_back(parameters[i]);
	}
}

void analyze_func(TreeNode* icode){
	char *idname = icode->id;
	if(icode->child[0] == NULL || icode->child[0]->op != FUNCNODE){
		cout << "error in function define" << endl;
		exit(1);
	}
	std::string functionname(icode->id);
	getBucketByid(icode->id)->infer_type = new FunctionType;
	TreeNode *funcNode = icode->child[0];
	analyze_parameters(funcNode->child[0], functionname);
	analyze_functionContent(funcNode->child[1], functionname);
}

void analyze_s_stmt(TreeNode* icode){
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
						analyzeExp(icode->child[0]);
					}
				else if(getTypeByid(icode->id) == DATATYPE_FUNC){
					analyze_func(icode);
				}
				else{
					cerr << "type must be match" <<endl;
					exit(1);
				}
				break;
			case OUT:
				analyzeExp(icode -> child[0]);
				break;
			/*
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
			*/
			default:
				cerr << "statements grammer error: " << icode->op << endl;
				exit(1);
		}
		icode = icode->sibling;
	}
}

void analyze(TreeNode* icode) {	
    while (icode != NULL) {
		switch(icode -> op){
			case BLOCKNODE:
				if(scope_index_counter >= MAXSCOPE){
					cerr << "exceed max number of scopes: " << endl;
			        exit(1);
				}
				analyzeBlock(icode);
			    scope_index_counter++;
				break;
			default:
				cerr << "statements grammer error: " << icode->op << endl;
				exit(1);
		}
		icode = icode->sibling;
    }
}


