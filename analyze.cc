#include <iostream>
#include "globals.h"
#include <map>
#include <stdio.h>
#include <stdlib.h>

// store the variables in different function.    <ID, the bucket for each variablle in ID section>
std::map <string, std::map<string, bucket *> > variableMap;
std::map <string, TreeNode *> functionCalleeMap;

int gloablAddr = 0;
int generated_functIndex=0;
void analyzeIFS(TreeNode* icode, std::string scopename);

int unknownparameterCounter = 0;
char *unknownparameterCollection = "123456789";


bucket *getFunctionBucketbyNameScope(std::string name, std::string scopename){
	if(variableMap[scopename][name] != NULL){
		return variableMap[scopename][name];
	} 
	if(variableMap[scopename+"_parameters"][name] != NULL){
		return variableMap[scopename+"_parameters"][name];
	}else if(variableMap[name][SELFTEXT] != NULL){
		return variableMap[name][SELFTEXT];
	}
	else{
		cerr<<"Debug support: error in type tagging" << endl;
	}
}


bucket *getVariableBucketByIDScope(std::string idname, std::string scopename){	
	if(variableMap[scopename][idname] != NULL){
		return variableMap[scopename][idname];
	}
	if(variableMap[scopename + "_parameters"][idname] != NULL){
		return variableMap[scopename + "_parameters"][idname];
	}
	if(variableMap[idname][SELFTEXT] != NULL){
		return variableMap[idname][SELFTEXT];
	}
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

TreeNode *getReturnVarType(TreeNode *icode){
	if(icode->op == LETNODE){
		return getReturnVarType(icode->child[1]);
	}else if(icode->op == IFSELECTION){
		return getReturnVarType(icode->child[1]);
	}
	else{
		while(icode->isCall == 1 &&  icode->child[1] != NULL){
			icode = icode->child[1];
		}
		return icode;
	}
}

TreeNode *getEqualExpr(TreeNode *icode){
	if(icode->op == LETNODE){
		return getReturnVarType(icode->child[1]);
	}else if(icode->op == IFSELECTION){
		return getReturnVarType(icode->child[1]);
	}
	else{
		while( icode != NULL && icode->type == DATATYPE_FUNC && icode->child[1] != NULL){
			icode = icode->child[1];
		}
		return icode;
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

void setFunctionVariableType(char* varName, std::string functionname, int dataType, std::vector<VarType *> parameters, VarType *returntype){
	std::string varKey(varName);
	if(dataType != DATATYPE_FUNC){
		return;
	}
	if(variableMap[varKey][SELFTEXT] == NULL){
		if(variableMap[functionname+ "_parameters"][varKey] == NULL && variableMap[functionname][varKey]){
			cout << "Function - " << varKey << " couln't be found." << endl;
		}
		//could add conflict check.
	}
	else if(variableMap[varKey][SELFTEXT]->infer_type == NULL){
		variableMap[varKey][SELFTEXT]->infer_type = new FunctionType;
		variableMap[varKey][SELFTEXT]->infer_type->parameterstype = parameters;
		variableMap[varKey][SELFTEXT]->infer_type->returnvaltype->dataType = returntype->dataType;
	}
	else{
		int ifmatch = checkParameterIfMatch(variableMap[varKey][SELFTEXT]->infer_type->parameterstype, parameters);
		if(ifmatch == -1){
			cout << varName << "parameter doesn't match." << endl;
		}
		if(returntype->dataType == DATATYPE_UNKOWN){
			//do nothing.
		}
		else if(variableMap[varKey][SELFTEXT]->infer_type->returnvaltype->dataType == DATATYPE_UNKOWN){
			variableMap[varKey][SELFTEXT]->infer_type->returnvaltype->dataType = returntype->dataType;
		} 
		else{
			cout << varName << "return type doesn't match." << endl;
		}
	}
}

int typeTagging(TreeNode *icode, std::string functionname, int proposedType){
	switch(icode -> op){
		case OPLESSEQ:
		case OPGREATEREQ:
		case OPEQ:
		case OPLESS:
		case OPGREATER:
		case OPNOTEQ:
			if(proposedType != DATATYPE_UNKOWN && proposedType != DATATYPE_BOOL){
				cout << "type inference error: " << icode->id <<" need to be DATATYPE_BOOL" <<endl;
			}
			icode->type = DATATYPE_BOOL;
			typeTagging(icode->child[0], functionname, DATATYPE_INT);
			typeTagging(icode->child[1], functionname, DATATYPE_INT);
			break;
		case OPNOT:
			if(proposedType != DATATYPE_UNKOWN && proposedType != DATATYPE_BOOL){
				cout << "type inference error: " << icode->id <<" need to be DATATYPE_BOOL" <<endl;
			}
			icode->type = DATATYPE_BOOL;
			typeTagging(icode->child[0], functionname, DATATYPE_BOOL);
			break;
		case OPAND:
			if(proposedType != DATATYPE_UNKOWN && proposedType != DATATYPE_BOOL){
				cout << "type inference error: " << icode->id <<" need to be DATATYPE_BOOL" <<endl;
			}
			icode->type = DATATYPE_BOOL;
			typeTagging(icode->child[0], functionname, DATATYPE_BOOL);
			typeTagging(icode->child[1], functionname, DATATYPE_BOOL);
			break;
		case PLUS:
		case SUB:
		case TIMES:
		case DIV:
			icode->type = DATATYPE_INT;
			typeTagging(icode->child[0], functionname, DATATYPE_INT);
			typeTagging(icode->child[1], functionname, DATATYPE_INT);
			break;
		case ID:
			//deal with call.
			if(icode->type == DATATYPE_FUNC){
				if(proposedType != DATATYPE_UNKOWN){
					//getEqualExpr(icode)->type = proposedType;
					
					string returncodeVal(getEqualExpr(icode)->id);
					
					if(getVariableBucketByIDScope(returncodeVal, functionname)->infer_type == NULL){
						getVariableBucketByIDScope(returncodeVal, functionname)->infer_type = new FunctionType;
					}
					if(	getVariableBucketByIDScope(returncodeVal, functionname)->infer_type->returnvaltype == NULL){
						getVariableBucketByIDScope(returncodeVal, functionname)->infer_type->returnvaltype = new VarType;
					}
					getVariableBucketByIDScope(returncodeVal, functionname)->infer_type->returnvaltype->dataType = proposedType;
				}
			}
			else{
				string idcodestr(icode->id);
				if(proposedType != DATATYPE_UNKOWN){
					
					if(getVariableBucketByIDScope(idcodestr, functionname)->infer_type == NULL){
						getVariableBucketByIDScope(idcodestr, functionname)->infer_type = new FunctionType;
					}
					if(	getVariableBucketByIDScope(idcodestr, functionname)->infer_type->returnvaltype == NULL){
						getVariableBucketByIDScope(idcodestr, functionname)->infer_type->returnvaltype = new VarType;
					}
					getVariableBucketByIDScope(idcodestr, functionname)->infer_type->returnvaltype->dataType = proposedType;
				}
			}
			/*
			if(icode->type == DATATYPE_FUNC){
				//Collect parameters
				std:vector<VarType *> parameters;
				TreeNode *parameterPointer = icode->child[0];
				while(parameterPointer != NULL){
					typeTagging(parameterPointer, functionname, DATATYPE_UNKOWN);
					VarType *tmpType = new VarType;
					tmpType->dataType = parameterPointer->type;
					strcpy(tmpType->varid, parameterPointer->id);		
					parameters.push_back(tmpType);
					parameterPointer = parameterPointer->sibling;
				}
				VarType *returnType = new VarType;
				returnType->dataType = proposedType;
				setFunctionVariableType(icode->id, functionname, DATATYPE_FUNC, parameters, returnType);
			}
			else{
				std::string varKey(icode->id);
				icode->type = getVariableBucketByIDScope(varKey, functionname)->type;
				setVariableType(icode->id, functionname, proposedType);
			}*/
			break;
		case NUM:
			icode->type = DATATYPE_INT;
			break;
		case LETNODE:{
			TreeNode *assignNode = icode->child[0];
			std::string variableName(assignNode->id);
			typeTagging(assignNode->child[0], functionname, DATATYPE_UNKOWN);			
			variableMap[functionname][variableName] = new bucket;
			
			variableMap[functionname][variableName]->infer_type = new functionType;
			variableMap[functionname][variableName]->infer_type ->returnvaltype = new VarType;
			variableMap[functionname][variableName]->infer_type ->returnvaltype->dataType = icode->child[0]->type;
			break;
			}
		case ASSIGN:
			typeTagging(icode->child[0], functionname, DATATYPE_UNKOWN);
			setVariableType(icode->id, functionname, icode->child[0]->type);
			break;
		case KEYWORD_TRUE:
		case KEYWORD_FALSE:
			icode->type = DATATYPE_BOOL;
			break;
		case FUNCNODE:
			icode->type = DATATYPE_FUNC;
			if(strcmp(icode->id,"") == 0){
				std::string commentname(icode->comment);
				
				//Collect parameters
				std::vector<VarType *> parameters;
				TreeNode *parameterPointer = icode->child[0];
				while(parameterPointer != NULL){
					typeTagging(parameterPointer, commentname, DATATYPE_UNKOWN);
					VarType *tmpType = new VarType;
					tmpType->dataType = parameterPointer->type;
					strcpy(tmpType->varid, parameterPointer->id);		
					parameters.push_back(tmpType);
					parameterPointer = parameterPointer->sibling;
				}
				VarType *returnType = new VarType;
				typeTagging(icode->child[1], commentname, DATATYPE_UNKOWN);
				returnType->dataType = icode->child[1]->type;
				setFunctionVariableType(icode->comment, commentname, DATATYPE_FUNC, parameters, returnType);
			}
			else{
				std::string functionidname(icode->id);
				
				//Collect parameters
				std::vector<VarType *> parameters;
				TreeNode *parameterPointer = icode->child[0];
				while(parameterPointer != NULL){
					typeTagging(parameterPointer, functionidname, DATATYPE_UNKOWN);
					VarType *tmpType = new VarType;
					tmpType->dataType = parameterPointer->type;
					strcpy(tmpType->varid, parameterPointer->id);		
					parameters.push_back(tmpType);
					parameterPointer = parameterPointer->sibling;
				}
				VarType *returnType = new VarType;
				typeTagging(icode->child[1], functionidname, DATATYPE_UNKOWN);
				returnType->dataType = icode->child[1]->type;
				setFunctionVariableType(icode->comment, functionidname, DATATYPE_FUNC, parameters, returnType);
			}
			break;
		case IFSELECTION:
			typeTagging(icode->child[1], functionname, proposedType);
			typeTagging(icode->child[2], functionname, proposedType);
			break;
		default:
			cout << "Debug support: unhandled op: " << icode->op << endl;
			exit(1);
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

TreeNode *deepCopy(TreeNode *icode){
	if(icode == NULL){
		return NULL;
	}
	TreeNode *res = (TreeNode *) malloc(sizeof(TreeNode));
	for(int i=0; i <MAXCHILDREN; i++){
		res->child[i] = deepCopy(icode->child[i]);
	}
	res->sibling = deepCopy(icode->sibling);
	res->op =  icode->op;
	res->val = icode->val;
	strcpy(res->id, icode->id);
	res->type = icode->type;
	res->st_hash_index = icode->st_hash_index;
	strcpy(res->comment, icode->comment);
	res->isCopy = 1;
	res->isCall = icode->isCall;
	return res;
}

void replace(TreeNode *icode, std::map<std::string, VarType*> replacementMap, std::map<string, TreeNode *> replacementVarParametersMap){
	if(icode == NULL){
		return;
	}
	std::string icode_id(icode->id);
	if(replacementMap[icode_id] != NULL){
		icode->type = replacementMap[icode_id]->dataType;
		strcpy(icode->id, replacementMap[icode_id]->varid);
		if(replacementVarParametersMap[icode_id] != NULL){
			icode->child[0] = replacementVarParametersMap[icode_id];
		}
	}
	for(int i=0; i <MAXCHILDREN; i++){
		replace(icode->child[i], replacementMap, replacementVarParametersMap);
	}
	replace(icode->sibling, replacementMap, replacementVarParametersMap);
}

void analyzeExp(TreeNode* icode, std::string scopename) {
	if(icode != NULL){
		if(icode ->op == ID){
			std::string id_name(icode->id);
			if(ifVariableDefined(id_name, scopename) == -1){
				cerr << "undefined variable: " << icode->id << endl;
				exit(1);
			}
			if(getVariableBucketByIDScope(id_name, scopename)->type == DATATYPE_FUNC){
				//if(getVariableBucketByIDScope(id_name, scopename)->infer_type == NULL){
				if((variableMap[id_name][SELFTEXT]!= NULL && (variableMap[id_name][SELFTEXT]->infer_type != NULL || variableMap[id_name][SELFTEXT]->type == DATATYPE_FUNC) )
				|| (variableMap[scopename][id_name]!= NULL && (variableMap[scopename][id_name]->infer_type != NULL || variableMap[scopename][id_name]->type == DATATYPE_FUNC)) 
				|| functionCalleeMap[id_name]!= NULL){
					
				}else{
					bucket * tmp = variableMap[id_name][SELFTEXT];
					tmp = variableMap[scopename][id_name];
					cerr << "function name was defined but the coentent was not decleared: " << icode->id << endl;
					exit(1);
				}
			}
			//deal with call
			if(icode->type == DATATYPE_FUNC){
				bucket *tmpBucket = getVariableBucketByIDScope(id_name, scopename);
				tmpBucket->type = DATATYPE_FUNC;
				
				if(functionCalleeMap[id_name] != NULL){
					// add the real function to the specific call.
					TreeNode *functionNode = functionCalleeMap[id_name];
					icode->child[1] = deepCopy(functionNode->child[1]);
					/*
					TreeNode *parameteraddrTagger = icode->child[0];

					int parameterCounter = 0;
					while(parameteraddrTagger != NULL){
						if(parameteraddrTagger->op == ID &&parameteraddrTagger->type == DATATYPE_FUNC){
							//do nothing.
						}else{
							std::string parameterID(parameteraddrTagger->id);
							getVariableBucketByIDScope(parameterID, id_name)->addr = parameterCounter;
							parameterCounter++;
						}
						parameteraddrTagger = parameteraddrTagger->sibling;
					}
					parameteraddrTagger = icode->child[0];
					while(parameteraddrTagger != NULL){
						if(parameteraddrTagger->op == ID &&parameteraddrTagger->type == DATATYPE_FUNC){
							//do nothing.
						}else{
							std::string parameterID(parameteraddrTagger->id);
							getVariableBucketByIDScope(parameterID, id_name)->addr =parameterCounter-getVariableBucketByIDScope(parameterID, id_name)->addr;	
						}
						parameteraddrTagger = parameteraddrTagger->sibling;
					}
					*/
					// replace the parameter to the real function.
					std::map<string, VarType*> replacementMap;
					std::map<string, TreeNode *> replacementVarParametersMap;
					TreeNode *parameterPointer = icode->child[0];
					TreeNode *originalPointer = functionNode->child[0];
					while(parameterPointer != NULL){
						if(parameterPointer->op == ID &&parameterPointer->type == DATATYPE_FUNC){
							if(parameterPointer->type != originalPointer->type && originalPointer->type != DATATYPE_UNKOWN && parameterPointer->type != DATATYPE_UNKOWN){
								cout<< "parameters type doesn't match! "<<endl;
							}
							std::string orignalstr(originalPointer->id);
							replacementMap[orignalstr] = new VarType;
							replacementMap[orignalstr]->dataType = parameterPointer->type;
							strcpy(replacementMap[orignalstr]->varid, parameterPointer->id);
							replacementVarParametersMap[orignalstr]= deepCopy(parameterPointer->child[0]);
						}
						else{
							if(parameterPointer->type != originalPointer->type && originalPointer->type != DATATYPE_UNKOWN && parameterPointer->type != DATATYPE_UNKOWN){
								cout<< "parameters type doesn't match! "<<endl;
							}
							std::string orignalstr(originalPointer->id);
							replacementMap[orignalstr] = new VarType;
							replacementMap[orignalstr]->dataType = parameterPointer->type;
							strcpy(replacementMap[orignalstr]->varid, parameterPointer->id);
						}
						parameterPointer = parameterPointer->sibling;
						originalPointer = originalPointer->sibling;
					}
					if(originalPointer != NULL){
						cout<< "number of parameters doesn't match! "<<endl;
					}
					replace(icode->child[1], replacementMap, replacementVarParametersMap);
					std::string nxtfuncName(icode->child[1]->id);
					if(icode->child[1]->type == DATATYPE_UNKOWN){
						if(getVariableBucketByIDScope(nxtfuncName, scopename)!= NULL)
							icode->child[1]->type = getVariableBucketByIDScope(nxtfuncName, scopename)->type;
					}
					analyzeExp(icode->child[1], scopename);
				}
				else{
					TreeNode *parameterPointer = icode->child[0];
					std::vector<VarType *> parameters;
					while(parameterPointer != NULL){
						analyzeExp(parameterPointer, scopename);
						typeTagging(parameterPointer, scopename, DATATYPE_UNKOWN);
						VarType *tmpVarType = new VarType;
						tmpVarType->dataType = parameterPointer->type;
						strcpy(tmpVarType->varid, parameterPointer->id);
						parameters.push_back(tmpVarType);
						parameterPointer = parameterPointer->sibling;
					}
					bucket *tmpBucket = getVariableBucketByIDScope(id_name, scopename);
					tmpBucket->type = DATATYPE_FUNC;
					
					tmpBucket->infer_type = new FunctionType;
					tmpBucket->infer_type->parameterstype = parameters;
					tmpBucket->infer_type->returnvaltype = new VarType;
					tmpBucket->infer_type->returnvaltype->dataType = DATATYPE_UNKOWN;
					if(strcmp(tmpBucket->infer_type->returnvaltype->varid, "") == 0){
						tmpBucket->infer_type->returnvaltype->varid[0] = unknownparameterCollection[unknownparameterCounter];
						unknownparameterCounter++;
					}
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
			analyzeExp(icode->child[1], scopename);
			typeTagging(icode, scopename, DATATYPE_BOOL);
		}
		else if(icode-> op == OPAND){
			analyzeExp(icode->child[0], scopename);
			analyzeExp(icode->child[1], scopename);
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
			typeTagging(icode->child[0]->child[0], scopename, DATATYPE_UNKOWN);
			variableMap[scopename][variableName]->addr = -2;
			variableMap[scopename][variableName]->infer_type = new functionType;
			variableMap[scopename][variableName]->infer_type ->returnvaltype = new VarType;
			variableMap[scopename][variableName]->infer_type ->returnvaltype->dataType = icode->child[0]->child[0]->type;
			
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
			char numstr[21]; // enough to hold all numbers up to 64-bits
			sprintf(numstr, "%d", generated_functIndex);
			std::string generated_functname = scopename + "-FUNC" + numstr;
			generated_functIndex++;
			strcpy(icode->comment, generated_functname.c_str());
			
			variableMap[generated_functname][SELFTEXT] = new bucket;
			strcpy(variableMap[generated_functname][SELFTEXT]->name, icode->comment);
			variableMap[generated_functname][SELFTEXT]->type = DATATYPE_FUNC;
			//analyze parameter part.
			TreeNode *p = icode->child[0];
			while(p!=NULL){
				std::string variableName(p->id);
				variableMap[generated_functname+ "_parameters"][variableName] = new bucket;
				variableMap[generated_functname+ "_parameters"][variableName]->type = DATATYPE_UNKOWN;
				p= p->sibling;
			}
			
			//analyze function content part.
			analyzeExp(icode->child[1], generated_functname);
		}
		else if(icode->op == KEYWORD_FALSE || icode->op == KEYWORD_TRUE){
			icode->type = DATATYPE_BOOL;
		}
		else if(icode->op == NUM){
			icode->type = DATATYPE_INT;
		}
		else{
			cerr << "unhandle op: "<< icode->op <<endl;
		}	
	}
}

std::string getUniqueID(TreeNode *icode){
	if(strcmp(icode->id, "") != 0 ){
		std::string res(icode->id);
		return res;
	}else if(strcmp(icode->comment, "")!=0){
		std::string res(icode->comment);
		return res;
	}else
		return NULL;
}

int ifFuctionTypeMatch(FunctionType *ftone, FunctionType *fttwo){
	if(checkParameterIfMatch(ftone->parameterstype , fttwo->parameterstype) != 1){
		return -1;
	}
	if(ftone->returnvaltype->dataType != fttwo->returnvaltype->dataType && fttwo->returnvaltype->dataType !=DATATYPE_UNKOWN && ftone->returnvaltype->dataType != DATATYPE_UNKOWN)
	{
		return -1;
	}
	return 1;
}

void unify(TreeNode* icode1, TreeNode* icode2, std::string scopename){
	TreeNode *returnNode1 = getReturnVarType(icode1);
	TreeNode *returnNode2 = getReturnVarType(icode2);
	
	if(returnNode1->type == DATATYPE_UNKOWN){
		typeTagging(icode1, scopename, returnNode2->type);
	}else if(returnNode2->type == DATATYPE_UNKOWN){
		typeTagging(icode2, scopename, returnNode1->type);
	}else if((returnNode2->type == DATATYPE_INT || returnNode2->type == DATATYPE_BOOL) && returnNode1->type == returnNode2->type){
		//do nothing.
	}else{
		//unify the parameter.
	}
}

void analyzeIFS(TreeNode* icode, std::string scopename) {
  analyzeExp( icode->child[0], scopename);
  analyzeExp( icode->child[1], scopename );
  analyzeExp( icode->child[2], scopename );
unify(icode->child[1], icode->child[2], scopename);
/*
	int ifType = typeTagging(icode->child[1], scopename, DATATYPE_UNKOWN);
	int elseType = typeTagging(icode->child[2], scopename, DATATYPE_UNKOWN);
	if(ifType == elseType && ifType ==DATATYPE_FUNC){
		//check parameter if match
		if(ifFuctionTypeMatch(variableMap[getUniqueID(icode->child[1])][SELFTEXT]->infer_type, variableMap[getUniqueID(icode->child[2])][SELFTEXT]->infer_type) != 1)
		{
			cout << "the expression in if & else doesn't match"  << endl;
			exit(1);
		}
	}
	if(ifType != elseType){
		cout << "the expression in if & else doesn't match" << endl;
		exit(1);
	}
*/
}

void analyze_parameters(TreeNode* icode){
	std::string functionname(icode->id);
	std::string params_scopename = functionname + "_parameters";
	TreeNode *p = icode->child[0]->child[0];
	int parameterCounter = 0;
	while(p != NULL){
		std::string variableName(p->id);
		//update the variable table
		variableMap[params_scopename][variableName] = new bucket;
		variableMap[params_scopename][variableName]->type = DATATYPE_UNKOWN;

		variableMap[params_scopename][variableName]->addr = parameterCounter;
		//update functiontype tree
		VarType *tmpType = new VarType;
		tmpType->dataType = DATATYPE_UNKOWN;
		strcpy(tmpType->varid, p->id);		
		
		variableMap[functionname][SELFTEXT]->infer_type->parameterstype.push_back(tmpType);
		p = p->sibling;
		parameterCounter++;
	}
	p = icode->child[0]->child[0];
	while(p != NULL){
		std::string variableName(p->id);
		variableMap[params_scopename][variableName]->addr = parameterCounter - variableMap[params_scopename][variableName]->addr;
		p = p->sibling;
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
	analyzeExp(icode->child[0]->child[1], functionname);
	//update infer_type.
	//update return_type
	VarType *tmpType = new VarType;
	tmpType->dataType = getReturnVarType(icode->child[0]->child[1])->type;
	tmpType->isCall = getReturnVarType(icode->child[0]->child[1])->isCall;
	strcpy(tmpType->varid, getReturnVarType(icode->child[0]->child[1])->id);	
	variableMap[functionname][SELFTEXT]->infer_type->returnvaltype = tmpType;
	//update parameters_type
	vector<VarType *>parameters = variableMap[functionname][SELFTEXT]->infer_type->parameterstype;
	for(int i=0; i<parameters.size(); i++){
		parameters[i]->dataType = variableMap[functionname + "_parameters"][parameters[i]->varid]->type;
	}
}

void analyzeExpWithoutTypeInference(TreeNode* icode) {
    /* look for all occurrences of undefined variables */
    if (icode != NULL) {
	//if (icode -> op == ID && !ST[icode -> id - 'a']) {
	std::string varKey(icode->id);
	if (icode -> op == ID && variableMap[varKey][SELFTEXT] == NULL) {
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
	    analyzeExpWithoutTypeInference(icode -> child[i]);
  }
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
				|| (variableMap[scopename][SELFTEXT]->type == DATATYPE_INT && isBexprSubtree(icode->child[0]) == 0)){
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
				functionCalleeMap[scopename] = icode->child[0];
				break;
			case OUT:
				analyzeExpWithoutTypeInference(icode -> child[0]);
				break;
			default:
				cerr << "statements grammer error: " << icode->op << endl;
				exit(1);
			}
			icode = icode->sibling;
			unknownparameterCounter = 0;
		}
}

std::string infertypeReturnToString(bucket *func, std::string scopename){
	FunctionType *funcType = func->infer_type;
	std::string res = "";
	if(funcType->returnvaltype->dataType == DATATYPE_INT || funcType->returnvaltype->dataType == DATATYPE_BOOL){
		res +=  typeEncoding(funcType->returnvaltype->dataType);
	}
	else if(funcType->returnvaltype->dataType == DATATYPE_FUNC){
		std::string tmpstr(funcType->returnvaltype->varid);
		bucket *tmpfunc = getVariableBucketByIDScope(tmpstr, scopename);
		res += infertypeReturnToString(tmpfunc, scopename);
	}
	else if(funcType->returnvaltype->dataType == DATATYPE_UNKOWN){
		res += funcType->returnvaltype->varid;
	}
	return res;
}


std::string funcTypeToString(bucket *func, std::string scopename){
	FunctionType *funcType = func->infer_type;
	std::string res = "( ";
	if(funcType != NULL){
		for(int i=0; i<funcType->parameterstype.size(); i++){
			if(funcType->parameterstype[i]->dataType == DATATYPE_INT || funcType->parameterstype[i]->dataType == DATATYPE_BOOL){
				res +=  typeEncoding(funcType->parameterstype[i]->dataType);
			}
			else if(funcType->parameterstype[i]->dataType == DATATYPE_FUNC){
				std::string tmpstr(funcType->parameterstype[i]->varid);
				bucket *tmpfunc = getVariableBucketByIDScope(tmpstr, scopename);
				if(tmpfunc!= NULL && tmpfunc->type == ID){
					res += funcTypeToString(tmpfunc, scopename);
				}
				else
					res += funcTypeToString(tmpfunc, scopename);
			}else if(funcType->parameterstype[i]->dataType == DATATYPE_UNKOWN){
				std::string tmpstr(funcType->parameterstype[i]->varid);
				bucket *tmpfunc = getVariableBucketByIDScope(tmpstr, scopename);
				if(tmpfunc!= NULL && tmpfunc->type == DATATYPE_FUNC){
					res += funcTypeToString(tmpfunc, scopename);
				}
				else
					res += funcType->parameterstype[i]->varid;
			}		
			if(i != (funcType->parameterstype.size() -1))
				res += "*";
		}
	
		res += "->";
		if(funcType->returnvaltype->dataType == DATATYPE_INT || funcType->returnvaltype->dataType == DATATYPE_BOOL){
			res +=  typeEncoding(funcType->returnvaltype->dataType);
		}
		else if(funcType->returnvaltype->dataType == DATATYPE_FUNC){
			std::string tmpstr(funcType->returnvaltype->varid);
			bucket *tmpfunc = getVariableBucketByIDScope(tmpstr, scopename);
			if(funcType->returnvaltype->isCall == 1){
				res += infertypeReturnToString(tmpfunc, scopename);
			}else
				res += funcTypeToString(tmpfunc, scopename);
		}else if(funcType->returnvaltype->dataType == DATATYPE_UNKOWN){
			
			std::string tmpstr(funcType->returnvaltype->varid);
			bucket *tmpfunc = getVariableBucketByIDScope(tmpstr, scopename);
			if(tmpfunc!= NULL && tmpfunc->type == DATATYPE_FUNC){
				/*
				if(funcType->returnvaltype->isCall == 1){
					res += infertypeReturnToString(tmpfunc, scopename);
				}
				else*/
					res += funcTypeToString(tmpfunc, scopename);
				
			}
			else
				res += funcType->returnvaltype->varid;
		}
	}
	res += " )";
	return res;
}

void printTypeInference(char *varName){
	std::string varString(varName);
	bucket *tmp = variableMap[varString][SELFTEXT];
	if(tmp != NULL){
		if(tmp->type == DATATYPE_FUNC){
			cout << varName << ":" << funcTypeToString(tmp, varString) << endl;
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
bucket **debug_tracker = NULL;

void analyzeBlock(TreeNode* icode){
	TreeNode *tmpNode = icode->child[0];
	string tmpstr(tmpNode->id);
	debug_tracker = &variableMap[tmpstr][SELFTEXT];
	
	TreeNode* p= icode->child[0];
	while(p != NULL){
		if(p->op != DECNODE){
			cerr << "error in declaration check: " << icode->id << endl;
			exit(1);
		}
		std::string scopename(p->id);
		variableMap[scopename][SELFTEXT] = new bucket;
		variableMap[scopename][SELFTEXT]->type = p->type;
		if(variableMap[scopename][SELFTEXT]->type != DATATYPE_FUNC){
			variableMap[scopename][SELFTEXT]->addr = gloablAddr;
			gloablAddr--;
		}
		else{
			variableMap[scopename][SELFTEXT]->addr = 100;
		}
		//variableMap[var_str][defaultName]->addr = ;
		p = p->sibling;
	}
	analyze_s_stmt(icode->child[1]);
	p = icode->child[0];	
	//reverse the address.
	while( p != NULL){
		std::string scopename(p->id);
		if(p->type != DATATYPE_FUNC){
			variableMap[scopename][SELFTEXT]->addr = variableMap[scopename][SELFTEXT]->addr - gloablAddr + 2;
		}
		p = p->sibling;
	}
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