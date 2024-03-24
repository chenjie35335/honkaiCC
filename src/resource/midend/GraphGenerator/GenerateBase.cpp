#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern bool InWhile;
extern IRBuilder* irBuilder;
extern unordered_map <string,RawValueP> MidVarTable;

void CompUnitAST::generateGraph(RawProgramme *IR) const {
    irBuilder = new IRBuilder();
    IdentTable = new IdentTableNode();
    IdentTable->level = 0;
    setTempProgramme(IR);
    multCompUnit->generateGraph();
    delete IdentTable;
    delete irBuilder;
}

void MultCompUnitAST::generateGraph() const{
    for(auto &sinComp : sinCompUnit) {
        sinComp->generateGraph();
    }
}

void SinCompUnitAST::generateGraph() const{
    switch(type){
        case COMP_FUNC: {
            funcDef->generateGraph();
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
void FuncDefAST::generateGraph() const{
    string FirstBB = "entry";
    auto programme = getTempProgramme();
    auto &funcs = programme->Funcs;
    funcs.kind = RSK_FUNCTION;funcs.len  = 0;
    funcs.buffer = (const void **) malloc(sizeof(const void *) * 100);
    RawFunction* p = (RawFunction *) malloc(sizeof(RawFunction));
    setTempFunction(p);
    funcs.buffer[funcs.len++] = (const void *) p;
    p->name = ident.c_str();
    auto &bbs = p->bbs;
    bbs.kind = RSK_BASICBLOCK;bbs.len = 0;
    bbs.buffer = (const void **) malloc(sizeof(const void *)*100);
    RawBasicBlock *q = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
    bbs.buffer[bbs.len++] = (const void *)q;
    setTempBasicBlock(q);setFinished(false);
    q->name = FirstBB;
    auto &insts = q->insts;
    insts.kind = RSK_BASICVALUE;insts.len = 0;
    insts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
    block->generateGraph();
}
//这个blockAST的generateGraph对于分支语句来说是个重点
//所以需要一个数据结构来存储当前的RawFunction下的RawSlice
//到了函数阶段以后，由于产生了函数调用，所以可能还需要添加当前RawProgram下的RawSlice
void BlockAST::generateGraph() const {
    auto BlockScope = new IdentTableNode();
    if(IdentTable->child == nullptr){
      ScopeLevel++;
      BlockScope->father = IdentTable;
      BlockScope->level  = ScopeLevel;
      IdentTable->child  = BlockScope;
    }
    IdentTable = IdentTable->child;
    MulBlockItem->generateGraph();
    IdentTable = IdentTable->father;
    IdentTable->child = NULL;
    delete BlockScope;
}

void MulBlockItemAST::generateGraph() const {
    for(auto &sinBlockItem : SinBlockItem) {
        auto finished = getFinished();
        if(finished) break;
        sinBlockItem->generateGraph();
    }
}

void SinBlockItemAST::generateGraph() const{
    switch(type) {
       case SINBLOCKITEM_DEC: 
                decl->generateGraph();break;
                break;
        case SINBLOCKITEM_STM: 
                stmt->generateGraph();break;
        default:assert(0);
      }
}

void StmtAST::generateGraph() const {
      string sign;
      switch(type) {
          case STMTAST_RET:{
              SinExp->generateGraph(sign);
              RawValueP RetSrc;
              generateRawValue(RetSrc,sign);
              generateRawValue(RetSrc);
              setFinished(true);
              break;
          }
          case STMTAST_LVA: 
          {//这里虽然能过测试，但是这里没有考虑作用域的问题lv5的时候要考虑起来
              string sign1,sign2;
              Lval->Dump(sign1);
              Exp->generateGraph(sign2);
              int IdentType;
              string DestSign,SrcSign;
              IdentTable->IdentSearch(sign1,DestSign,IdentType);
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
              generateRawValue(src,dest);
              break;
              }
          }
          case STMTAST_SINE: break; 
          case STMTAST_BLO: 
              Block->generateGraph();break;
          case STMTAST_IF:
              ifStmt->generateGraph();break;
          case STMTAST_WHILE:
              WhileHead->generateGraph();break;
          case STMTAST_INWHILE:
              if(!InWhile) {
                cerr << "Break or Continue not in while scope" << endl;
                exit(-1);
              }
              //else InWhileStmt->generateGraph();
              break;
          default: assert(0);
      }
}
//这里有最后一个小问题就是常数的问题，这个暂且搁置