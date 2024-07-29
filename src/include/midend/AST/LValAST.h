<<<<<<< HEAD
#include "BaseAST.h"
#include "../ValueTable/SignTable.h"
#ifndef LVALAST_STORMY
#define LVALAST_STORMY
extern SignTable signTable;
class LValLAST : public BaseAST {
  public:
    string ident;
};

// int a = arr[i];
// a[i][j] = {xxx,xxx,xx}; store alloc (global)
// a[i] = {0}   initstore
// a[1][2] = 1; tempstore
// func(a[1][2],3,4);


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
        //RawValueP arrayIdent = signTable.getVarR(ident);
        vector<RawValueP> dimens;
        array->generateGraph(dimens);
        int position = 0;
        for(auto * dimen: dimens){
          position+= dimen->value.integer.value;
        }
        if (signTable.IdentTable->ArrayTable.find(ident) != signTable.IdentTable->ArrayTable.end()) {
            throw std::out_of_range("Cannot find name of array in array table for integer");
        }
        auto arrayTableEntry = signTable.IdentTable->ArrayTable.at(ident);
        if (position >= arrayTableEntry->arrValue.elements.size()) {
            throw std::out_of_range("ArrayTable position index out of range for intrger");
        }
        RawValue* arrayIdent = arrayTableEntry->arrValue.elements.at(position);
        int res = arrayIdent->value.integer.value;
        return res;
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
        vector<RawValueP> dimens;
        array->generateGraph(dimens);
        int position = 0;
        for(auto * dimen: dimens){
          position+= dimen->value.integer.value;
        }
         if (signTable.IdentTable->ArrayTable.find(ident) != signTable.IdentTable->ArrayTable.end()) {
            throw std::out_of_range("Cannot find name of array in array table for float number");
        }
        auto arrayTableEntry = signTable.IdentTable->ArrayTable.at(ident);
        if (position >= arrayTableEntry->arrValue.elements.size()) {
            throw std::out_of_range("ArrayTable position index out of range for float number");
        }
        RawValue* arrayIdent = arrayTableEntry->arrValue.elements.at(position);
        float res = arrayIdent->value.floatNumber.value;
        return res;
      } else return 0;
    }
    void generateGraph(string &sign) const override;
};

//做到这里发现没有data这个结构体了
//后续的改的版本在这个分支上意义已经不大了
//所以决定切换到lxh2分支上，这个作为一个暂存的仓库保留

=======
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

>>>>>>> backold
#endif