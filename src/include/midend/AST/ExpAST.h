#include "BaseAST.h"
#ifndef EXPAST_STORMY
#define EXPAST_STORMY
class ExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> LOrExp;
  [[nodiscard]] int calc() const override
  {
    return LOrExp->calc();
  }
  void generateGraph(string &sign) const override;
};

class SinExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> Exp;
  uint32_t type;
  void generateGraph(string &sign) const override;
};

class LOrExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> LAndExp;
  std::unique_ptr<BaseAST> LOrExp;
  uint32_t type;
  [[nodiscard]] int calc() const override
  {
    int value = 0;
    switch (type)
    {
    case LOREXPAST_LAN:
      value = LAndExp->calc();
      break;
    case LOREXPAST_LOR:
    {
      int value1 = LOrExp->calc();
      int value2 = LAndExp->calc();
      value = value1 || value2;
      break;
    }
    }
    return value;
  }
  void generateGraph(string &sign) const override;
};

class LAndExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> EqExp;
  std::unique_ptr<BaseAST> LAndExp;
  uint32_t type;
  [[nodiscard]] int calc() const override
  {
    int value = 0;
    switch (type)
    {
    case LANDEXPAST_EQE:
      value = EqExp->calc();
      break;
    case LANDEXPAST_LAN:
    {
      int value1 = EqExp->calc();
      int value2 = LAndExp->calc();
      value = value1 && value2;
      break;
    }
    }
    return value;
  }
  void generateGraph(string &sign) const override;
};

class EqExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> EqExp;
  std::unique_ptr<BaseAST> RelExp;
  std::unique_ptr<BaseAST> EqOp;
  uint32_t type;
  [[nodiscard]] int calc() const override
  {
    int value = 0;
    switch (type)
    {
    case EQEXPAST_REL:
      value = RelExp->calc();
      break;
    case EQEXPAST_EQE:
    {
      int value1 = EqExp->calc();
      int value2 = RelExp->calc();
      int OpType = EqOp->calc();
      switch (OpType)
      {
      case EQOPAST_EQ:
        value = value1 == value2;
        break;
      case EQOPAST_NE:
        value = value1 != value2;
        break;
      default:
        break;
      }
      break;
    }
    }
    return value;
  }
  void generateGraph(string &sign) const override;
};

class RelExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> AddExp;
  std::unique_ptr<BaseAST> RelExp;
  std::unique_ptr<BaseAST> RelOp;
  uint32_t type;
  [[nodiscard]] int calc() const override
  {
    int value;
    int value1, value2;
    switch (type)
    {
    case RELEXPAST_ADD:
    {
      value = AddExp->calc();
      break;
    }
    case RELEXPAST_REL:
    {
      value1 = RelExp->calc();
      value2 = AddExp->calc();
      int OpRel = RelOp->calc();
      switch (OpRel)
      {
      case RELOPAST_GE:
        value = value1 >= value2;
        break;
      case RELOPAST_LE:
        value = value1 <= value2;
        break;
      case RELOPAST_G:
        value = value1 > value2;
        break;
      case RELOPAST_L:
        value = value1 < value2;
        break;
      default:
        assert(0);
      }
      break;
    }
    default:
      assert(0);
    }
    return value;
  }
  void generateGraph(string &sign) const override;
};

class AddExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> MulExp;
  std::unique_ptr<BaseAST> AddExp;
  uint32_t type;
  std::unique_ptr<BaseAST> AddOp;
  [[nodiscard]] int calc() const override
  {
    int value;
    switch (type)
    {
    case MULEXP:
      value = MulExp->calc();
      break;
    case ADDMUL:
    {
      int value1, value2;
      value1 = AddExp->calc();
      value2 = MulExp->calc();
      int OpAdd = AddOp->calc();
      switch (OpAdd)
      {
      case '+':
        value = value1 + value2;
        break;
      case '-':
        value = value1 - value2;
        break;
      default:
        assert(0);
      }
      break;
    }
    default:
      assert(0);
    }
    return value;
  }
  void generateGraph(string &sign) const override;
};

class MulExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> UnaryExp;
  std::unique_ptr<BaseAST> MulExp;
  uint32_t type;
  std::unique_ptr<BaseAST> MulOp;
  [[nodiscard]] int calc() const override
  {
    int value;
    switch (type)
    {
    case MULEXPAST_MUL:
    {
      int value1, value2;
      value1 = MulExp->calc();
      value2 = UnaryExp->calc();
      int OpMul = MulOp->calc();
      switch (OpMul)
      {
      case '*':
        value = value1 * value2;
        break;
      case '/':
        value = value1 / value2;
        break;
      case '%':
        value = value1 % value2;
        break;
      default:
        assert(0);
      }
      break;
    }
    case MULEXPAST_UNA:
      value = UnaryExp->calc();
      break;
    default:
      assert(0);
    }
    return value;
  }
  void generateGraph(string &sign) const override;
};

class UnaryExpAST_P : public BaseAST
{
public:
  // UnaryExp第一种情况
  std::unique_ptr<BaseAST> PrimaryExp;
  int type;
  [[nodiscard]] int calc() const override
  {
    return PrimaryExp->calc();
  }
  void generateGraph(string &sign) const override;
};

class UnaryExpAST_U : public BaseAST
{
public:
  // UnaryExp的递归第二种情况
  std::unique_ptr<BaseAST> UnaryOp;
  std::unique_ptr<BaseAST> UnaryExp;
  [[nodiscard]] int calc() const override
  {
    int value;
    int OpUnary = UnaryOp->calc();
    int value1 = UnaryExp->calc();
    switch (OpUnary)
    {
    case '+':
      value = value1;
      break;
    case '-':
      value = -value1;
      break;
    case '!':
      value = !value1;
      break;
    default:
      assert(0);
    }
    return value;
  }
  void generateGraph(string &sign) const override;
};

class UnaryExpAST_F : public BaseAST
{
public:
  std::unique_ptr<BaseAST> FuncExp;
  [[nodiscard]] int calc() const override{
    return FuncExp->calc();
  }
  void generateGraph(string &sign) const override;
};

class FuncExpAST : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> para;
  void generateGraph(string &sign) const override;
};

class ParamsAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> sinParams;
  void generateGraph(vector<RawValueP> &params) const override;
};

class SinParamsAST : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;
  void generateGraph(RawValueP &params) const override;
};

class PrimaryExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> Exp;
  std::unique_ptr<BaseAST> Lval;
  int number;
  uint32_t kind;
  /*如果遍历结果为常数，直接返回，如果不是，继续遍历*/
  [[nodiscard]] int calc() const override
  {
    int value;
    switch (kind)
    {
    case UNARYEXP:
      value = Exp->calc();
      break;
    case LVAL:
      value = Lval->calc();
      break;
    case NUMBER:
      value = number;
      break;
    }
    return value;
  }
  void generateGraph(string &sign) const override;
};

#endif
