#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
int ValueNumber = 0;
unordered_map <string,RawValueP> MidVarTable;
//这里这个RawProgramme是生成好
void CompUnitAST::generateGraph(RawProgramme &IR) const {
    func_def->generateGraph(IR.Funcs);
}

//这里访问的是RawFunction
void FuncDefAST::generateGraph(RawSlice &IR) const{
    IR.kind = RSK_FUNCTION;
    IR.len  = 1;
    RawFunction* p = (RawFunction *) malloc(sizeof(RawFunction));
    p->name = ident.c_str();
    IR.buffer = (const void **) malloc(sizeof(const void *));
    IR.buffer[0] = (const void *) p;
    block->generateGraph(p->bbs);
}

void BlockAST::generateGraph(RawSlice &IR) const {
    IR.kind = RSK_BASICBLOCK;
    IR.len = 1;
    RawBasicBlock* p = (RawBasicBlock *) malloc(sizeof(RawFunction));
    p->name = nullptr;
    IR.buffer = (const void **) malloc(sizeof(const void *));
    IR.buffer[0] = (const void *) p;
    auto &inst = p->insts;
    inst.kind = RSK_BASICVALUE;
    inst.len = 0;
    inst.buffer = (const void **) malloc(sizeof(const void *) * 100);
    MulBlockItem->generateGraph(p->insts);
}

void MulBlockItemAST::generateGraph(RawSlice &IR) const {
    for(auto &sinBlockItem : SinBlockItem) {
        sinBlockItem->generateGraph(IR);
      }
}

void SinBlockItemAST::generateGraph(RawSlice &IR) const{
    switch(type) {
       case SINBLOCKITEM_DEC: 
                break;
        case SINBLOCKITEM_STM: 
                stmt->generateGraph(IR);break;
        default:assert(0);
      }
}

void StmtAST::generateGraph(RawSlice &IR) const {
      auto p = IdentTable;
      string sign;
      switch(type) {
          case STMTAST_RET:{
              SinExp->generateGraph(IR,sign);
              RawValue *p; RawValueP RetSrc;
              generateRawValue(RetSrc,sign);
              generateRawValue(p , RetSrc);
              IR.buffer[IR.len] = (const void *)p;
              IR.len++;
              break;
          }
          case STMTAST_SINE: break;
      }
}

void SinExpAST::generateGraph(RawSlice &IR, string &sign) const{
    switch(type) {
          case SINEXPAST_EXP:
          Exp->generateGraph(IR,sign); break;
          default: 
          assert(0); 
      }
}

void ExpAST::generateGraph(RawSlice &IR, string &sign) const{
      LOrExp->generateGraph(IR,sign);
}

void LOrExpAST::generateGraph(RawSlice &IR, string &sign) const{
        string sign1,sign2;
        switch(type) {
          case LOREXPAST_LAN:
            LAndExp->generateGraph(IR,sign);break;
          case LOREXPAST_LOR:
            {
              LOrExp->generateGraph(IR,sign1);
              LAndExp->generateGraph(IR,sign2);
              alloc_now++;
              sign = "%"+to_string(alloc_now);
              RawValueP lhs, rhs;
              generateRawValue(lhs,sign1);
              generateRawValue(rhs,sign2);
              RawValue *StmtOR;
              generateRawValue(StmtOR,lhs,rhs,RBO_OR);
              MidVarTable.insert(pair<string,RawValueP>(sign,StmtOR));
              IR.buffer[IR.len++] = (const void *) StmtOR;
              alloc_now++;
              sign = "%"+to_string(alloc_now);
              RawValue *zero;
              generateRawValue(zero,0,IR);
              RawValue *StmtNEQ;
              generateRawValue(StmtNEQ,StmtOR,zero,RBO_NOT_EQ);
              IR.buffer[IR.len++] = (const void *) StmtNEQ;
              MidVarTable.insert(pair<string,RawValueP>(sign,StmtNEQ));
              break;
            }
          default: assert(0);
        }
}

