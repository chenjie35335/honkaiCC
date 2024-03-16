#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
#include "../../../include/midend/AST/BaseAST.h"
using namespace std;
extern unordered_map <string,RawValueP> MidVarTable;

void CompUnitAST::generateGraph(RawProgramme &IR) const {
    IdentTable = new IdentTableNode();
    ScopeLevel = 0;
    IdentTable->level = ScopeLevel;
    multCompUnit->generateGraph(IR.Funcs);
    delete IdentTable;
}

void MultCompUnitAST::generateGraph(RawSlice &IR) const{
    for(auto &sinComp : sinCompUnit) {
        sinComp->generateGraph(IR);
    }
}

void SinCompUnitAST::generateGraph(RawSlice &IR) const{
    switch(type){
        case COMP_FUNC: {
            funcDef->generateGraph(IR);
            break;
        }
        case COMP_CON:
            //constGlobal->Dump();
            break;
        case COMP_VAR:{
            //varGlobal->Dump(DECL_GLOB);
            break;
        }
        default:
            assert(0);
      }
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
    auto BlockScope = new IdentTableNode();
    if(IdentTable->child == NULL){
      ScopeLevel++;
      BlockScope->father = IdentTable;
      BlockScope->level  = ScopeLevel;
      IdentTable->child  = BlockScope;
    }
    IdentTable = IdentTable->child;
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
    IdentTable = IdentTable->father;
    IdentTable->child = NULL;
    delete BlockScope;
}

void MulBlockItemAST::generateGraph(RawSlice &IR) const {
    for(auto &sinBlockItem : SinBlockItem) {
        sinBlockItem->generateGraph(IR);
      }
}

void SinBlockItemAST::generateGraph(RawSlice &IR) const{
    switch(type) {
       case SINBLOCKITEM_DEC: 
                decl->generateGraph(IR);break;
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