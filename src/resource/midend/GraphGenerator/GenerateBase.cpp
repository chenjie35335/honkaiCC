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
    IR.len  = 0;
    RawFunction* p = (RawFunction *) malloc(sizeof(RawFunction));
    p->name = ident.c_str();
    IR.buffer = (const void **) malloc(sizeof(const void *));
    IR.buffer[IR.len++] = (const void *) p;
    auto &bbs = p->bbs;
    bbs.kind = RSK_BASICBLOCK;bbs.len = 0;
    bbs.buffer = (const void **) malloc(sizeof(const void *));
    RawBasicBlock *q = (RawBasicBlock *) malloc(sizeof(RawFunction));
    q->name = nullptr;
    bbs.buffer[bbs.len++] = (const void *)q;
    block->generateGraph(q->insts);
}
//这个blockAST的generateGraph对于分支语句来说是个重点
//所以需要一个数据结构来存储当前的RawFunction下的RawSlice
//到了函数阶段以后，由于产生了函数调用，所以可能还需要添加当前RawProgram下的RawSlice
void BlockAST::generateGraph(RawSlice &IR) const {
    auto BlockScope = new IdentTableNode();
    if(IdentTable->child == NULL){
      ScopeLevel++;
      BlockScope->father = IdentTable;
      BlockScope->level  = ScopeLevel;
      IdentTable->child  = BlockScope;
    }
    IdentTable = IdentTable->child;
    IR.kind = RSK_BASICVALUE;IR.len = 0;
    IR.buffer = (const void **) malloc(sizeof(const void *) * 100);
    MulBlockItem->generateGraph(IR);
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
          case STMTAST_LVA: 
          {//这里虽然能过测试，但是这里没有考虑作用域的问题lv5的时候要考虑起来
              int dep = p->level; 
              string sign1,sign2;
              cout << sign1 << endl;
              Lval->Dump(sign1);
              Exp->generateGraph(IR,sign2);
              sign1 = "@"+sign1+"_"+to_string(dep);
              RawValueP src,dest;
              generateRawValue(dest,sign1);
              generateRawValue(src,sign2);
              int value = Exp->calc();
              //p->VarAlter(sign1,sign2,value);
              generateRawValue(src,dest,IR);
              break;
          }
          case STMTAST_SINE: break;
          default: assert(0);
      }
}