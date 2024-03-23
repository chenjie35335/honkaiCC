#include "BaseAST.h"
#ifndef LVALAST_STORMY
#define LVALAST_STORMY
class LValLAST : public BaseAST {
  public:
    string ident;
    void Dump(string &sign) const override {
      sign = ident;
    }
};

class LValRAST : public BaseAST {
  public:
    string ident;
    void Dump() const override {}
    //这里想办法在identTable那里定义一个函数用来实现这种遍历
    //这里的遍历实际上很麻烦，主要是那个全局变量的问题
    void Dump(string &sign) const override {
      IdentTable->IdentSearch(ident,sign);
    }
    int calc() const override {
      return IdentTable->IdentCalc(ident);
      }
    void generateGraph(string &sign) const override;
};

#endif