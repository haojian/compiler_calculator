#include <iostream>
#include "globals.h"
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>

/* Code Generation for IBM */
extern int gloablAddr;
static int ICounter = 0;
static int localAddr = 100;
static CodeType codeArray[CODESIZE];
extern std::map <string, std::map<string, bucket *> > variableMap;



TreeNode *getEqualExpr(TreeNode *icode);
/*tmp code start*/
/*tmp code end*/

void emit(string code, OpCodeType ctype, 
	  int operand1, int operand2, int operand3) {
		if(ICounter> CODESIZE-1){
			cerr << "exceed the codesize limitation(500)"<<endl;
			exit(1);
		}
  codeArray[ICounter].opcode = code;
  codeArray[ICounter].ctype = ctype;
  codeArray[ICounter].rand1 = operand1;
  codeArray[ICounter].rand2 = operand2;
  codeArray[ICounter++].rand3 = operand3;
}

void cmpjmp(TreeNode* p,int base,int Counter) {
  switch(p->op) {
    case OPEQ:
      emit("JNE",RM,base,Counter,MAX_REG); // comparison value in reg b
      break;
    case OPNOTEQ:
      emit("JEQ",RM,base,Counter,MAX_REG);
      break;
    case OPGREATER:
      emit("JLE",RM,base,Counter,MAX_REG);
      break;
    case OPGREATEREQ:
      emit("JLT",RM,base,Counter,MAX_REG);
      break;
    case OPLESSEQ:
      emit("JGT",RM,base,Counter,MAX_REG);
      break;
    case OPLESS:
      emit("JGE",RM,base,Counter,MAX_REG);
      break;
	case KEYWORD_TRUE:
		emit("JEQ",RM,base,Counter,MAX_REG);
		break;
	case KEYWORD_FALSE:
		emit("JEQ",RM,base,Counter,MAX_REG);
		break;
    default:
      cerr << "comparison generation error: " << p->op << endl;
  }
}

void cmpjmpwithOP(int op,int base,int Counter) {
  switch(op) {
    case OPEQ:
      emit("JNE",RM,base,Counter,MAX_REG); // comparison value in reg b
      break;
    case OPNOTEQ:
      emit("JEQ",RM,base,Counter,MAX_REG);
      break;
    case OPGREATER:
      emit("JLE",RM,base,Counter,MAX_REG);
      break;
    case OPGREATEREQ:
      emit("JLT",RM,base,Counter,MAX_REG);
      break;
    case OPLESSEQ:
      emit("JGT",RM,base,Counter,MAX_REG);
      break;
    case OPLESS:
      emit("JGE",RM,base,Counter,MAX_REG);
      break;
	case KEYWORD_TRUE:
	  emit("JLE",RM,base,Counter,MAX_REG);
	  break;
	case KEYWORD_FALSE:
	  emit("JLE",RM,base,Counter,MAX_REG);
	  break;
    default:
      cerr << "comparison generation error: " << op << endl;
  }
}

