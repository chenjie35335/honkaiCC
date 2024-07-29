#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include <cstdlib>
#include <unordered_map>
#include <memory>
using namespace std;
extern int IfNum;
extern int WhileNum;
// 这里这个RawProgramme是生成好

void SinExpAST::generateGraph(string &sign,int &RetType) const
{
  RetType = type;
  switch (type)
  {
  case SINEXPAST_EXP:
    Exp->generateGraph(sign);
    break;
  case SINEXPAST_NULL:
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
    string name = "aaaaaaa" + to_string(++alloc_now);
    generateRawValue(name,RTT_INT32);
    RawValueP MidValueData = signTable.getVarL(name);
    RawValueP zero,one,zeroFloat;
    string ZeroSign = to_string(0);
    string OneSign = to_string(1);
    generateRawValue(0);
    generateRawValue((float)0.0);
    getMidVarValue(zero, ZeroSign);
    generateRawValue(zero,MidValueData);
    string ThenSign = "scthen" + to_string(IfNum);
    string ElseSign = "scelse" + to_string(IfNum);
    string EndSign = "scend" + to_string(IfNum);
    IfNum++;
    RawBasicBlock *Thenbb,*Elsebb,*Endbb;
    generateRawBasicBlock(Thenbb,ThenSign.c_str());
    generateRawBasicBlock(Elsebb,ElseSign.c_str());
    generateRawBasicBlock(Endbb,EndSign.c_str());
    LOrExp->generateGraph(sign1);
    RawValueP cond1 = signTable.getMidVar(sign1);
    if(cond1->ty->tag == RTT_FLOAT) {
      string FloatZero = to_string(0.0);
      getMidVarValue(zeroFloat,FloatZero);
      generateRawValue(sign, cond1, zeroFloat, RBO_NOT_FEQ);
      cond1 = signTable.getMidVar(sign);
    }
    generateRawValue(cond1, Thenbb, Elsebb);
    PushFollowBasieBlock(Thenbb,Elsebb);
    setTempBasicBlock(Thenbb);
    PushRawBasicBlock(Thenbb);
    setFinished(false);
    generateRawValue(1);
    getMidVarValue(one, OneSign);
    generateRawValue(one,MidValueData);
    generateRawValue(Endbb);
    PushFollowBasieBlock(Endbb);
    setTempBasicBlock(Elsebb);
    PushRawBasicBlock(Elsebb);
    setFinished(false);
    LAndExp->generateGraph(sign2);
    RawValueP cond2 = signTable.getMidVar(sign2);
    if(cond2->ty->tag == RTT_FLOAT) {
      string FloatZero = to_string(0.0);
      getMidVarValue(zeroFloat,FloatZero);
      generateRawValue(sign, cond2, zeroFloat, RBO_NOT_FEQ);
      cond2 = signTable.getMidVar(sign);
    }
    generateRawValue(cond2, Thenbb, Endbb);
    PushFollowBasieBlock(Thenbb,Endbb);
    setTempBasicBlock(Endbb);
    PushRawBasicBlock(Endbb);
    setFinished(false);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    generateRawValue(sign,MidValueData);
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
    string name = "bbbbbb" + to_string(++alloc_now);
    generateRawValue(name,RTT_INT32);
    RawValueP MidValueData = signTable.getVarL(name);
    RawValueP zero,one;
    string ZeroSign = to_string(0);
    string OneSign = to_string(1);
    generateRawValue(1);
    getMidVarValue(one, OneSign);
    generateRawValue(one,MidValueData);
    string ThenSign = "scthen" + to_string(IfNum);
    string ElseSign = "scelse" + to_string(IfNum);
    string EndSign = "scend" + to_string(IfNum);
    IfNum++;
    RawBasicBlock *Thenbb,*Elsebb,*Endbb;
    generateRawBasicBlock(Thenbb,ThenSign.c_str());
    generateRawBasicBlock(Elsebb,ElseSign.c_str());
    generateRawBasicBlock(Endbb,EndSign.c_str());
    LAndExp->generateGraph(s1);
    RawValueP lhs = signTable.getMidVar(s1);
    s1 = "@" + to_string(++alloc_now);
    if(lhs->ty->tag == RTT_FLOAT) {
      generateRawValue((float)0.0);
      string ZeroFloat = to_string(0.0);
      getMidVarValue(zero, ZeroFloat);
      generateRawValue(s1,zero,lhs,RBO_FEQ);
    } else{
      generateRawValue(0);
      getMidVarValue(zero, ZeroSign);
      generateRawValue(s1,zero,lhs,RBO_EQ);
    }
    RawValueP cond1 = signTable.getMidVar(s1);
    generateRawValue(cond1, Thenbb, Elsebb);
    PushFollowBasieBlock(Thenbb,Elsebb);
    setTempBasicBlock(Thenbb);
    PushRawBasicBlock(Thenbb);
    setFinished(false);
    generateRawValue(0);
    getMidVarValue(zero, ZeroSign);
    generateRawValue(zero,MidValueData);
    generateRawValue(Endbb);
    PushFollowBasieBlock(Endbb);
    setTempBasicBlock(Elsebb);
    PushRawBasicBlock(Elsebb);
    setFinished(false);
    EqExp->generateGraph(s2);
    RawValueP rhs = signTable.getMidVar(s2);
    s2 = "@" + to_string(++alloc_now);
    if(rhs->ty->tag == RTT_FLOAT) {
      generateRawValue((float)0.0);
      string ZeroFloat = to_string(0.0);
      getMidVarValue(zero, ZeroSign);
      generateRawValue(s2,zero,rhs,RBO_FEQ);
    } else {
      generateRawValue(0);
      getMidVarValue(zero, ZeroSign);
      generateRawValue(s2,zero,rhs,RBO_EQ);
    }
    RawValueP cond2 = signTable.getMidVar(s2);
    generateRawValue(cond2, Thenbb, Endbb);
    PushFollowBasieBlock(Thenbb,Endbb);
    setTempBasicBlock(Endbb);
    PushRawBasicBlock(Endbb);
    setFinished(false);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    generateRawValue(sign,MidValueData);
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
    int OpType = EqOp->getType();
    EqExp->generateGraph(s1);
    RelExp->generateGraph(s2);
    RawValueP signL, signR;
    getMidVarValue(signL, s1);
    getMidVarValue(signR, s2);
    auto LTag = signL->ty->tag;
    auto RTag = signR->ty->tag;
    auto OpFloat = (LTag == RTT_FLOAT || RTag == RTT_FLOAT);
    if(LTag == RTT_FLOAT && RTag != RTT_FLOAT) {
      generateConvert(signR,s2);
      getMidVarValue(signR, s2);
    }
    if(LTag != RTT_FLOAT && RTag == RTT_FLOAT) {
      generateConvert(signL,s1);
      getMidVarValue(signL, s1);
    }
    alloc_now++; sign = "%" + to_string(alloc_now);
    switch (OpType)
    {
    case EQOPAST_EQ:
      //signL->value.tag
      if(OpFloat){//这里需要判断
        generateRawValue(sign, signL, signR, RBO_FEQ);
      } else {
        generateRawValue(sign, signL, signR, RBO_EQ);
      }
      break;
    case EQOPAST_NE:
      if(OpFloat){
        generateRawValue(sign, signL, signR, RBO_NOT_FEQ);
      } else {
        generateRawValue(sign, signL, signR, RBO_NOT_EQ);
      }
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
    int OpRel = RelOp->getType();
    RawValueP signL, signR;
    getMidVarValue(signL, s1);
    getMidVarValue(signR, s2);
    auto LTag = signL->ty->tag;
    auto RTag = signR->ty->tag;
    auto OpFloat = (LTag == RTT_FLOAT || RTag == RTT_FLOAT);
    if(LTag == RTT_FLOAT && RTag != RTT_FLOAT) {
      generateConvert(signR,s2);
      getMidVarValue(signR, s2);
    }
    if(LTag != RTT_FLOAT && RTag == RTT_FLOAT) {
      generateConvert(signL,s1);
      getMidVarValue(signL, s1);
    }
    alloc_now++; sign = "%" + to_string(alloc_now);
    switch (OpRel)
    {
    case RELOPAST_GE:
      if(OpFloat)
        generateRawValue(sign, signL, signR, RBO_FGE);
      else 
        generateRawValue(sign, signL, signR, RBO_GE);
      break;
    case RELOPAST_LE:
      if(OpFloat){
        generateRawValue(sign, signL, signR, RBO_FLE);
      }else {
        generateRawValue(sign, signL, signR, RBO_LE);
      }
      break;
    case RELOPAST_L:
      if(OpFloat){
        generateRawValue(sign, signL, signR, RBO_FLT);
      }else {
        generateRawValue(sign, signL, signR, RBO_LT);
      }
      break;
    case RELOPAST_G:
      if(OpFloat){
        generateRawValue(sign, signL, signR, RBO_FGT);
      }else {
        generateRawValue(sign, signL, signR, RBO_GT);
      }
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
    int OpAdd = AddOp->getType();
    RawValueP signL, signR;
    getMidVarValue(signL, s1);
    getMidVarValue(signR, s2);
    auto LTag = signL->ty->tag;
    auto RTag = signR->ty->tag;
    auto OpFloat = (LTag == RTT_FLOAT || RTag == RTT_FLOAT);
    if(LTag == RTT_FLOAT && RTag != RTT_FLOAT) {
      generateConvert(signR,s2);
      getMidVarValue(signR, s2);
    }
    if(LTag != RTT_FLOAT && RTag == RTT_FLOAT) {
      generateConvert(signL,s1);
      getMidVarValue(signL, s1);
    }
    alloc_now++; sign = "%" + to_string(alloc_now);
    switch (OpAdd){
    case '+':
      if(OpFloat){
        generateRawValue(sign, signL, signR, RBO_FADD);
      } else {
        generateRawValue(sign, signL, signR, RBO_ADD);
      }
      break;
    case '-':
      if(OpFloat){
        generateRawValue(sign, signL, signR, RBO_FSUB);
      }else {
        generateRawValue(sign, signL, signR, RBO_SUB);
      }
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
    int OpMul = MulOp->getType();
    RawValueP signL, signR;
    getMidVarValue(signL, s1);
    getMidVarValue(signR, s2);
    auto LTag = signL->ty->tag;
    auto RTag = signR->ty->tag;
    auto OpFloat = (LTag == RTT_FLOAT || RTag == RTT_FLOAT);
    if(LTag == RTT_FLOAT && RTag != RTT_FLOAT) {
      generateConvert(signR,s2);
      getMidVarValue(signR, s2);
    }
    if(LTag != RTT_FLOAT && RTag == RTT_FLOAT) {
      generateConvert(signL,s1);
      getMidVarValue(signL, s1);
    }
    alloc_now++; sign = "%" + to_string(alloc_now);
    switch (OpMul)
    {
    case '*':
      if(OpFloat){
        generateRawValue(sign, signL, signR, RBO_FMUL);
      } else {
        generateRawValue(sign, signL, signR, RBO_MUL);
      }
      break;
    case '/':
      if(OpFloat){
        generateRawValue(sign, signL, signR, RBO_FDIV);
      }else {
        generateRawValue(sign, signL, signR, RBO_DIV);
      }
      break;
    case '%':
      //浮点数不取余
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
  //cout << "p" << endl;
  PrimaryExp->generateGraph(sign);
}

void UnaryExpAST_U::generateGraph(string &sign) const
{
  //cout << "u" << endl;
  UnaryExp->generateGraph(sign);
  UnaryOp->generateGraph(sign);
}

void UnaryExpAST_F::generateGraph(string &sign) const{
  //cout << "f" << endl;
  FuncExp->generateGraph(sign);
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
    RawValueP zero, FloatZero;
    int number = 0;
    float floatNumber = 0.0;
    string ZeroSign = to_string(0);
    string FloatZeroSign = to_string(0.0);
    generateRawValue(0);
    generateRawValue(floatNumber);
    getMidVarValue(zero, ZeroSign);
    getMidVarValue(FloatZero, FloatZeroSign);
    getMidVarValue(exp, sign);
    alloc_now++;
    sign = "%" + to_string(alloc_now);
    if (op == '-')
      if(exp->ty->tag == RTT_FLOAT){
        generateRawValue(sign, FloatZero, exp, RBO_FSUB);
      }else 
        generateRawValue(sign,zero, exp, RBO_SUB);
    else
      if(exp->ty->tag == RTT_FLOAT)
       generateRawValue(sign, FloatZero, exp, RBO_FEQ);
      else 
       generateRawValue(sign,zero,exp,RBO_EQ);
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
  case FLOAT_NUMBER:
    sign = to_string(floatNumber);
    generateRawValue(floatNumber);
    break;
  case LVAL:
    Lval->generateGraph(sign);
    break;
  default:
    assert(0);
  }
}


// to calc()
void FuncExpAST::generateGraph(string &sign) const{
  //cerr << "function name = " << ident << endl;
  RawFunctionP callee= signTable.getFunction(ident);
  vector<RawValueP> paramsValue;
  para->generateGraph(paramsValue);
  generateRawValue(callee,paramsValue,sign);
}

void ParamsAST::generateGraph(vector<RawValueP> &params) const {
  for(auto &sinParam : sinParams) {
    RawValueP param;
    sinParam->generateGraph(param);
    params.push_back(param);
  }
}

void SinParamsAST::generateGraph(RawValueP &params) const {
  string ExpSign;
  exp->generateGraph(ExpSign);
  //判断分析出是否能算出来
  getMidVarValue(params,ExpSign);
}
//如果我最后发现没有返回值该怎么办？
//没有办法，这里还是只能说返回一个vector,因为必须在这里生成