#include "BaseAST.h"
#ifndef DECLAST_STORMY
#define DECLAST_STORMY
class DeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> ConstDecl;
    std::unique_ptr<BaseAST> VarDecl;
    uint32_t type;
    void generateGraph() const override;
};

class ConstDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> Btype;
    std::unique_ptr<BaseAST> MulConstDef;
    void generateGraph() const override; 
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
};

class SinConstDefAST : public BaseAST{
  public:
    string ident;
    unique_ptr<BaseAST>ConstExp;
    void generateGraph() const override;
};

class VarDeclAST : public BaseAST {
public:
     unique_ptr <BaseAST> MulVarDef;
     void generateGraph() const override;
};

class MulVarDefAST : public BaseAST {
public:
    vector <unique_ptr<BaseAST>> SinValDef;
    void generateGraph() const override;
};

class SinVarDefAST : public BaseAST {
public:
    string ident;
    unique_ptr<BaseAST>InitVal;
    unique_ptr<BaseAST>func_exp;
    uint32_t type; 
    void generateGraph() const override;
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
};
#endif