void LAndExpAST::generateGraph(RawSlice &IR, string &sign) const{
       string s1,s2;
    switch(type) {
      case LANDEXPAST_EQE:
          EqExp->generateGraph(IR,sign);break;
      case LANDEXPAST_LAN:{
            LAndExp->generateGraph(IR,s1);
            EqExp->generateGraph(IR,s2);
            RawValueP signL,signR;
            generateRawValue(signL,s1);
            generateRawValue(signR,s2);
            alloc_now++;sign = "%"+to_string(alloc_now);
            RawValue *zero;
            generateRawValue(zero,0,IR);
            RawValue *StmtNeL, *StmtNeR;
            generateRawValue(StmtNeL,signL,zero,RBO_NOT_EQ);
            IR.buffer[IR.len++] = (const void *) StmtNeL;
            MidVarTable.insert(pair<string,RawValueP>(sign,StmtNeL));
            alloc_now++;sign = "%"+to_string(alloc_now);
            generateRawValue(StmtNeR,signR,zero,RBO_NOT_EQ);
            IR.buffer[IR.len++] = (const void *) StmtNeR;
            MidVarTable.insert(pair<string,RawValueP>(sign,StmtNeR));
            alloc_now++;sign = "%"+to_string(alloc_now);
            RawValue *StmtAnd;
            generateRawValue(StmtAnd,StmtNeL,StmtNeR,RBO_AND);
            IR.buffer[IR.len++] = (const void *) StmtNeR;
            MidVarTable.insert(pair<string,RawValueP>(sign,StmtNeR));
            break;
      }
      default: 
          assert(0);
    }
}

void EqExpAST::generateGraph(RawSlice &IR, string &sign) const{
     switch(type) {
        case EQEXPAST_REL: {
          RelExp->generateGraph(IR,sign);break;
        }
        default:
          assert(0);
     }
}

void RelExpAST::generateGraph(RawSlice &IR, string &sign) const{
      //string s1,s2;
      switch(type) {
      case RELEXPAST_ADD: {
        AddExp->generateGraph(IR,sign);break;
      }
      default:
        assert(0);
      }
}

void AddExpAST::generateGraph(RawSlice &IR, string &sign) const{
  switch(type) {
        case MULEXP:  
            MulExp->generateGraph(IR,sign);break;
        default:
            assert(0);
      }
}

void MulExpAST::generateGraph(RawSlice &IR, string &sign) const{
  switch(type) {
        case MULEXPAST_UNA: UnaryExp->generateGraph(IR,sign);break;
        default: assert(0);
      }
}

void UnaryExpAST_P::generateGraph(RawSlice &IR, string &sign) const{
    PrimaryExp->generateGraph(IR,sign);
}

//其实这里分开确实不太好
void UnaryExpAST_U::generateGraph(RawSlice &IR,string &sign) const{
    UnaryExp->generateGraph(IR,sign);
    UnaryOp->generateGraph(IR,sign);
}

void UnaryOpAST::generateGraph(RawSlice &IR,string &sign) const {
    alloc_now++;
    
    switch(op) {
      case '+': break;
      case '-':case '!': {
          RawValue *p;
          RawValue *zero;RawValueP exp;
          generateRawValue(zero,0,IR);
          generateRawValue(exp,sign);
          if(op == '-') generateRawValue(p,zero,exp,RBO_SUB);
          else  generateRawValue(p,zero,exp,RBO_EQ);
          IR.buffer[IR.len++] = (const void *)p;
          sign = "%"+to_string(alloc_now);
          MidVarTable.insert(pair<string,RawValueP>(sign,p));
          break;
      }
      default : assert(false); 
    } 
    
}

void PrimaryExpAST::generateGraph(RawSlice &IR, string &sign) const{
      switch(kind) {
        case UNARYEXP: Exp->generateGraph(IR,sign); break;
        // case LVAL: Lval->Dump(sign);break;
        case NUMBER:
              sign = to_string(number);
              RawValue *p;
              generateRawValue(p,number,IR);
              break;
        // default: assert(0);
      }
}





