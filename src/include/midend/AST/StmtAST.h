#include "BaseAST.h"
#ifndef STMTAST_STORMY
#define STMTAST_STORMY
//无论是左值还是右值，都需要在全体作用域中寻找
class StmtAST : public BaseAST {
 public:
    //int num;
    std::unique_ptr<BaseAST> SinExp;
    std::unique_ptr<BaseAST> Exp;
    std::string ident;
    std::unique_ptr<BaseAST> Block;
    std::unique_ptr<BaseAST> ifStmt;
    std::unique_ptr<BaseAST> WhileHead;
    std::unique_ptr<BaseAST> InWhileStmt;
    std::unique_ptr<BaseAST> arrPara;
    uint32_t type;
    void generateGraph() const override;
};

//非终结符不存在类
class IfStmtAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> sinIfStmt;
    std::unique_ptr<BaseAST> multElseStmt;
    int type;
    void generateGraph() const override;
};

//非终结符不存在类
class SinIfStmtAST : public BaseAST{
  public:
     std::unique_ptr<BaseAST> exp;
     std::unique_ptr<BaseAST> stmt;                              
     void generateGraph() const override;
};

class MultElseStmtAST : public BaseAST{
  public:
   std::unique_ptr<BaseAST> exp;
   std::unique_ptr<BaseAST> if_stmt;
   std::unique_ptr<BaseAST> else_stmt;
  void generateGraph() const override; 
};

class WhileStmtHeadAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> WhileHead;
    void generateGraph() const override; 
};

class WhileStmtAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> stmt;
    void generateGraph() const override;
};

//cut stmt after break
class InWhileAST : public BaseAST{
  public:
    int type;
    void generateGraph() const override;
};

#endif
