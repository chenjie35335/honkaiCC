#include "BaseAST.h"
#include "../ValueTable/SignTable.h"
#include <new>
#ifndef LVALAST_STORMY
#define LVALAST_STORMY
extern SignTable signTable;
class LValLAST : public BaseAST {
  public:
    string ident;
};

class LValRAST : public BaseAST {
  public:
    enum{
      IDENT,
      ARRAY
    }kind;
    string ident;
    unique_ptr<BaseAST> array;
    int type;
    ExpResult *Calculate() const override {
      if(type == IDENT) {
        RawValueP IdentSrc = signTable.getVarR(ident);
        auto &tag = IdentSrc->value.tag;
        if(tag == RVT_INTEGER) return new ExpResult(IdentSrc->value.integer.value);
        else if(tag == RVT_FLOAT) return new ExpResult(IdentSrc->value.floatNumber.value);
        else if(tag == RVT_GLOBAL) {
          auto &init = IdentSrc->value.global.Init;
          if(tag == RVT_INTEGER) return new ExpResult(init->value.integer.value);
          else if(tag == RVT_FLOAT) return new ExpResult(init->value.floatNumber.value);
          else return new ExpResult(0);
        } else return new ExpResult(0);
      } else return new ExpResult(0);
    }
    void generateGraph(string &sign) const override;
};

//做到这里发现没有data这个结构体了
//后续的改的版本在这个分支上意义已经不大了
//所以决定切换到lxh2分支上，这个作为一个暂存的仓库保留

#endif