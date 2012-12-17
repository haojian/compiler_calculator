#include <iostream>
#include <stdio.h>
#include <ctype.h>
#include "globals.h"

/* Lexical Analyzer for the calculator language */

/* states in the scanner DFA */
typedef enum { 
  START,INNUM,DONE,INCOMMENT,INID
} StateType;

bool LayOutCharacter(char c) {
  return ((c == ' ') || (c == '\t') || (c == '\n'));
}

TokenType getToken(void) {  
   /* holds current token to be returned */
   TokenType currentToken;
   /* current state - always begins at START */
   StateType state = START;
   /* next character */
   char c, c_next;

   do {
     c = cin.get();
   } while (LayOutCharacter(c));

   while (state != DONE) {
     switch (state) {
     		case START:
			   if (LayOutCharacter(c)) {}
		       else if (isdigit(c)){
					currentToken.TokenString += c;
			 		state = INNUM;
			   }
		       else if (isalpha(c)) {
			 		state = INID;
					currentToken.TokenString += c;
			 		currentToken.TokenClass = ID;
		       }
			   else if(c == '%'){
					state = INCOMMENT;
			   }
			   else {
				 		state = DONE;
				 		switch (c)
					           { case EOF:
					               currentToken.TokenClass = ENDFILE;
					               break;
					             case '+':
									currentToken.TokenString += c;
					               currentToken.TokenClass = PLUS;
					               break;
					             case '-':
									currentToken.TokenString += c;
					               currentToken.TokenClass = SUB;
					               break;
								case '*':
									currentToken.TokenString += c;
								   currentToken.TokenClass = TIMES;
								   break;
								case '/':
									currentToken.TokenString += c;
								   currentToken.TokenClass = DIV;
								   break;
					             case '(':
									currentToken.TokenString += c;
					               currentToken.TokenClass = LPAREN;
					               break;
					             case ')':
									currentToken.TokenString += c;
					               currentToken.TokenClass = RPAREN;
					               break;
								 case ',':
									currentToken.TokenString += c;
									currentToken.TokenClass = KEYWORD_COMMA;
									break;
					             case '=':
									c_next = cin.get();
									if(c_next == '='){
										currentToken.TokenClass = OPEQ;
									    currentToken.TokenString = "==";
									}
									else if(c_next == '>'){
										currentToken.TokenClass = OPRETURN;
									    currentToken.TokenString = "=>";
									}
									else{
										cin.putback(c_next);
										currentToken.TokenClass = ASSIGN;
										currentToken.TokenString = "=";
									}
					               break;
								case '<':
									c_next = cin.get();
									if(c_next == '='){
										currentToken.TokenClass = OPLESSEQ;
									    currentToken.TokenString = "<=";
									}
									else{
										cin.putback(c_next);
										currentToken.TokenClass = OPLESS;
										currentToken.TokenString = "<";
									}
					               break;
								case '>':
									c_next = cin.get();
									if(c_next == '='){
										currentToken.TokenClass = OPGREATEREQ;
									    currentToken.TokenString = ">=";
									}
									else{
										cin.putback(c_next);
										currentToken.TokenClass = OPGREATER;
										currentToken.TokenString = ">";
									}
					               break;
								case '!':
									c_next = cin.get();
									if(c_next == '='){
										currentToken.TokenClass = OPNOTEQ;
									    currentToken.TokenString = "!=";
									}
									else{
										cin.putback(c_next);
										currentToken.TokenClass = OPNOT;
										currentToken.TokenString = "!";
									}
					               break;
								case '&':
									c_next = cin.get();
									if(c_next == '&'){
										currentToken.TokenClass = OPAND;
									    currentToken.TokenString = "&&";
									}
									else{
										cin.putback(c_next);
										cout << "unrecognized token" <<endl;
										exit(1);
									}
					               break;
								/*
								case '{':
									currentToken.TokenClass = LBRKT;
									currentToken.TokenString += c;
									break;
								case '}':
									currentToken.TokenClass = RBRKT;
									currentToken.TokenString += c;
									break;
								*/
					             case '$':
									currentToken.TokenString += c;
					               currentToken.TokenClass = OUT;
					               break;
					             case ';':
									currentToken.TokenString += c;
					               currentToken.TokenClass = SEMI;
					               break;
					             default:
									//cerr  << c <<"error2" <<endl;
									currentToken.TokenString += c;
					               currentToken.TokenClass = ERROR;
					               break;
					           }
			   }
			   break;
		       case INNUM:
		         if (!isdigit(c))
		         { /* backup in the input */
		           cin.putback(c);
		           state = DONE;
		           currentToken.TokenClass = NUM;
		         }
				 else{
					currentToken.TokenString += c;
				}
		         break;
				case INID:
					/*Check if the character is legal or not*/
					if(!isdigit(c) && !isalpha(c)){
						cin.putback(c);
						state = DONE;
						currentToken.TokenClass = ID;
						if(currentToken.TokenString.compare("int") == 0){
							currentToken.TokenClass = KEYWORD_INT;
						}
						else if(currentToken.TokenString.compare("if") == 0){
							currentToken.TokenClass = KEYWORD_IF;
						}
						else if(currentToken.TokenString.compare("input") == 0){
							currentToken.TokenClass = INPUT;
						}
						else if(currentToken.TokenString.compare("else") == 0){
							currentToken.TokenClass = KEYWORD_ELSE;
						}
						else if(currentToken.TokenString.compare("while") == 0){
							currentToken.TokenClass = KEYWORD_WHILE;
						}
						else if(currentToken.TokenString.compare("break") == 0){
							currentToken.TokenClass = KEYWORD_BREAK;
						}
						else if(currentToken.TokenString.compare("bool") == 0){
							currentToken.TokenClass = KEYWORD_BOOL;
						}
						else if(currentToken.TokenString.compare("fun") == 0){
							currentToken.TokenClass = KEYWORD_FUNDEF;
						}
						else if(currentToken.TokenString.compare("fn") == 0){
							currentToken.TokenClass = KEYWORD_FN;
						}
						else if(currentToken.TokenString.compare("let") == 0){
							currentToken.TokenClass = KEYWORD_LET;
						}
						else if(currentToken.TokenString.compare("in") == 0){
							currentToken.TokenClass = KEYWORD_IN;
						}
						else if(currentToken.TokenString.compare("end") == 0){
							currentToken.TokenClass = KEYWORD_END;
						}
						else if(currentToken.TokenString.compare("true") == 0){
							currentToken.TokenClass = KEYWORD_TRUE;
							currentToken.TokenString = "1";
						}
						else if(currentToken.TokenString.compare("false") == 0){
							currentToken.TokenClass = KEYWORD_FALSE;
							currentToken.TokenString = "-1";
						}
					}else{
						currentToken.TokenString += c;
					}
					break;
				case INCOMMENT:
					if(c=='\n'){
						state = START;
						//currentToken.TokenString = "";
					}else if(c == EOF){
						currentToken.TokenClass = ENDFILE;
						state = DONE;
					}
					break;
		       case DONE:
		       default: /* should never happen */
		         state = DONE;
		         currentToken.TokenClass = ERROR;
		         break;
     }
     	
     if (state != DONE)
       c = cin.get();
   }
	
   //cerr  << "|"<< currentToken.TokenClass << "\t" << "|" << currentToken.TokenString <<endl;
   return currentToken;
} /* end getToken */

