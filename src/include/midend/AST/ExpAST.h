#include "BaseAST.h"
#ifndef EXPAST_STORMY
#define EXPAST_STORMY
class ExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> LOrExp;
  ExpResult *Calculate() const override {
    return LOrExp->Calculate();
  }
  void generateGraph(string &sign) const override;
};

class SinExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> Exp;
  uint32_t type;
  ExpResult *Calculate() const override {
    return Exp->Calculate();
  }
  void generateGraph(string &sign,int &type) const override;
};

class LOrExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> LAndExp;
  mutable std::unique_ptr<BaseAST> LOrExp;
  uint32_t type;
  ExpResult *Calculate() const override {
    switch(type) {
    case LOREXPAST_LOR:{
    auto value1 = LOrExp->Calculate();
    auto value2 = LAndExp->Calculate();
    bool OpFloat = value1->type == ExpResult::FLOAT || value2->type == ExpResult::FLOAT;
          if(OpFloat) {
              value1->Convert();
              value2->Convert();
              return new ExpResult(value1->FloatResult || value2->FloatResult);
          } else return new ExpResult(value1->IntResult || value2->IntResult);
    }
    case LOREXPAST_LAN:   
      return LAndExp->Calculate();
    }
  }
  void generateGraph(string &sign) const override;
};

class LAndExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> EqExp;
  mutable std::unique_ptr<BaseAST> LAndExp;
  uint32_t type;
  ExpResult *Calculate() const override{
    switch(type) {
    case LANDEXPAST_LAN:{
        auto value1 = EqExp->Calculate();
        auto value2 = LAndExp->Calculate();
        bool OpFloat = value1->type == ExpResult::FLOAT || value2->type == ExpResult::FLOAT;
              if(OpFloat) {
                  value1->Convert();
                  value2->Convert();
                  return new ExpResult(value1->FloatResult && value2->FloatResult);
              } else return new ExpResult(value1->IntResult && value2->IntResult);
    }
    case LANDEXPAST_EQE: 
        return EqExp->Calculate();
    }
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
  ExpResult *Calculate() const override {
    switch(type) {
        case EQEXPAST_EQE:{
          auto value1 = EqExp->Calculate();
          auto value2 = RelExp->Calculate();
          int OpMul = EqOp->getType();
          bool OpFloat = value1->type == ExpResult::FLOAT || value2->type == ExpResult::FLOAT;
          if(OpFloat) {
              value1->Convert();
              value2->Convert();
          }
          switch (OpMul) {
            case EQOPAST_EQ:
              if(OpFloat) {
                return new ExpResult(value1->FloatResult == value2->FloatResult);
              } else return new ExpResult(value1->IntResult == value2->IntResult);
              break;
            case EQOPAST_NE:
              if(OpFloat) {
                return new ExpResult(value1->FloatResult != value2->FloatResult);
              } else return new ExpResult(value1->IntResult != value2->IntResult);
              break;
            default:
              assert(0);
          }
        }
        case EQEXPAST_REL:{
          return RelExp->Calculate();
        }
        default:assert(0);
      }
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
   ExpResult *Calculate() const override{
    switch(type) {
        case RELEXPAST_REL:{
          auto value1 = RelExp->Calculate();
          auto value2 = AddExp->Calculate();
          int OpMul = RelOp->getType();
          bool OpFloat = value1->type == ExpResult::FLOAT || value2->type == ExpResult::FLOAT;
          if(OpFloat) {
              value1->Convert();
              value2->Convert();
          }
          switch (OpMul) {
            case RELOPAST_GE:
              if(OpFloat) {
                return new ExpResult(value1->FloatResult >= value2->FloatResult);
              } else return new ExpResult(value1->IntResult >= value2->IntResult);
              break;
            case RELOPAST_LE:
              if(OpFloat) {
                return new ExpResult(value1->FloatResult <= value2->FloatResult);
              } else return new ExpResult(value1->IntResult <= value2->IntResult);
              break;
            case RELOPAST_G: 
              if(OpFloat) {
                return new ExpResult(value1->FloatResult > value2->FloatResult);
              } else return new ExpResult(value1->IntResult > value2->IntResult);
              break;
            case RELOPAST_L:
              if(OpFloat) {
                return new ExpResult(value1->FloatResult < value2->FloatResult);
              } else return new ExpResult(value1->IntResult < value2->IntResult);
            default:
              assert(0);
          }
        }
        case RELEXPAST_ADD:{
          return AddExp->Calculate();
        }
        default:assert(0);
      }
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
  ExpResult *Calculate() const override{
    switch(type) {
        case ADDMUL:{
          auto value1 = AddExp->Calculate();
          auto value2 = MulExp->Calculate();
          int OpMul = AddOp->getType();
          bool OpFloat = value1->type == ExpResult::FLOAT || value2->type == ExpResult::FLOAT;
          if(OpFloat) {
              value1->Convert();
              value2->Convert();
          }
          switch (OpMul) {
            case '+':
              if(OpFloat) {
                return new ExpResult(value1->FloatResult + value2->FloatResult);
              } else return new ExpResult(value1->IntResult + value2->IntResult);
              break;
            case '-':
              if(OpFloat) {
                return new ExpResult(value1->FloatResult - value2->FloatResult);
              } else return new ExpResult(value1->IntResult - value2->IntResult);
              break;
            default:
              assert(0);
          }
        }
        case MULEXP:{
          return MulExp->Calculate();
        }
        default:assert(0);
      }
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
  ExpResult *Calculate() const override {
      switch(type) {
        case MULEXPAST_MUL:{
          auto value1 = MulExp->Calculate();
          auto value2 = UnaryExp->Calculate();
          int OpMul = MulOp->getType();
          bool OpFloat = value1->type == ExpResult::FLOAT || value2->type == ExpResult::FLOAT;
          if(OpFloat) {
              value1->Convert();
              value2->Convert();
          }
          switch (OpMul) {
            case '*':
              if(OpFloat) {
                return new ExpResult(value1->FloatResult * value2->FloatResult);
              } else return new ExpResult(value1->IntResult * value2->IntResult);
              break;
            case '/':
              if(OpFloat) {
                return new ExpResult(value1->FloatResult / value2->FloatResult);
              } else return new ExpResult(value1->IntResult / value2->IntResult);
              break;
            case '%':
              if(OpFloat)  assert(0);
              else return new ExpResult(value1->IntResult % value2->IntResult);
              break;
            default:
              assert(0);
          }
        }
        case MULEXPAST_UNA:{
          return UnaryExp->Calculate();
        }
        default:assert(0);
      }
  }
  void generateGraph(string &sign) const override;
};

class UnaryExpAST_P : public BaseAST
{
public:
  // UnaryExp第一种情况
  std::unique_ptr<BaseAST> PrimaryExp;
  int type;
  ExpResult *Calculate() const override { return PrimaryExp->Calculate();}
  [[nodiscard]] int UnaryExpType() const override { return 1; }
  void generateGraph(string &sign) const override;
};

class UnaryExpAST_U : public BaseAST
{
public:
  // UnaryExp的递归第二种情况
  std::unique_ptr<BaseAST> UnaryOp;
  std::unique_ptr<BaseAST> UnaryExp;
  ExpResult *Calculate() const override{
     auto value1 = UnaryExp->Calculate();
     int OpUnary = UnaryOp->getType();
     switch(OpUnary) {
        case '+': 
            return value1;
        case '-': 
            return value1->neg();
        case '!': {
           if(value1->type == ExpResult::INT)
               return new ExpResult(-value1->IntResult);
            else
               return new ExpResult(-value1->FloatResult);
        }
        default : break;
     }
  }
  [[nodiscard]] int UnaryExpType() const override { return 2;}
  void generateGraph(string &sign) const override;
};

class UnaryExpAST_F : public BaseAST
{
public:
  std::unique_ptr<BaseAST> FuncExp;
  ExpResult *Calculate() const override{
    return new ExpResult(0);
  }
  [[nodiscard]] int UnaryExpType() const override { return 3;}
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
  unique_ptr<BaseAST> dimension;
  int type;
  string ident;
  void generateGraph(RawValueP &params) const override;
};

class PrimaryExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> Exp;
  std::unique_ptr<BaseAST> Lval;
  float floatNumber;
  int number;
  uint32_t kind;
  /*如果遍历结果为常数，直接返回，如果不是，继续遍历*/

  ExpResult *Calculate() const override{
    switch (kind) {
      case UNARYEXP: 
        return Exp->Calculate();
      case LVAL:
        return Lval->Calculate();
      case NUMBER:
        return new ExpResult(number);
      case FLOAT_NUMBER:
        return new ExpResult(floatNumber);
      default: assert(0);
    }
  }
  void generateGraph(string &sign) const override;
};

#endif
