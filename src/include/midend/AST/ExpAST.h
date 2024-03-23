#include "BaseAST.h"
#ifndef EXPAST_STORMY
#define EXPAST_STORMY
class ExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> LOrExp;
  void Dump(string &sign) const override
  {
    LOrExp->Dump(sign);
  }
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
  void Dump(string &sign) const override
  {
    switch (type)
    {
    case SINEXPAST_EXP:
      Exp->Dump(sign);
      break;
    case SINEXPAST_NULL:
      sign = "";
      break;
    default:
      assert(0);
    }
  }
  void generateGraph(string &sign) const override;
};

class LOrExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> LAndExp;
  std::unique_ptr<BaseAST> LOrExp;
  uint32_t type;
  void Dump(string &sign) const override
  {
    string sign1;
    string sign2;
    switch (type)
    {
    case LOREXPAST_LAN:
      LAndExp->Dump(sign);
      break;
    case LOREXPAST_LOR:
    {
      string MidValue = "@a" + to_string(ScNum++)+"_";
      cout << "  " << MidValue << " = alloc i32" << endl;
      cout << "  store " << 0 << ", " << MidValue << endl;
      LOrExp->Dump(sign1);
      string TrueBranch = "%L" + to_string(shortcircuit_cnt++);
      string FalseBranch = "%L" + to_string(shortcircuit_cnt++);
      string NextBranch = "%L" + to_string(shortcircuit_cnt++);
      cout << "  br " << sign1 << ", " << TrueBranch << ", " << FalseBranch << endl
           << endl;
      cout << TrueBranch << ":" << endl;
      cout << "  store " << 1 << ", " << MidValue << endl;
      cout << "  jump " << NextBranch << endl
           << endl;
      cout << FalseBranch << ":" << endl;
      LAndExp->Dump(sign2);
      cout << "  br " << sign2 << ", " << TrueBranch << ", " << NextBranch << endl
           << endl;
      cout << NextBranch << ":" << endl;
      alloc_now++;
      sign = "%" + to_string(alloc_now);
      cout << "  " << sign << " = load " + MidValue << endl;
      break;
    }
    }
  }
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
  void Dump(string &sign) const override
  {
    string sign1, sign2;
    switch (type)
    {
    case LANDEXPAST_EQE:
      // cout << "enter landexp1" << endl;
      EqExp->Dump(sign);
      break;
    case LANDEXPAST_LAN:
    {
      // cout << "enter landexp1" << endl;
      string MidValue = "@a" + to_string(ScNum++)+"_";
      cout << "  " << MidValue << " = alloc i32" << endl;
      cout << "  store " << 1 << ", " << MidValue << endl;
      LAndExp->Dump(sign1);
      string TrueBranch = "%L" + to_string(shortcircuit_cnt++);
      string FalseBranch = "%L" + to_string(shortcircuit_cnt++);
      string NextBranch = "%L" + to_string(shortcircuit_cnt++);
      cout << "  %"+to_string(alloc_now+1) << ' '<< '=' << ' ' <<"eq " <<sign1<< ", " << 0 << endl;
      sign1 = "%"+to_string(++alloc_now);
      cout << "  br " << sign1 << ", " << TrueBranch << ", " << FalseBranch << endl
           << endl;
      cout << TrueBranch << ":" << endl;
      cout << "  store " << 0 << ", " << MidValue << endl;
      cout << "  jump " << NextBranch << endl
           << endl;
      cout << FalseBranch << ":" << endl;
      EqExp->Dump(sign2);
      cout << "  %"+to_string(alloc_now+1) << ' '<< '=' << ' ' <<"eq " <<sign2<< ", " << 0 << endl;
      sign2 = "%"+to_string(++alloc_now);
      cout << "  br " << sign2 << ", " << TrueBranch << ", " << NextBranch << endl
           << endl;
      cout << NextBranch << ":" << endl;
      alloc_now++;
      sign = "%" + to_string(alloc_now);
      cout << "  " << sign << " = load " + MidValue << endl;

      break;
    }
    default:
      assert(0);
    }
  }
  void Dump(string &sign1, string &sign2, string &sign) const override
  {
    alloc_now++;
    cout << "  %" + to_string(alloc_now) << " = "
         << "ne " << sign1 << ", " << 0 << endl;
    sign1 = "%" + to_string(alloc_now);
    alloc_now++;
    cout << "  %" + to_string(alloc_now) << " = "
         << "ne " << sign2 << ", " << 0 << endl;
    sign2 = "%" + to_string(alloc_now);
    alloc_now++;
    cout << "  %" + to_string(alloc_now) << " = "
         << "and " << sign1 << ", " << sign2 << endl;
    sign = "%" + to_string(alloc_now);
  }
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
  void Dump(string &sign) const override
  {
    string s1, s2;
    switch (type)
    {
    case EQEXPAST_REL:
    {
      RelExp->Dump(sign);
      break;
    }
    case EQEXPAST_EQE:
    {
      EqExp->Dump(s1);
      RelExp->Dump(s2);
      EqOp->Dump(s1, s2, sign);
      break;
    }
    default:
      assert(0);
    }
  }
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
  void Dump(string &sign) const override
  {
    string s1, s2;
    switch (type)
    {
    case RELEXPAST_ADD:
    {
      AddExp->Dump(sign);
      break;
    }
    case RELEXPAST_REL:
    {
      RelExp->Dump(s1);
      AddExp->Dump(s2);
      RelOp->Dump(s1, s2, sign);
      break;
    }
    default:
      assert(0);
    }
  }
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
  void Dump(string &sign) const override
  {
    // cout << "enter AddExp" << endl;
    switch (type)
    {
    case MULEXP:
      MulExp->Dump(sign);
      break;
    case ADDMUL:
    {
      string sign1;
      string sign2;
      AddExp->Dump(sign1);
      MulExp->Dump(sign2);
      AddOp->Dump(sign1, sign2, sign);
      break;
    }
    default:
      assert(0);
    }
  }
  void Dump(string &sign1, string &sign2, string &sign) const override {}
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
  void Dump(string &sign) const override
  {
    switch (type)
    {
    case MULEXPAST_MUL:
    {
      string sign1;
      string sign2;
      MulExp->Dump(sign1);
      UnaryExp->Dump(sign2);
      MulOp->Dump(sign1, sign2, sign);
      break;
    }
    case MULEXPAST_UNA:
      UnaryExp->Dump(sign);
      break;
    default:
      assert(0);
    }
  }
  void Dump(string &sign1, string &sign2, string &sign) const override {}
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
  void Dump(string &sign) const override
  {
    PrimaryExp->Dump(sign);
  }
  void Dump(string &sign1, string &sign2, string &sign) const override {}
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
  void Dump(string &sign) const override
  {
    UnaryExp->Dump(sign);
    UnaryOp->Dump(sign);
  }
  void Dump(string &sign1, string &sign2, string &sign) const override {}
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
  // UnaryExp第一种情况
  std::unique_ptr<BaseAST> PrimaryExp;
  int type;
  void Dump(string &sign) const override
  {
    // cout<<"let's begin"<<endl;
    PrimaryExp->Dump(sign);
  }
  void Dump(string &sign1, string &sign2, string &sign) const override {}
  [[nodiscard]] int calc() const override
  {
    return PrimaryExp->calc();
  }
};

