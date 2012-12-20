#include <iostream>
#include "globals.h"

int main(void) {
  //TreeNode* icode = statements();
  TreeNode* icode = program();
  if (icode == NULL) {
    cerr << "no top-level expression\n";
    exit(1);
  }
  analyze(icode);
  codeGenProgram(icode);
  printCode();
  return 0;
}