void codeGenExp(TreeNode* icode, std::string scopename, int neg){
	if(icode == NULL || icode->isCopy == 1) return;
	bucket* tmpBucket = NULL;
  	
	switch(icode->op){
		case ID:{
			if(icode->type != DATATYPE_FUNC){
				std::string tmpstr(icode->id);
				tmpBucket = getVariableBucketByIDScope(tmpstr, scopename);
		    	emit("LD", RM, 0, tmpBucket->addr, 6);
			}
			//deal with function call.
			else{
				if(icode->isTailRecursion != 0){
					 //emit("tailrecursion",RM,0,0,5); 
					int saveLoc = ICounter;
					std::string tmpstr(icode->id);
					TreeNode *parameterPoint = icode->child[0];
					int parameterCounter = 0;
					while(parameterPoint != NULL){
						parameterCounter++;
						codeGenExp(parameterPoint, scopename, 0);
						parameterPoint = parameterPoint->sibling;
						emit("ST",RM,0,0,5);
					  	emit("LDA",RM,5,-1,5); 
					}
					//copy the value to the original parameter position
					//emit("LDA",RM, 2, parameterCounter,5);	//load new parameter initial start position
					emit("SUB",RO,2,6,5);				//get the move distance
					for(int i=0; i<parameterCounter; i++){
						emit("LDA",RM,5,1,5);	//stack back 
						emit("LD",RM,0,0,5);	//load data on statck
						emit("ADD",RO,1,2,5);	//get the responsing position. store it in 1.
						emit("ST",RM,0,0,1);	//store the new parameter to the original position.
					}
					emit("ADD",RO,5,2,5);		//stack recovery 
					for(int i=0; i<parameterCounter; i++){
						emit("LDA",RM,5,-1,5);
					}
					emit("LDA",RM,5,-2,5);		//
					//emit("LDA",RM,5,-1,1);	
					//emit("LDC",RM,0,ICounter+7,0); //return address already been setted. 
				    //emit("ST",RM,0,0,5); //return address already been setted. 
				    //emit("LDA",RM,5,-1,5); //return address already been setted. 
				    // emit("ST",RM,6,0,5); //control link already been setted. 
				    //emit("LDA",RM,5,-1,5); //control link already been setted. 
					//emit("LDA",RM,6,2,5);  //using the same frame point, shouldn't update fp.
				
					/* jump tp function */
					tmpBucket = getVariableBucketByIDScope(tmpstr, scopename);
					emit("LDC",RM,7,tmpBucket->addr,0);
				
					//emit("LDA",RM,5,parameterCounter,6);
					//emit("LD",RM,6,-1,6);
				}
				else{
					std::string tmpstr(icode->id);
					TreeNode *parameterPoint = icode->child[0];
					int parameterCounter = 0;
					while(parameterPoint != NULL){
						parameterCounter++;
						codeGenExp(parameterPoint, scopename, 0);
						parameterPoint = parameterPoint->sibling;
						emit("ST",RM,0,0,5);
					  	emit("LDA",RM,5,-1,5); 
					}
					emit("LDC",RM,0,ICounter+7,0); 
				    emit("ST",RM,0,0,5); 
				    emit("LDA",RM,5,-1,5); 
				    emit("ST",RM,6,0,5); 
				    emit("LDA",RM,5,-1,5); 
					emit("LDA",RM,6,2,5); 
				
					/* jump tp function */
					tmpBucket = getVariableBucketByIDScope(tmpstr, scopename);
					emit("LDC",RM,7,tmpBucket->addr,0); 
				
					emit("LDA",RM,5,parameterCounter,6);
					emit("LD",RM,6,-1,6);
				}
			}
			break;
		}
		case NUM:{
			emit  ("LDC",RM,0,icode->val,0); 
			break;
		}
		case KEYWORD_TRUE:{
			if(neg)
		  		emit("LDC", RM, 0, 0, 0);
			else
		  		emit("LDC", RM, 0, 1, 0);
			break;
		}
		case KEYWORD_FALSE:{
			if(neg)
		  		emit("LDC", RM, 0, 1, 0);
			else
		  		emit("LDC", RM, 0, 0, 0);
			break;
		}
		case INPUT:
			emit("IN",RO,0,0,0);
			break;
		case OPNOT:
			codeGenExp(icode->child[0],scopename, 1-neg);
			break;
		case OPLESSEQ:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("SUB",RO,0,1,0);  
			if (neg)
			  emit("JLE",RM,0,ICounter+3,MAX_REG);
			else
			  emit("JGT",RM,0,ICounter+3,MAX_REG);
			emit("LDC",RM,0,1,0);
			emit("LDC",RM,7,ICounter+2,0); 
			emit("LDC",RM,0,0,0);
			break;
      	case OPGREATEREQ:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("SUB",RO,0,1,0);

			if (neg)
			  emit("JGE",RM,0,ICounter+3,MAX_REG);
			else
			  emit("JLT",RM,0,ICounter+3,MAX_REG);
			emit("LDC",RM,0,1,0);
			emit("LDC",RM,7,ICounter+2,0); 
			emit("LDC",RM,0,0,0);
			break;
      	case OPEQ:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("SUB",RO,0,1,0);

			if (neg)
			  emit("JEQ",RM,0,ICounter+3,MAX_REG);
			else
			  emit("JNE",RM,0,ICounter+3,MAX_REG);
			emit("LDC",RM,0,1,0);
			emit("LDC",RM,7,ICounter+2,0); 
			emit("LDC",RM,0,0,0);
			break;
      	case OPLESS:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("SUB",RO,0,1,0);  
			if (neg)
			  emit("JLT",RM,0,ICounter+3,MAX_REG);
			else
			  emit("JGE",RM,0,ICounter+3,MAX_REG);
			emit("LDC",RM,0,1,0);
			emit("LDC",RM,7,ICounter+2,0); 
			emit("LDC",RM,0,0,0);
			break;
      	case OPGREATER:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("SUB",RO,0,1,0);  
			if (neg)
			  emit("JGT",RM,0,ICounter+3,MAX_REG);
			else
			  emit("JLE",RM,0,ICounter+3,MAX_REG);
			emit("LDC",RM,0,1,0);
			emit("LDC",RM,7,ICounter+2,0);
			emit("LDC",RM,0,0,0);
			break;
      	case OPNOTEQ:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("SUB",RO,0,1,0);

		    if (neg)
			  emit("JNE",RM,0,ICounter+3,MAX_REG);
			else
			  emit("JEQ",RM,0,ICounter+3,MAX_REG);
			emit("LDC",RM,0,1,0);
			emit("LDC",RM,7,ICounter+2,0); 
			emit("LDC",RM,0,0,0);
        	break;
		case LETNODE:
			{
				string tmpid(icode->child[0]->id);
				tmpBucket = getVariableBucketByIDScope(tmpid, scopename);
				if(tmpBucket->type == DATATYPE_FUNC){
					int saveLoc0 = ICounter;
			  		emit("LDC",RM,7,-1,0);
			  		tmpBucket->addr = ICounter;
			  		codeGenExp(icode->child[0]->child[0], scopename, neg);
			  		codeArray[saveLoc0].rand2 = ICounter;
				}
				else{
					codeGenExp(icode->child[0]->child[0], scopename, neg); 
		      		emit("ST", RM, 0, tmpBucket->addr, 6);
			  		emit("LDA",RM,5,-1,5);
				}
				codeGenExp(icode->child[1], scopename, neg);
				if (tmpBucket->type!=DATATYPE_FUNC)
					emit("LDA",RM,5,1,5);
				break;
			}
		case IFSELECTION:{
			int saveLoc1, saveLoc2;
			codeGenExp(icode->child[0], scopename, 0);
			saveLoc1 = ICounter;
			emit("JEQ",RM,0,-1,MAX_REG);
			codeGenExp(icode->child[1], scopename, neg);
			saveLoc2 = ICounter;
			emit("LDC",RM,7,-1,0);
			codeArray[saveLoc1].rand2 = ICounter;
			codeGenExp(icode->child[2], scopename, neg);
			codeArray[saveLoc2].rand2 = ICounter;
			break;
		}
		case DATATYPE_FUNC:
			codeGenExp(icode->child[1], scopename, neg);
			emit("LD",RM,7,0,6);
			break;
		case FUNCNODE:
			codeGenExp(icode->child[1], scopename, neg);
		    emit("LD",RM,7,0,6);
			break;
		case PLUS:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("ADD",RO,0,1,0);
			break;
		case SUB:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("SUB",RO,0,1,0);
			break;
		case TIMES:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("MUL",RO,0,1,0);
			break;
		case DIV:
			codeGenExp(icode -> child[0], scopename, neg);
			emit("ST", RM, 0, 0, 4);
			emit("LDA", RM, 4, 1, 4);
			codeGenExp(icode -> child[1], scopename, neg);
			emit("LDA", RM, 4, -1, 4);
		    emit("LD", RM, 1, 0, 4);
			emit("DIV",RO,0,1,0);
			break;
		default:
			cerr<< "unknown node type! " << endl;
			break;
	}
}