class FuncExpAST : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> para;
  void Dump(string &sign) const override
  {
    vector<string> ParaSign;
    para->Dump(sign, ParaSign);
    funcTable.init();
    if (funcTable.FuncTable.at(ident))
    {
      alloc_now++;
      sign = "%" + to_string(alloc_now);
      cout << "\t" << sign << " = ";
    }
    cout << "call @";
    cout << ident;
    cout << "(";
    for (auto it = ParaSign.begin(); it != ParaSign.end(); it++)
    {
      if (it != ParaSign.begin())
        cout << ",";
      cout << (*it);
    }
    cout << ")" << endl;
  }
  void Dump(string &sign1, string &sign2, string &sign) const override {}
  [[nodiscard]] int calc() const override { return 37; }
};

class ParamsAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> sinParams;
  void Dump(string &sign, vector<string> &Para) const override
  {
    for (auto &sinParam : sinParams)
    {
      sinParam->Dump(sign);
      Para.push_back(sign);
    }
  }
};

class SinParamsAST : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;
  void Dump(string &sign) const override
  {
    exp->Dump(sign);
  }
};

class PrimaryExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> Exp;
  std::unique_ptr<BaseAST> Lval;
  int number;
  uint32_t kind;
  /*如果遍历结果为常数，直接返回，如果不是，继续遍历*/
  void Dump(string &sign) const override
  {
    switch (kind)
    {
    case UNARYEXP:
      Exp->Dump(sign);
      break;
    case LVAL:
      Lval->Dump(sign);
      break;
    case NUMBER:
      sign = to_string(number);
      break;
    default:
      assert(0);
    }
  }
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
