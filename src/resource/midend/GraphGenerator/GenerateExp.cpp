#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
unordered_map<string, RawValueP> MidVarTable;
// 这里这个RawProgramme是生成好

void SinExpAST::generateGraph(string &sign) const
{
  switch (type)
  {
  case SINEXPAST_EXP:
    Exp->generateGraph(sign);
    break;
  default:
    assert(0);
  }
}

void ExpAST::generateGraph(string &sign) const
{
  LOrExp->generateGraph(sign);
}

void LOrExpAST::generateGraph(string &sign) const
{
  string sign1, sign2;
  switch (type)
  {
  case LOREXPAST_LAN:
    LAndExp->generateGraph(sign);
    break;
  case LOREXPAST_LOR:
  {
    LOrExp->generateGraph(sign1);
    LAndExp->generateGraph(sign2);
    alloc_now++;
    string OrSign = "%" + to_string(alloc_now);
    RawValueP lhs, rhs;
    generateRawValue(lhs, sign1);
    generateRawValue(rhs, sign2);
    generateRawValue(OrSign, lhs, rhs, RBO_OR);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    string ZeroSign = to_string(0);
    RawValueP zero, StmtOR;
    generateRawValue(0);
    generateRawValue(zero, ZeroSign);
    generateRawValue(StmtOR, OrSign);
    generateRawValue(sign, StmtOR, zero, RBO_NOT_EQ);
    break;
  }
  default:
    assert(0);
  }
}

void LAndExpAST::generateGraph(string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case LANDEXPAST_EQE:
    EqExp->generateGraph(sign);
    break;
  case LANDEXPAST_LAN:
  {
    LAndExp->generateGraph(s1);
    EqExp->generateGraph(s2);
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    RawValueP zero;
    string ZeroSign = to_string(0);
    generateRawValue(0);
    generateRawValue(zero, ZeroSign);
    alloc_now++;
    string Nelsign = "%" + to_string(alloc_now);
    RawValueP StmtNeL, StmtNeR;
    generateRawValue(Nelsign, signL, zero, RBO_NOT_EQ);
    alloc_now++;
    string Nersign = "%" + to_string(alloc_now);
    generateRawValue(Nersign, signR, zero, RBO_NOT_EQ);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    generateRawValue(StmtNeL,Nelsign);
    generateRawValue(StmtNeR,Nersign);
    generateRawValue(sign, StmtNeL, StmtNeR, RBO_AND);
    break;
  }
  default:
    assert(0);
  }
}

void EqExpAST::generateGraph(string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case EQEXPAST_REL:
  {
    RelExp->generateGraph(sign);
    break;
  }
  case EQEXPAST_EQE:
  {
    int OpType = EqOp->calc();
    EqExp->generateGraph(s1);
    RelExp->generateGraph(s2);
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    switch (OpType)
    {
    case EQOPAST_EQ:
      generateRawValue(sign, signL, signR, RBO_EQ);
      break;
    case EQOPAST_NE:
      generateRawValue(sign, signL, signR, RBO_NOT_EQ);
      break;
    default:
      assert(0);
    }
    break;
  }
  default:
    assert(0);
  }
}

void RelExpAST::generateGraph(string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case RELEXPAST_ADD:
  {
    AddExp->generateGraph(sign);
    break;
  }
  case RELEXPAST_REL:
  {
    RelExp->generateGraph(s1);
    AddExp->generateGraph(s2);
    int OpRel = RelOp->calc();
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    switch (OpRel)
    {
    case RELOPAST_GE:
      generateRawValue(sign, signL, signR, RBO_GE);
      break;
    case RELOPAST_LE:
      generateRawValue(sign, signL, signR, RBO_LE);
      break;
    case RELOPAST_L:
      generateRawValue(sign, signL, signR, RBO_LT);
      break;
    case RELOPAST_G:
      generateRawValue(sign, signL, signR, RBO_GT);
      break;
    default:
      assert(0);
    }
    break;
  }
  default:
    assert(0);
  }
}

void AddExpAST::generateGraph(string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case MULEXP:
    MulExp->generateGraph(sign);
    break;
  case ADDMUL:
  {
    AddExp->generateGraph(s1);
    MulExp->generateGraph(s2);
    int OpAdd = AddOp->calc();
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    switch (OpAdd)
    {
    case '+':
      generateRawValue(sign, signL, signR, RBO_ADD);
      break;
    case '-':
      generateRawValue(sign, signL, signR, RBO_SUB);
      break;
    default:
      assert(0);
    }
    break;
  }
  default:
    assert(0);
  }
}

void MulExpAST::generateGraph(string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case MULEXPAST_UNA:
    UnaryExp->generateGraph(sign);
    break;
  case MULEXPAST_MUL:
  {
    MulExp->generateGraph(s1);
    UnaryExp->generateGraph(s2);
    int OpMul = MulOp->calc();
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    switch (OpMul)
    {
    case '*':
      generateRawValue(sign, signL, signR, RBO_MUL);
      break;
    case '/':
      generateRawValue(sign, signL, signR, RBO_DIV);
      break;
    case '%':
      generateRawValue(sign, signL, signR, RBO_MOD);
      break;
    default:
      assert(0);
    }
    break;
  }
  default:
    assert(0);
  }
}

void UnaryExpAST_P::generateGraph(string &sign) const
{
  PrimaryExp->generateGraph(sign);
}

void UnaryExpAST_U::generateGraph(string &sign) const
{
  UnaryExp->generateGraph(sign);
  UnaryOp->generateGraph(sign);
}

void UnaryOpAST::generateGraph(string &sign) const
{

  switch (op)
  {
  case '+':
    break;
  case '-':
  case '!':
  {
    RawValueP exp;
    RawValueP zero;
    string ZeroSign = to_string(0);
    generateRawValue(0);
    generateRawValue(zero, ZeroSign);
    generateRawValue(exp, sign);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    if (op == '-')
      generateRawValue(sign, zero, exp, RBO_SUB);
    else
      generateRawValue(sign, zero, exp, RBO_EQ);
    break;
  }
  default:
    assert(false);
  }
}

void PrimaryExpAST::generateGraph(string &sign) const
{
  switch (kind)
  {
  case UNARYEXP:
    Exp->generateGraph(sign);
    break;
  case NUMBER:
    sign = to_string(number);
    generateRawValue(number);
    break;
  case LVAL:
    Lval->generateGraph(sign);
    break;
  default:
    assert(0);
  }
}
