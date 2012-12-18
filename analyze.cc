#include <iostream>
#include "globals.h"
#include <map>
#include <stdio.h>
#include <stdlib.h>

// store the variables in different function.    <ID, the bucket for each variablle in ID section>
std::map <string, std::map<string, bucket *> > variableMap;

int generated_functIndex=0;
void analyzeIFS(TreeNode* icode, std::string scopename);

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

void setVariableType(char* varName, std::string scopename, int dataType){
	std::string varKey(varName);
	if(dataType == DATATYPE_UNKOWN){
		//do nothing
		return;
	}
	//look in local variables
	if(variableMap[scopename][varKey] != NULL){
		if(variableMap[scopename][varKey]->type == DATATYPE_UNKOWN || variableMap[scopename][varKey]->type == dataType){
			variableMap[scopename][varKey]->type = dataType;
		}else {
			cout << "conflicts in: " << varName <<"\t" <<dataType << "\t" <<variableMap[scopename][varKey]->type<<endl;
		}
	}
	//look in arguments 
	else if(variableMap[scopename + "_parameters"][varKey] != NULL){
		if(variableMap[scopename + "_parameters"][varKey]->type == DATATYPE_UNKOWN || variableMap[scopename + "_parameters"][varKey]->type == dataType){
			variableMap[scopename + "_parameters"][varKey]->type = dataType;
		}else {
			cout << "conflicts in: " << varName <<"\t" <<dataType << "\t" <<variableMap[scopename + "_parameters"][varKey]->type<<endl;
		}
	}
	//look in globle variables
	else if(variableMap[varKey][SELFTEXT] != NULL){
		if(variableMap[varKey][SELFTEXT]->type == DATATYPE_UNKOWN || variableMap[varKey][SELFTEXT]->type == dataType){
			variableMap[varKey][SELFTEXT]->type = dataType;
		}else {
			cout << "conflicts in: " << varName <<"\t" <<dataType << "\t" <<variableMap[varKey][SELFTEXT]->type<<endl;
		}
	}
	//no variable available.
	/*
	else{
		cout << "variable is not existing." <<endl;
	}
	*/
	int lastindex = scopename.find_last_of("-"); 
	if(lastindex >= 0){
		string lastScope = scopename.substr(0, lastindex); 
		setVariableType(varName, lastScope, dataType);
	}
}

void setFunctionVariableType(char* varName, std::string functionname, int dataType, std::vector<VarType *> parameters, int returntype){

}

int typeTagging(TreeNode *icode, std::string functionname, int proposedType){
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
	}
	else if((icode-> op == PLUS) || (icode->op == SUB) || (icode->op == TIMES) || (icode->op == DIV)){
		icode->type = DATATYPE_INT;
		typeTagging(icode->child[0], functionname, DATATYPE_INT);
		typeTagging(icode->child[1], functionname, DATATYPE_INT);
	}
	else if(icode->op == ID){
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
	return icode->type;
}

int ifVariableDefined(std::string idname, std::string scopename){	
	if(variableMap[scopename][idname] != NULL || variableMap[scopename + "_parameters"][idname] != NULL || variableMap[idname][SELFTEXT] != NULL){
		return 1;
	}
	int lastindex = scopename.find_last_of("-"); 
	if(lastindex >= 0){
		string lastScope = scopename.substr(0, lastindex); 
		return ifVariableDefined(idname, lastScope);
	}
	return -1;
}

void analyzeExp(TreeNode* icode, std::string scopename) {
	if(icode != NULL){
		if(icode ->op == ID){
			std::string id_name(icode->id);
			if(ifVariableDefined(id_name, scopename) == -1){
				cerr << "undefined variable: " << icode->id << endl;
				exit(1);
			}
			//deal with call
			if(icode->type == DATATYPE_FUNC){
				//check if the parameters match
				TreeNode *parameterPointer = icode->child[0];
				std::vector<VarType *> parameters;
				while(parameterPointer != NULL){
					analyzeExp(parameterPointer, scopename);
					typeTagging(parameterPointer, id_name, DATATYPE_UNKOWN);
					VarType *tmpVarType = new VarType;
					tmpVarType->dataType = parameterPointer->type;
					parameters.push_back(tmpVarType);
					parameterPointer = parameterPointer->sibling;
				}
				if(checkParameterIfMatch(parameters, variableMap[scopename][id_name]->infer_type->parameterstype) != 1){
					cerr << "Function call: " << icode->id << "parameter doesn't match"<< endl;
					exit(1);
				}
			}
		}
		else if((icode-> op == PLUS) || (icode->op == SUB) || (icode->op == TIMES)){
			constant_folding(icode);
			typeTagging(icode, scopename, DATATYPE_INT);
		}
		else if(icode->op == DIV){
			constant_folding(icode->child[0]);
			constant_folding(icode->child[1]);
			if((icode->child[1]->op == NUM) && (icode->child[1]->val == 0)){
				cerr << "Error! Division by zero!" <<endl;
				exit(1);
			}
			constant_folding(icode);
			typeTagging(icode, scopename, DATATYPE_INT);
		}
		else if(icode-> op >= 280 && icode-> op<=285){
			//for comparison
			analyzeExp(icode->child[0], scopename);
			analyzeExp(icode->child[0], scopename);
			typeTagging(icode, scopename, DATATYPE_BOOL);
		}
		else if(icode-> op == OPAND){
			analyzeExp(icode->child[0], scopename);
			analyzeExp(icode->child[0], scopename);
			typeTagging(icode, scopename, DATATYPE_BOOL);
		}
		else if(icode-> op == OPNOT){
			analyzeExp(icode->child[0], scopename);
			typeTagging(icode, scopename, DATATYPE_BOOL);
		}else if(icode->op == LETNODE){
			//let node.
			//update variable table.
			std::string variableName(icode->child[0]->id);
			variableMap[scopename][variableName] = new bucket;
			variableMap[scopename][variableName]->type = DATATYPE_UNKOWN;
			typeTagging(icode->child[0]->child[0], scopename, DATATYPE_UNKOWN);
			variableMap[scopename][variableName]->type = icode->child[0]->child[0]->type;
			
			analyzeExp(icode->child[1], scopename);
		}
		else if(icode->op == IFSELECTION){
			analyzeIFS(icode, scopename);
		}
		else if(icode->op == FUNCNODE){
			/*
			analyze_parameters(icode);
			analyzeExp(icode, functionname);
			*/
			std::string generated_functname = scopename + "-FUNC" + generated_functIndex;
			icode->comment = generated_functname;
			generated_functIndex++;
			
			//analyze parameter part.
			TreeNode *p = icode->child[0];
			while(p!=NULL){
				std::string variableName(p->id);
				variableMap[generated_functname+ "_parameters"][variableName] = new bucket;
				variableMap[generated_functname+ "_parameters"][variableName]->type = DATATYPE_UNKOWN;
			}
			
			//analyze function content part.
			analyzeExp(icode->child[1], generated_functname);
		}
		else{
			cerr << "unhandle op: "<< icode->op <<endl;
		}	
	}
}

