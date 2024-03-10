#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
int ValueNumber = 0;
unordered_map<string, RawValueP> MidVarTable;
// 这里这个RawProgramme是生成好

void SinExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  switch (type)
  {
  case SINEXPAST_EXP:
    Exp->generateGraph(IR, sign);
    break;
  default:
    assert(0);
  }
}

void ExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  LOrExp->generateGraph(IR, sign);
}

void LOrExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  string sign1, sign2;
  switch (type)
  {
  case LOREXPAST_LAN:
    LAndExp->generateGraph(IR, sign);
    break;
  case LOREXPAST_LOR:
  {
    LOrExp->generateGraph(IR, sign1);
    LAndExp->generateGraph(IR, sign2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    RawValueP lhs, rhs;
    generateRawValue(lhs, sign1);
    generateRawValue(rhs, sign2);
    RawValue *StmtOR;
    generateRawValue(StmtOR, lhs, rhs, RBO_OR);
    MidVarTable.insert(pair<string, RawValueP>(sign, StmtOR));
    IR.buffer[IR.len++] = (const void *)StmtOR;
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    RawValue *zero;
    generateRawValue(zero, 0, IR);
    RawValue *StmtNEQ;
    generateRawValue(StmtNEQ, StmtOR, zero, RBO_NOT_EQ);
    IR.buffer[IR.len++] = (const void *)StmtNEQ;
    MidVarTable.insert(pair<string, RawValueP>(sign, StmtNEQ));
    break;
  }
  default:
    assert(0);
  }
}

void LAndExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case LANDEXPAST_EQE:
    EqExp->generateGraph(IR, sign);
    break;
  case LANDEXPAST_LAN:
  {
    LAndExp->generateGraph(IR, s1);
    EqExp->generateGraph(IR, s2);
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    RawValue *zero;
    generateRawValue(zero, 0, IR);
    RawValue *StmtNeL, *StmtNeR;
    generateRawValue(StmtNeL, signL, zero, RBO_NOT_EQ);
    IR.buffer[IR.len++] = (const void *)StmtNeL;
    MidVarTable.insert(pair<string, RawValueP>(sign, StmtNeL));
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    generateRawValue(StmtNeR, signR, zero, RBO_NOT_EQ);
    IR.buffer[IR.len++] = (const void *)StmtNeR;
    MidVarTable.insert(pair<string, RawValueP>(sign, StmtNeR));
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    RawValue *StmtAnd;
    generateRawValue(StmtAnd, StmtNeL, StmtNeR, RBO_AND);
    IR.buffer[IR.len++] = (const void *)StmtNeR;
    MidVarTable.insert(pair<string, RawValueP>(sign, StmtNeR));
    break;
  }
  default:
    assert(0);
  }
}

void EqExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case EQEXPAST_REL:
  {
    RelExp->generateGraph(IR, sign);
    break;
  }
  case EQEXPAST_EQE:
  {
    int OpType = EqOp->calc();
    EqExp->generateGraph(IR, s1);
    RelExp->generateGraph(IR, s2);
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    RawValue *Stmte;
    switch (OpType)
    {
    case EQOPAST_EQ:
      generateRawValue(Stmte, signL, signR, RBO_EQ);
      break;
    case EQOPAST_NE:
      generateRawValue(Stmte, signL, signR, RBO_NOT_EQ);
      break;
    default:
      assert(0);
    }
    IR.buffer[IR.len++] = (const void *)Stmte;
    MidVarTable.insert(pair<string, RawValueP>(sign, Stmte));
    break;
  }
  default:
    assert(0);
  }
}

void RelExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case RELEXPAST_ADD:
  {
    AddExp->generateGraph(IR, sign);
    break;
  }
  case RELEXPAST_REL:
  {
    RelExp->generateGraph(IR, s1);
    AddExp->generateGraph(IR, s2);
    int OpRel = RelOp->calc();
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    RawValue *StmtR;
    switch (OpRel)
    {
    case RELOPAST_GE:
      generateRawValue(StmtR, signL, signR, RBO_GE);
      break;
    case RELOPAST_LE:
      generateRawValue(StmtR, signL, signR, RBO_LE);
      break;
    case RELOPAST_L:
      generateRawValue(StmtR, signL, signR, RBO_LT);
      break;
    case RELOPAST_G:
      generateRawValue(StmtR, signL, signR, RBO_GT);
      break;
    default:
      assert(0);
    }
    IR.buffer[IR.len++] = (const void *)StmtR;
    MidVarTable.insert(pair<string, RawValueP>(sign, StmtR));
    break;
  }
  default:
    assert(0);
  }
}

void AddExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case MULEXP:
    MulExp->generateGraph(IR, sign);
    break;
  case ADDMUL:
  {
    AddExp->generateGraph(IR, s1);
    MulExp->generateGraph(IR, s2);
    int OpAdd = AddOp->calc();
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    RawValue *StmtAdd;
    switch (OpAdd)
    {
    case '+':
      generateRawValue(StmtAdd, signL, signR, RBO_ADD);
      break;
    case '-':
      generateRawValue(StmtAdd, signL, signR, RBO_SUB);
      break;
    default:
      assert(0);
    }
    IR.buffer[IR.len++] = (const void *)StmtAdd;
    MidVarTable.insert(pair<string, RawValueP>(sign, StmtAdd));
    break;
  }
  default:
    assert(0);
  }
}

void MulExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  string s1, s2;
  switch (type)
  {
  case MULEXPAST_UNA:
    UnaryExp->generateGraph(IR, sign);
    break;
  case MULEXPAST_MUL:
  {
    MulExp->generateGraph(IR,s1);
    UnaryExp->generateGraph(IR,s2);
    int OpMul = MulOp->calc();
    RawValueP signL, signR;
    generateRawValue(signL, s1);
    generateRawValue(signR, s2);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    RawValue *StmtMul;
    switch (OpMul)
    {
    case '*':
      generateRawValue(StmtMul, signL, signR, RBO_MUL);
      break;
    case '/':
      generateRawValue(StmtMul, signL, signR, RBO_DIV);
      break;
    case '%':
      generateRawValue(StmtMul, signL, signR, RBO_MOD);
      break;
    default:
      assert(0);
    }
    IR.buffer[IR.len++] = (const void *)StmtMul;
    MidVarTable.insert(pair<string, RawValueP>(sign, StmtMul));
    break;
  }
  default:
    assert(0);
  }
}

void UnaryExpAST_P::generateGraph(RawSlice &IR, string &sign) const
{
  PrimaryExp->generateGraph(IR, sign);
}

void UnaryExpAST_U::generateGraph(RawSlice &IR, string &sign) const
{
  UnaryExp->generateGraph(IR, sign);
  UnaryOp->generateGraph(IR, sign);
}

void UnaryOpAST::generateGraph(RawSlice &IR, string &sign) const
{
  alloc_now++;
  switch (op)
  {
  case '+':
    break;
  case '-':
  case '!':
  {
    RawValue *p;
    RawValue *zero;
    RawValueP exp;
    generateRawValue(zero, 0, IR);
    generateRawValue(exp, sign);
    if (op == '-')
      generateRawValue(p, zero, exp, RBO_SUB);
    else
      generateRawValue(p, zero, exp, RBO_EQ);
    IR.buffer[IR.len++] = (const void *)p;
    sign = "%" + to_string(alloc_now);
    MidVarTable.insert(pair<string, RawValueP>(sign, p));
    break;
  }
  default:
    assert(false);
  }
}

void PrimaryExpAST::generateGraph(RawSlice &IR, string &sign) const
{
  switch (kind)
  {
  case UNARYEXP:
    Exp->generateGraph(IR, sign);
    break;
  // case LVAL: Lval->Dump(sign);break;
  case NUMBER:
    sign = to_string(number);
    RawValue *p;
    generateRawValue(p, number, IR);
    break;
    // default: assert(0);
  }
}
