#include "BaseAST.h"
#include "../ValueTable/SignTable.h"
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
    int calc() const override {
      if(type == IDENT) {
        RawValueP IdentSrc = signTable.getVarR(ident);
        auto &tag = IdentSrc->value.tag;
        if(tag == RVT_INTEGER) {
          return IdentSrc->value.integer.value;
        } 
        else if(tag == RVT_GLOBAL){
          return IdentSrc->value.global.Init->value.integer.value;
        } else {
          cout << "non-const variable can't be assigned to a const variable" << endl;
          assert(0);
        }
      } else if(type == ARRAY){
        //未实现map[i][j]
        RawValueP arrayIdent = signTable.getVarR(ident);
        
        return 0;
      } else return 0;
    }
     float fcalc() const override {
      if(type == IDENT) {
        //这里符号表记得也给浮点数升级一下
        RawValueP IdentSrc = signTable.getVarR(ident);
        auto &tag = IdentSrc->value.tag;
        if(tag == RVT_FLOAT) {
          return IdentSrc->value.floatNumber.value;
        } else if(tag == RVT_GLOBAL){
          return IdentSrc->value.global.Init->value.floatNumber.value;
        } else {
          cout << "non-const variable can't be assigned to a const variable" << endl;
          assert(0);
        }
      } else if(type == ARRAY){
        return 0;
      } else return 0;
    }
    void generateGraph(string &sign) const override;
};

//做到这里发现没有data这个结构体了
//后续的改的版本在这个分支上意义已经不大了
//所以决定切换到lxh2分支上，这个作为一个暂存的仓库保留

#endif