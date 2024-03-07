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
        //if(sinBlockItem->calc() == STMTAST_RET) {
          //  break;//减支
        //}
      }
}

void SinBlockItemAST::generateGraph(RawSlice &IR) const{
    switch(type) {
       case SINBLOCKITEM_DEC: 
                //decl->generateGraph(IR);break;
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
          // case SINEXPAST_NULL:
          // sign = ""; break;
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
          //cout << "enter lorexp2" << endl;
            LAndExp->generateGraph(IR,sign);break;
          case LOREXPAST_LOR:
            {
              // LOrExp->generateGraph(IR,sign1);
              // LAndExp->generateGraph(IR,sign2);
              // RawValue * p = (RawValue *) malloc(sizeof(RawValue));
              // RawValueP LhsSrc = MidVarTable.at(sign1);
              // RawValueP RhsSrc = MidVarTable.at(sign2);
              // p->value.tag = RVT_BINARY;
              // p->value.data.binary.op = RBO_OR;
              // p->value.data.binary.lhs = LhsSrc;
              // p->value.data.binary.rhs = RhsSrc;
              // IR.buffer[IR.len++] = (const void *)p;
              // MidVarTable.insert(pair<string,RawValueP>(sign,(RawValueP)p));
              break;
            }
          default: assert(0);
        }
}

void LAndExpAST::generateGraph(RawSlice &IR, string &sign) const{
       //string s1,s2;
    switch(type) {
      case LANDEXPAST_EQE:
      //cout << "enter landexp1" << endl;
          EqExp->generateGraph(IR,sign);break;
      // case LANDEXPAST_LAN:
      //     {
      //       //cout << "enter landexp1" << endl;
      //       LAndExp->Dump(s1);
      //       EqExp->Dump(s2);
      //       Dump(s1,s2,sign);
      //       break;
      //     }
      default: 
          assert(0);
    }
}

void EqExpAST::generateGraph(RawSlice &IR, string &sign) const{
//string s1,s2;
     switch(type) {
        case EQEXPAST_REL: {
          RelExp->generateGraph(IR,sign);break;
        }
        // case EQEXPAST_EQE: {
        //   EqExp->Dump(s1);
        //   RelExp->Dump(s2);
        //   EqOp->Dump(s1,s2,sign);
        //   break;
        // }
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
      // case RELEXPAST_REL: {
      //   RelExp->Dump(s1);
      //   AddExp->Dump(s2);
      //   RelOp->Dump(s1,s2,sign);
      //   break;
      // }
      default:
        assert(0);
      }
}

void AddExpAST::generateGraph(RawSlice &IR, string &sign) const{
  switch(type) {
        case MULEXP:  
            MulExp->generateGraph(IR,sign);break;
        // case ADDMUL:
        // {
        //     string sign1;
        //     string sign2;
        //     AddExp->Dump(sign1);
        //     MulExp->Dump(sign2);
        //     AddOp->Dump(sign1,sign2,sign);
        //     break;
        // }
        default:
            assert(0);
      }
}

void MulExpAST::generateGraph(RawSlice &IR, string &sign) const{
  switch(type) {
        // case MULEXPAST_MUL: {
        //   string sign1;
        //   string sign2;
        //   MulExp->Dump(sign1);
        //   UnaryExp->Dump(sign2);
        //   MulOp->Dump(sign1,sign2,sign);
        //   break;
        // }
        case MULEXPAST_UNA: UnaryExp->generateGraph(IR,sign);break;
        default: assert(0);
      }
}

void UnaryExpAST_P::generateGraph(RawSlice &IR, string &sign) const{
    PrimaryExp->generateGraph(IR,sign);
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





