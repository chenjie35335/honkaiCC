#include "BaseAST.h"
#ifndef DECLAST_STORMY
#define DECLAST_STORMY
class DeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> ConstDecl;
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
    string type;
};
//这里使用mulConstDef递归嵌套的方式实现多个，而且使用vector存储可以提高树的平衡性
class MulConstDefAST : public BaseAST {
  public:
    vector <unique_ptr<BaseAST>> SinConstDef;
    void generateGraph() const override{
        for(auto &sinConstDef : SinConstDef) {
          sinConstDef->generateGraph();
        }
    }
    void generateGraphGlobal() const override;
};

class SinConstDefAST : public BaseAST{
  public:
  enum {
    SINCONST_VAR,
    SINCONST_ARRAY
  }kind;
    string ident;
    unique_ptr<BaseAST>arrayDimen;
    unique_ptr<BaseAST>constExp;
    unique_ptr<BaseAST>constArrayInit;
    int type;
    void generateGraph() const override;
    void generateGraphGlobal() const override;
};

class VarDeclAST : public BaseAST {
public:
     unique_ptr <BaseAST> MulVarDef;
     void generateGraph() const override;
     void generateGraphGlobal() const override;
};

class MulVarDefAST : public BaseAST {
public:
    vector <unique_ptr<BaseAST>> SinValDef;
    void generateGraph() const override;
    void generateGraphGlobal() const override;
};

class SinVarDefAST : public BaseAST {
public:
    string ident;
    unique_ptr<BaseAST>InitVal;
    unique_ptr<BaseAST>dimen;
    unique_ptr<BaseAST>constInit;
    int number;
    uint32_t type; 
    void generateGraph() const override;
    void generateGraphGlobal() const override;
};

class InitValAST : public BaseAST {
public:
    unique_ptr<BaseAST>Exp;
    [[nodiscard]] int calc() const override {return Exp->calc();}
    void generateGraph(string &sign) const override;
};

class ConstExpAST : public BaseAST {
  public:
    unique_ptr<BaseAST>Exp;
    [[nodiscard]] int calc() const override{
        return Exp->calc();
    }
    void generateGraph(string &name) const override { Exp->generateGraph(name);}
};

#endif
