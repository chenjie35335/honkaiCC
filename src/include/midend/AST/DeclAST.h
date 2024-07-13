#include "BaseAST.h"
#ifndef DECLAST_STORMY
#define DECLAST_STORMY
class DeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> ConstDecl;
    std::unique_ptr<BaseAST> btype;
    std::unique_ptr<BaseAST> VarDecl;
    std::unique_ptr<BaseAST> arrDef;
    uint32_t type;
    void generateGraph() const override;
};

class ConstDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> Btype;
    std::unique_ptr<BaseAST> MulConstDef;
    void generateGraph() const override; 
    void generateGraphGlobal() const override;
};

class BtypeAST : public BaseAST {
  public:
    int32_t type;
    int calc() const override{
      if(type == BTYPE_FLOAT) return RTT_FLOAT;
      else return RTT_INT32;
    }
};
//这里使用mulConstDef递归嵌套的方式实现多个，而且使用vector存储可以提高树的平衡性
class MulConstDefAST : public BaseAST {
  public:
    vector <unique_ptr<BaseAST>> SinConstDef;
    void generateGraph(int &retType) const override{
        for(auto &sinConstDef : SinConstDef) {
          sinConstDef->generateGraph(retType);
        }
    }
    void generateGraphGlobal(int &retType) const override;
};

class SinConstDefAST : public BaseAST{
  public:
  enum {
    SINCONST_VAR,
    FSINCONST_VAR,
    SINCONST_ARRAY,
    FSINCONST_ARRAY
  }kind;
    string ident;
    unique_ptr<BaseAST>arrayDimen;
    unique_ptr<BaseAST>constExp;
    unique_ptr<BaseAST>constArrayInit;
    int type;
    void generateGraph(int &retType) const override;
    void generateGraphGlobal(int &retType) const override;
};

class VarDeclAST : public BaseAST {
public:
     unique_ptr <BaseAST> MulVarDef;
     void generateGraph(int &retType) const override;
     void generateGraphGlobal(int &retType) const override;
};

class MulVarDefAST : public BaseAST {
public:
    vector <unique_ptr<BaseAST>> SinValDef;
    void generateGraph(int &retType) const override;
    void generateGraphGlobal(int &retType) const override;
};

class SinVarDefAST : public BaseAST {
public:
    string ident;
    unique_ptr<BaseAST>InitVal;
    unique_ptr<BaseAST>dimen;
    unique_ptr<BaseAST>constInit;
    int number;
    float fnumber;
    uint32_t type; 
    void generateGraph(int &retType) const override;
    void generateGraphGlobal(int &retType) const override;
};

class InitValAST : public BaseAST {
public:
    unique_ptr<BaseAST>Exp;
    [[nodiscard]] int calc() const override {return Exp->calc();}
    [[nodiscard]] float fcalc() const override {return Exp->fcalc();}
    void generateGraph(string &sign) const override;
};

class ConstExpAST : public BaseAST {
  public:
    unique_ptr<BaseAST>Exp;
    [[nodiscard]] int calc() const override{
        return Exp->calc();
    }
    [[nodiscard]] float fcalc() const override{
        return Exp->fcalc();
    }
    void generateGraph(string &name) const override { Exp->generateGraph(name);}
};

#endif
