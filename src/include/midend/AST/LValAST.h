#include "BaseAST.h"
#ifndef LVALAST_STORMY
#define LVALAST_STORMY
class LValLAST : public BaseAST {
  public:
    string ident;
};

class LValRAST : public BaseAST {
  public:
    string ident;
    int calc() const override {
      //return IdentTable->IdentCalc(ident);
      }
    void generateGraph(string &sign) const override;
};

#endif