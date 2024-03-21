#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
#include "../../../include/midend/AST/BaseAST.h"
using namespace std;
extern unordered_map <string,RawValueP> MidVarTable;
extern IdentTableNode *t;

void CompUnitAST::generateGraph(RawProgramme &IR) const {
    IdentTable = new IdentTableNode();
    ScopeLevel = 0;
    IdentTable->level = ScopeLevel;
    multCompUnit->generateGraph(IR.Funcs);
    t = IdentTable;
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
    auto &insts = q->insts;
    insts.kind = RSK_BASICVALUE;insts.len = 0;
    insts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
    block->generateGraph(insts);
}
//这个blockAST的generateGraph对于分支语句来说是个重点
//所以需要一个数据结构来存储当前的RawFunction下的RawSlice
//到了函数阶段以后，由于产生了函数调用，所以可能还需要添加当前RawProgram下的RawSlice
void BlockAST::generateGraph(RawSlice &IR) const {
    auto BlockScope = new IdentTableNode();
    if(IdentTable->child == nullptr){
      ScopeLevel++;
      BlockScope->father = IdentTable;
      BlockScope->level  = ScopeLevel;
      IdentTable->child  = BlockScope;
      //cout << "create level:" << ScopeLevel << endl;
    }
    IdentTable = IdentTable->child;
    //cout << "push level:" << IdentTable->level << endl;
    MulBlockItem->generateGraph(IR);
    //cout << "delete level:" << IdentTable->level << endl;
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
              //int dep = p->level; 
              string sign1,sign2;
              Lval->Dump(sign1);
              Exp->generateGraph(IR,sign2);
              //p->IdentSearch(sign1,sign,type);
              int IdentType;
              string DestSign,SrcSign;
              //cout << "sign1 = " << sign1 << endl;
              IdentTable->IdentSearch(sign1,DestSign,IdentType);
              //cout << DestSign << endl;
              if(IdentType == FIND_CONST) {
                cerr << "Error: " << '"' << sign1 << '"' << " is constant and can't be altered" << endl;
                exit(-1);
              }
              else {
              SrcSign = sign2;
              RawValueP src,dest;
              generateRawValue(dest,DestSign);
              generateRawValue(src,SrcSign);
              int value = Exp->calc();
              generateRawValue(src,dest,IR);
              break;
              }
          }
          case STMTAST_SINE: break; 
          case STMTAST_BLO: 
              Block->generateGraph(IR);break;
          default: assert(0);
      }
}