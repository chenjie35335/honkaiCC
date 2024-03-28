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
    string ident;
    int calc() const override {
        RawValueP IdentSrc = signTable.getVarR(ident);
        auto &tag = IdentSrc->value.tag;
        if(tag == RVT_INTEGER) {
          return IdentSrc->value.data.integer.value;
        } else {
          cout << "non-const variable can't be assigned to a const variable" << endl;
          assert(0);
        }
    }
    void generateGraph(string &sign) const override;
};

#endif