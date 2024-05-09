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
          return IdentSrc->value.data.integer.value;
        } else if(RVT_GLOBAL){
          return IdentSrc->value.data.global.Init->value.data.integer.value;
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

#endif