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
    ExpResult *Calculate() const override {
      if(type == IDENT) {
        RawValueP IdentSrc = signTable.getVarR(ident);
        auto &tag = IdentSrc->value.tag;
        if(tag == RVT_INTEGER) {
          return new ExpResult(IdentSrc->value.integer.value);
        } else if(tag == RVT_FLOAT){
          return new ExpResult(IdentSrc->value.floatNumber.value);
        } else if(tag == RVT_GLOBAL){
          auto &init = IdentSrc->value.global.Init;
          if(tag == RVT_INTEGER) return new ExpResult(init->value.integer.value);
          else if(tag == RVT_FLOAT) return new ExpResult(init->value.floatNumber.value);
          else return new ExpResult(0);
        } else {
          cout << "non-const variable can't be assigned to a const variable" << endl;
          assert(0);
        }
      } else if(type == ARRAY){
        //未实现map[i][j]
        //RawValueP arrayIdent = signTable.getVarR(ident);
        vector<RawValueP> dimens;
        array->generateGraph(dimens);
        int position = 0;
        for(auto * dimen: dimens){
          position+= dimen->value.integer.value;
        }
        if (signTable.IdentTable->ArrayTable.find(ident) != signTable.IdentTable->ArrayTable.end()) {
            cout<<"Cannot find name of array in array table for integer" << endl;
            return new ExpResult(-1);
        }
        auto arrayTableEntry = signTable.IdentTable->ArrayTable.at(ident);
        if (position >= arrayTableEntry->arrValue.elements.size()) {
            cout << "ArrayTable position index out of range for intrger" << endl;
            return new ExpResult(-2);
        }
        RawValue* arrayIdent = arrayTableEntry->arrValue.elements.at(position);
        //int res = arrayIdent->value.integer.value;
        auto &tag = arrayIdent->value.tag;
        if(tag == RVT_FLOAT){
          return new ExpResult(arrayIdent->value.floatNumber.value);
        } else return new ExpResult(arrayIdent->value.integer.value);
      } else return 0;
    }
    
    void generateGraph(string &sign) const override;
};

//做到这里发现没有data这个结构体了
//后续的改的版本在这个分支上意义已经不大了
//所以决定切换到lxh2分支上，这个作为一个暂存的仓库保留

#endif