void analyze_parameters(TreeNode* icode){
	std::string functionname(icode->id);
	std::string scopename = functionname + "_parameters";
	TreeNode *p = icode->child[0]->child[0];
	while(p != NULL){
		std::string variableName(p->id);
		//update the variable table
		variableMap[scopename+ "_parameters"][variableName] = new bucket;
		variableMap[scopename+ "_parameters"][variableName]->type = DATATYPE_UNKOWN;
		//update functiontype tree
		VarType *tmpType = new VarType;
		tmpType->dataType = DATATYPE_UNKOWN;
		strcpy(tmpType->varid, icode->id);		
	
		variableMap[functionname][defaultName]->infer_type->parameterstype.push_back(tmpType);
	}
}

void analyzeIFS(TreeNode* icode, std::string scopename) {
  analyzeExp( icode->child[0], scopename);
  analyzeExp( icode->child[1], scopename );
  analyzeExp( icode->child[2], scopename );
	int ifType = typeTagging(icode->child[0], functionname, DATATYPE_UNKOWN);
	int elseType = typeTagging(icode->child[0], functionname, DATATYPE_UNKOWN);
	if(ifType != elseType){
		cout << "the expression in if & else doesn't match" << endl;
		exit(1);
	}
}

void analyze_func(TreeNode* icode){
	if(icode->child[0] == NULL || icode->child[0]->op != FUNCNODE){
		cout << "error in function define" << endl;
		exit(1);
	}
	std::string functionname(icode->id);
	variableMap[functionname][SELFTEXT]->infer_type = new FunctionType;
	analyze_parameters(icode);
	analyzeExp(icode, functionname);
}

void analyze_s_stmt(TreeNode* icode){
	while(icode != NULL){
		std::string scopename(icode->id);
		switch(icode -> op){
			case ASSIGN:
				if(variableMap[scopename][SELFTEXT] == NULL ){
					cerr << "undefined variable: " << icode->id << endl;
					exit(1);
				}
				else if((variableMap[scopename][SELFTEXT]->type == DATATYPE_BOOL && isBexprSubtree(icode->child[0]) == 1)
				|| (variableMap[scopename][SELFTEXT]->type == DATATYPE_INT && isBexprSubtree(icode->child[0]) == 0){
					icode->type = variableMap[scopename][SELFTEXT]->type;
					analyzeExp(icode->child[0], scopename);
				}
				else if(variableMap[scopename][SELFTEXT]->type == DATATYPE_FUNC){
					analyze_func(icode);
				}
				else{
					cerr << "type must be match" <<endl;
					exit(1);
				}
				break;
			case OUT:
				scopename = "global";
				analyzeExp(icode -> child[0], "global");
				break;
			default:
				cerr << "statements grammer error: " << icode->op << endl;
				exit(1);
			}
			icode = icode->sibling;
		}
}



void analyzeBlock(TreeNode* icode){
	TreeNode* p= icode->child[0];
	while(p != NULL){
		if(p->op != DECNODE){
			cerr << "error in declaration check: " << icode->id << endl;
			exit(1);
		}
		std::string scopename(p->id);

		variableMap[scopename][SELFTEXT] = new bucket;
		variableMap[scopename][SELFTEXT]->type = p->type;
		//variableMap[var_str][defaultName]->addr = ;
		p = p->sibling;
	}
	analyze_s_stmt(icode->child[1]);
	//output(icode);
}

void analyze(TreeNode* icode) {	
    while (icode != NULL) {
		switch(icode -> op){
			case BLOCKNODE:
				analyzeBlock(icode);
				break;
			default:
				cerr << "statements grammer error: " << icode->op << endl;
				exit(1);
		}
		icode = icode->sibling;
    }
}