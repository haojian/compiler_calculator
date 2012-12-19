#include <iostream>
#include "globals.h"
#include <vector>
#include <map>
#include <stdio.h>
#include <stdlib.h>

/* Code Generation for IBM */

extern std::vector <string> scope;

static CodeType codeArray[CODESIZE];
static int ICounter = 0;
static int C_OFFSET = 1023;
static int tempAddr = 100;
static int HighCounter = 0;

std::vector < std::vector < int > > breakaddr;

int iblock = 0;

//std::vector <int> exitCodeLocs;			 // would be cleared every time, when in a new top-level -bexpr.
//std::vector <int> opNodeStack;  // would be cleared every time, when in a new top-level -bexpr.
std::vector < std::vector < int > > opNodeStack;
std::vector < std::vector < int > > exitCodeLocs;

// Return the SU number for each node.
int Sethi_Ullman_Counter(TreeNode *icode){
	int n = 0, n_left = 0, n_right = 0;
	if(icode == NULL){
		return 0;
	}
	if((icode->child[0] == NULL) && (icode->child[1] == NULL)){
		if(icode->op == ID){
			n = 1;
		}
		else if((icode->op == NUM) || (icode->op == INPUT)){
			n = 0;
		}
		else {
			//cerr << "Error! Unknown node type!" <<endl;
		}
	}
	else if((icode->op == PLUS) || (icode->op == SUB) ||
			(icode->op == TIMES) || (icode->op == DIV)){
		n_left = Sethi_Ullman_Counter(icode->child[0]);
		n_right = Sethi_Ullman_Counter(icode->child[1]);
		if(n_left == n_right){
			n = n_left + 1;
		}else{
			n = n_left > n_right? n_left: n_right;
		}
	}else if(icode->op == ASSIGN){
		//PLACE HOLDER.
	}else if(	icode->op == OPLESSEQ 
			|| icode->op == OPGREATEREQ
			|| icode->op == OPEQ
			|| icode->op== OPLESS
			|| icode->op == OPGREATER
			|| icode->op == OPNOTEQ
			|| icode->op == OPNOT
			|| icode->op == OPAND){
			//PLACE HOLDER.
	}
	else{
		cerr << "op invalid" << endl;
		exit(1);
	}
	return n;
}


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


#define OPLESSEQ 280
#define OPGREATEREQ 281
#define OPEQ  282
#define OPLESS 283
#define OPGREATER 284
#define OPNOTEQ 285
#define OPAND  286

int emitSkip(int n) {
  int c = ICounter;
  ICounter = ICounter+n;
  if(HighCounter<ICounter)
    HighCounter = ICounter;
  return c;
}

void emitBackup(int addr) {
  if(HighCounter<ICounter)
    HighCounter=ICounter;
  ICounter=addr;
}

