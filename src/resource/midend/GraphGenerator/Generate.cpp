#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
static int ValueNumber = 0;
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
          case STMTAST_RET:
              SinExp->generateGraph(IR,sign);
              RawValue* p = (RawValue *) malloc(sizeof(RawFunction));
              p->name = nullptr;
              p->value.tag = RVT_RETURN;
              RawValueP RetSrc = MidVarTable.at(sign);
              p->value.data.ret.value = RetSrc;
              IR.buffer[IR.len] = (const void *)p;
              IR.len++;
              break;
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
      // string sign1;
      //   string sign2;
        switch(type) {
          case LOREXPAST_LAN:
            LAndExp->generateGraph(IR,sign);break;
          case LOREXPAST_LOR:
            {
              break;
            }
          default: assert(0);
        }
}

void LAndExpAST::generateGraph(RawSlice &IR, string &sign) const{
       //string s1,s2;
    switch(type) {
      case LANDEXPAST_EQE:
          EqExp->generateGraph(IR,sign);break;
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
      case '-': {
          RawValue *p = (RawValue *) malloc(sizeof(RawValue));
          RawValue *zero, *exp;
          if(MidVarTable.find(to_string(0)) == MidVarTable.end()){
          zero = (RawValue *) malloc(sizeof(RawValue));
          zero->value.tag = RVT_INTEGER;
          zero->value.data.integer.value = 0;
          zero->name = nullptr;
          IR.buffer[IR.len++] = (const void *)zero;
          }
          else {
            zero = (RawValue *)MidVarTable.at(to_string(0));
          }
          if(MidVarTable.find(sign) == MidVarTable.end()) assert(0);
          else exp = (RawValue *) MidVarTable.at(sign);
          p->value.tag = RVT_BINARY;
          p->value.data.binary.lhs = zero;
          p->value.data.binary.op  = RBO_SUB;
          p->value.data.binary.rhs = exp;
          p->name = nullptr;
          IR.buffer[IR.len++] = (const void *)p;
          sign = "%"+to_string(alloc_now);
          MidVarTable.insert(pair<string,RawValueP>(to_string(0),zero));
          MidVarTable.insert(pair<string,RawValueP>(sign,p));
          break;
      }
      case '!': {
        RawValue *p = (RawValue *) malloc(sizeof(RawValue));
          RawValue *zero, *exp;
          if(MidVarTable.find(to_string(0)) == MidVarTable.end()){
          zero = (RawValue *) malloc(sizeof(RawValue));
          zero->value.tag = RVT_INTEGER;
          zero->value.data.integer.value = 0;
          zero->name = nullptr;
          IR.buffer[IR.len++] = (const void *)zero;
          }
          else {
            zero = (RawValue *)MidVarTable.at("0");
          }
          if(MidVarTable.find(sign) == MidVarTable.end()) assert(0);
          else exp = (RawValue *) MidVarTable.at(sign);
          p->value.tag = RVT_BINARY;
          p->value.data.binary.lhs = zero;
          p->value.data.binary.op  = RBO_EQ;
          p->value.data.binary.rhs = exp;
          p->name = nullptr;
          IR.buffer[IR.len++] = (const void *)p;
          sign = "%"+to_string(alloc_now);
          MidVarTable.insert(pair<string,RawValueP>(to_string(0),zero));
          MidVarTable.insert(pair<string,RawValueP>(sign,p));
          break;
      }
      default : assert(false); 
    } 
    
}

void PrimaryExpAST::generateGraph(RawSlice &IR, string &sign) const{
      switch(kind) {
        // case UNARYEXP: Exp->Dump(sign); break;
        // case LVAL: Lval->Dump(sign);break;
        case NUMBER:
              sign = to_string(number);
              RawValue *p = (RawValue *) malloc(sizeof(RawValue));
              p->name = nullptr;
              p->value.tag = RVT_INTEGER;
              p->value.data.integer.value = number;
              IR.buffer[IR.len++] = (const void *) p;
              MidVarTable.insert(pair<string,RawValueP>(sign,(RawValueP)p));
              break;
        // default: assert(0);
      }
}