void codeGenStmt(TreeNode* icode) {
	if (icode==NULL || icode->isCopy == 1) return;
  	bucket* tmpBucket = NULL;
	
	switch(icode->op){
		case ASSIGN:{
			std::string scopename(icode->id);
			tmpBucket = variableMap[scopename][SELFTEXT];
			if(tmpBucket->type != DATATYPE_FUNC){
				codeGenExp(icode -> child[0], "global", 0);
				emit("ST",RM,0,tmpBucket->addr,6);
			}
			else{
				if(icode->child[0]->op == ID){
					bucket* tmpBucket1 = variableMap[scopename][SELFTEXT];
					tmpBucket->addr = tmpBucket1->addr;
				}
				else if (icode->child[0]->op == FUNCNODE) {
				  	int saveLoc = ICounter;
				  	emit("LDC",RM,7,-1,0);
				  	tmpBucket->addr = ICounter;
				  	codeGenExp(icode->child[0], scopename, 0);
				  	codeArray[saveLoc].rand2 = ICounter;
				}
			}
			break;
		}
		case OUT:{
			std::string scopename = "global";
			codeGenExp(icode->child[0], scopename,0);
      		emit("OUT",RO,0,0,0);
			break;
		}
		default:
			cerr << "code op error" << endl;
		  	exit(1);
		  	break;
	}
}

void codeGenProgram(TreeNode* icode){
	//register 5 for stack pointer, 6 for frame-point (local variable address), 7 for  
	emit("LD",RM,6,0,0); 		//the intial address of dem[0] = 1023, load this address to frame pointer. fp(6) = 1023
  	emit("LDA",RM, 5, gloablAddr,6); 	//reserver global addresses, and set the stack to the position after globla variables. sp(5) = 1023 - #globalvariable = 1023 - gloabaladdr +2
  	emit("LDA",RM,6,2,5); 		//
  	emit("LDC", RM, 4, localAddr, 0);
	if(icode != NULL){
		icode = icode->child[1];
		while(icode != NULL){
			codeGenStmt(icode);
			icode = icode->sibling;
		}
	}
}

void printCode(void) {
  for (int i = 0; i < ICounter; i++) {
    cout << i << ": " 
	 << codeArray[i].opcode << " "
	 << codeArray[i].rand1 << ","
	 << codeArray[i].rand2 
	 << (codeArray[i].ctype == RO? "," : "(")
	 << codeArray[i].rand3;
    if (codeArray[i].ctype == RM) cout << ")";
    cout << endl;
  }
}