void emitRestore() {
  ICounter=HighCounter;
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


void codeGenExp(TreeNode* icode, int reg_base) {

	int n, n_left, n_right, k, j, n_free;
	int ib, is;
	if(reg_base > (MAX_REG-1)){
		cerr << "Out of registers' range!" << endl;
	}
	n_free = MAX_REG - reg_base;
	if(icode != NULL){
		n_left = Sethi_Ullman_Counter(icode->child[0]);
		n_right = Sethi_Ullman_Counter(icode->child[1]);
		//Check if swap is required.
		if(n_left >= n_right){
			ib = 0;
			is = 1;
		}
		else{
			ib = 1;
			is = 0;
		}
		switch(icode -> op){
			case KEYWORD_TRUE:
				emit("LDC", RM, reg_base, 1,0);
				break;
			case KEYWORD_FALSE:
				emit("LDC", RM, reg_base, 0,0);
				break;
			case OPLESSEQ:
	      	case OPGREATEREQ:
	      	case OPEQ:
	      	case OPLESS:
	      	case OPGREATER:
	      	case OPNOTEQ:
	        	if ( n_free > 2 ) {  /* use reg instead of temp */
	          		codeGenExp(icode->child[0],reg_base);
	          		codeGenExp(icode->child[1],reg_base+1);
	          		emit("SUB",RO,reg_base,reg_base,reg_base+1);
	        	} else {            /* use temp */
	          		codeGenExp(icode->child[0],reg_base);
					emit("ST",RM,reg_base,tempAddr++, MAX_REG);
	          		codeGenExp(icode->child[1],reg_base);
					emit("LD",RM,reg_base+1,--tempAddr,MAX_REG);
	          		emit("SUB",RO,reg_base,reg_base+1,reg_base);
	        	}
	        	break;
			case OPAND:
				break;
			case OPNOT:
				codeGenExp(icode->child[0],reg_base);
				emit("LDC", RM, reg_base+1, -1,0);
				emit("MUL",RO,reg_base,reg_base+1,reg_base);
				break;
			case NUM:
				emit("LDC", RM, reg_base, icode->val,0);
				break;
			case ID:
				emit("LD", RM, reg_base,  C_OFFSET - getAddrByid(icode -> id), MAX_REG);
				break;
			case PLUS:
				if(n_free > 2){
					codeGenExp(icode -> child[ib], reg_base);
					codeGenExp(icode -> child[is], reg_base+1);
					emit("ADD",RO,reg_base,reg_base+1,reg_base);
				}
				else{
					codeGenExp(icode -> child[ib], reg_base);
				    emit("ST",RM,reg_base,tempAddr++, MAX_REG);
				    codeGenExp(icode -> child[is], reg_base);
				    emit("LD",RM,reg_base+1,--tempAddr,MAX_REG);
				    emit("ADD",RO,reg_base,reg_base+1,reg_base);
				}
				break;
			case SUB:
				if(n_free > 2){
					codeGenExp(icode -> child[ib], reg_base);
					codeGenExp(icode -> child[is], reg_base+1);
					if(ib == 0) {
						emit("SUB",RO,reg_base,reg_base,reg_base+1);
					}else{
						emit("SUB",RO,reg_base,reg_base+1,reg_base);
					}
				}
				else{
					codeGenExp(icode -> child[ib], reg_base);
				    emit("ST",RM,reg_base,tempAddr++, MAX_REG);
				    codeGenExp(icode -> child[is], reg_base);
				    emit("LD",RM,reg_base+1,--tempAddr,MAX_REG);
					if(ib == 1){
						emit("SUB", RO, reg_base, reg_base, reg_base+1);
					}else{
						emit("SUB", RO, reg_base, reg_base+1, reg_base);
					}
				}
				break;
			case TIMES:
				if(n_free > 2){
					codeGenExp(icode -> child[ib], reg_base);
					codeGenExp(icode -> child[is], reg_base+1);
					emit("MUL",RO,reg_base,reg_base+1,reg_base);
				}
				else{
					codeGenExp(icode -> child[ib], reg_base);
				    emit("ST",RM,reg_base,tempAddr++, MAX_REG);
				    codeGenExp(icode -> child[is], reg_base);
				    emit("LD",RM,reg_base+1,--tempAddr,MAX_REG);
				    emit("MUL",RO,reg_base,reg_base+1,reg_base);
				}
				break;
			case DIV:
				if(n_free > 2){
					codeGenExp(icode -> child[ib], reg_base);
					codeGenExp(icode -> child[is], reg_base+1);
					if(ib == 0) {
						emit("DIV",RO,reg_base,reg_base,reg_base+1);
					}else{
						emit("DIV",RO,reg_base,reg_base+1,reg_base);
					}
				}
				else{
					codeGenExp(icode -> child[ib], reg_base);
				    emit("ST",RM,reg_base,tempAddr++, MAX_REG);
				    codeGenExp(icode -> child[is], reg_base);
				    emit("LD",RM,reg_base+1,--tempAddr,MAX_REG);
					if(ib == 1){
						emit("DIV", RO, reg_base, reg_base, reg_base+1);
					}else{
						emit("DIV", RO, reg_base, reg_base+1, reg_base);
					}
				}
				break;
			case INPUT:
		      emit("IN",RO, reg_base, reg_base, reg_base);
			  //emit("ST",RM, 0, tempAddr, 6);
			  break;
			
			default:
		      cerr << "code generation error: statement level:  "<< icode -> op << endl;
			  exit(1);
		}
	}
}

void codeGenBexp(TreeNode* icode, int reg_base) {
	int saveLoc, newExitCode;
	int n, n_left, n_right, k, j, n_free;
	int ib, is;
	if(reg_base > (MAX_REG-1)){
		cerr << "Out of registers' range!" << endl;
	}
	n_free = MAX_REG - reg_base;
	if(icode != NULL){
		n_left = Sethi_Ullman_Counter(icode->child[0]);
		n_right = Sethi_Ullman_Counter(icode->child[1]);
		//Check if swap is required.
		if(n_left >= n_right){
			ib = 0;
			is = 1;
		}
		else{
			ib = 1;
			is = 0;
		}
		switch(icode -> op){
			case KEYWORD_TRUE:
				emit("LDC", RM, reg_base, 1,0);
				opNodeStack.back().push_back(icode->op);
				exitCodeLocs.back().push_back(ICounter);
				ICounter++;
				//cmpjmpwithOP(icode -> op, reg_base, ICounter);
				break;
			case KEYWORD_FALSE:
				emit("LDC", RM, reg_base, -1,0);
				opNodeStack.back().push_back(icode->op);
				exitCodeLocs.back().push_back(ICounter);
				ICounter++;
				//cmpjmpwithOP(icode -> op, reg_base, ICounter+2);
				break;
			case OPLESSEQ:
	      	case OPGREATEREQ:
	      	case OPEQ:
	      	case OPLESS:
	      	case OPGREATER:
	      	case OPNOTEQ:
	        	if ( n_free > 2 ) {  /* use reg instead of temp */
	          		codeGenExp(icode->child[0],reg_base);
	          		codeGenExp(icode->child[1],reg_base+1);
	          		emit("SUB",RO,reg_base,reg_base,reg_base+1);
	        	} else {            /* use temp */
	          		codeGenExp(icode->child[0],reg_base);
					emit("ST",RM,reg_base,tempAddr++, MAX_REG);
	          		codeGenExp(icode->child[1],reg_base);
					emit("LD",RM,reg_base+1,--tempAddr,MAX_REG);
	          		emit("SUB",RO,reg_base,reg_base+1,reg_base);
	        	}
				opNodeStack.back().push_back(icode->op);
				exitCodeLocs.back().push_back(ICounter);
				ICounter++;
	        	break;
			case OPAND:
				codeGenBexp(icode->child[0],reg_base);
				codeGenBexp(icode->child[1],reg_base);
				break;
			case OPNOT:
				codeGenBexp(icode->child[0],reg_base);
				emit("LDC", RM, reg_base, 1,0);		//load 1 for true
				saveLoc = ICounter;
				emit("LDC",RM,7,saveLoc+2,MAX_REG);
				//cmpjmpwithOP(KEYWORD_TRUE, reg_base, saveLoc+2);
				saveLoc = ICounter;					//all the false in past jump to here.
				emit("LDC", RM, reg_base, -1,0);	//for false
				emit("LDC", RM, reg_base+1, -1,0);	//load a factor.
				emit("MUL",RO,reg_base,reg_base+1,reg_base);
				newExitCode = ICounter;
				while(!exitCodeLocs.back().empty()){
					int exitCodeIndex = exitCodeLocs.back().back();
					exitCodeLocs.back().pop_back();
					int op = opNodeStack.back().back();
					opNodeStack.back().pop_back();
					ICounter = exitCodeIndex;
					cmpjmpwithOP(op, reg_base, saveLoc);
				}
				opNodeStack.back().push_back(KEYWORD_TRUE);
				exitCodeLocs.back().push_back(newExitCode);
				ICounter = newExitCode+1;
				break;
			case ID:
				if(getTypeByid(icode->id) == DATATYPE_BOOL){
					emit("LD", RM, reg_base,  C_OFFSET - getAddrByid(icode -> id), MAX_REG);
					opNodeStack.back().push_back(KEYWORD_TRUE);
					exitCodeLocs.back().push_back(ICounter);
					ICounter++;
					/*
					if(icode->val == 1){
						emit("LDC", RM, reg_base, 1,0);
						opNodeStack.back().push_back(icode->op);
						exitCodeLocs.back().push_back(ICounter);
						ICounter++;
					}else if(icode->val == -1){
						emit("LDC", RM, reg_base, -1,0);
						opNodeStack.back().push_back(icode->op);
						exitCodeLocs.back().push_back(ICounter);
						ICounter++;
					}
					*/
				}else{
					cerr << "ID in bexpr should be boolean."<<endl;
				}
				break;
			default:
		      cerr << "bexpr code generation error: statement level:  "<< icode -> op << endl;
			  exit(1);
		}
	}
}

void codeGenStmt(TreeNode* icode, int base) {
	int saveLoc, saveLoc2, saveLoc3;
	std::vector <int> tmpv;
	std::vector <int> tmpCode;
	std::vector <int> tmpOp;
  	std::string id_scope;
  
	TreeNode *tmpNode = NULL;
  /* register 0 for ac */
  /* register 1 for ac1 */
  while (icode != NULL) {
    switch (icode -> op) {
    case ASSIGN:
		if(icode->type == DATATYPE_INT){
			codeGenExp(icode -> child[0], base);
		    emit("ST",RM, base, C_OFFSET - getAddrByid(icode -> id), MAX_REG);
		}else if(icode->type == DATATYPE_BOOL){
			tmpOp.clear();
			tmpCode.clear();
	        opNodeStack.push_back(tmpOp);
			exitCodeLocs.push_back(tmpCode);
			codeGenBexp(icode -> child[0], base);
			emit("LDC", RM, base, 1,0);		//load 1 for true
			saveLoc = ICounter;
			emit("LDC",RM,7,saveLoc+2,MAX_REG);
			saveLoc = ICounter;				// tmp jump target.
			emit("LDC", RM, base, -1,0);	//for false
			saveLoc2 = ICounter;
			
			while(!exitCodeLocs.back().empty()){
				int exitCodeIndex = exitCodeLocs.back().back();
				exitCodeLocs.back().pop_back();
				int op = opNodeStack.back().back();
				opNodeStack.back().pop_back();
				ICounter = exitCodeIndex;
				cmpjmpwithOP(op, base, saveLoc);
			}
			ICounter = saveLoc2;
		    emit("ST",RM, base, C_OFFSET - getAddrByid(icode -> id), MAX_REG);
		
			opNodeStack.pop_back();
			exitCodeLocs.pop_back();
		}else{
			cerr << "unkonwn assign type" << endl;
			exit(1);
		}
      
      break;
    case OUT:
      codeGenExp(icode -> child[0], 0);
      emit("OUT",RO,0,0,0);
      break;
	case BLOCKNODE:
	  static std::string blockname;
      blockname = getblockname(iblock++);
      scope.push_back( blockname );
      codeGenStmt(icode -> child[1],base);
      scope.pop_back();
      break;
	case ITERATION:
		//opNodeStack.clear();
		//exitCodeLocs.clear();
		int exitaddr;
		tmpv.clear();
		tmpOp.clear();
		tmpCode.clear();
        opNodeStack.push_back(tmpOp);
		exitCodeLocs.push_back(tmpCode);
		breakaddr.push_back(tmpv);
		saveLoc = ICounter;			// for comparision, interation may need to return this line.
		codeGenBexp(icode->child[0],base);
		codeGenStmt(icode->child[1],base);	// for while content
		saveLoc3 = ICounter;		// for pc redirect.
		exitaddr = ICounter+1;
		while(!exitCodeLocs.back().empty()){
			int exitCodeIndex = exitCodeLocs.back().back();
			exitCodeLocs.back().pop_back();
			int op = opNodeStack.back().back();
			opNodeStack.back().pop_back();
			ICounter = exitCodeIndex;
			cmpjmpwithOP(op, base, exitaddr);
		}
		ICounter = saveLoc3;
		emit("LDC",RM,7,saveLoc,MAX_REG);
		
		if ( breakaddr[breakaddr.size()-1].size() != 0 ) {
          saveLoc = ICounter;
          for (int i=0;i<breakaddr.back().size();i++ ) {
            ICounter = breakaddr.back()[i];
            emit("LDC",RM,7,exitaddr,MAX_REG);
          }
          ICounter = saveLoc;
        }
		breakaddr.pop_back();
		opNodeStack.pop_back();
		exitCodeLocs.pop_back();
		break;
	case IFSELECTION:
		tmpOp.clear();
		tmpCode.clear();
		opNodeStack.push_back(tmpOp);
		exitCodeLocs.push_back(tmpCode);
		//exitCodeLocs.clear();
		//opNodeStack.clear();
		codeGenBexp(icode->child[0], base);
		codeGenStmt(icode->child[1],base);
		saveLoc = ICounter;  // save for PC redirect
		ICounter++;
		saveLoc2 = ICounter;	//save for elsestart;
		codeGenStmt(icode->child[2],base);
		saveLoc3 = ICounter; 	//save for command after ifs.
		
		while(!exitCodeLocs.back().empty()){
			int exitCodeIndex = exitCodeLocs.back().back();
			exitCodeLocs.back().pop_back();
			int op = opNodeStack.back().back();
			opNodeStack.back().pop_back();
			ICounter = exitCodeIndex;
			cmpjmpwithOP(op, base, saveLoc2);
		}
		ICounter = saveLoc;
		emit("LDC",RM,7,saveLoc3,MAX_REG);
		ICounter = saveLoc3;
		
		opNodeStack.pop_back();
		exitCodeLocs.pop_back();
        break;
    case KEYWORD_BREAK:
        //cout << "BREAK address " << ICounter << endl;
        breakaddr[breakaddr.size()-1].push_back( ICounter++ );
        break;
    default:
      cerr << "code generation error: statements level" << endl;
    }
    icode = icode -> sibling;
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
