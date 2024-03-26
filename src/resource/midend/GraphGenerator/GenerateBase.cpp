#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern IRBuilder* irBuilder;
extern unordered_map <string,RawValueP> MidVarTable;

void CompUnitAST::generateGraph(RawProgramme *&IR) const {
    irBuilder = new IRBuilder();
    IdentTable = new IdentTableNode();
    IdentTable->level = 0;
    createRawProgramme(IR);
    setTempProgramme(IR);
    multCompUnit->generateGraph();
    delete IdentTable;
    delete irBuilder;
}
/*
现在主要解决返回值问题，做以下的分类讨论：
1、 如果是有返回值，但是代码中（函数的最后一个bb）无返回值时，直接ret就行（到时候a0中存什么就是什么）
2、 如果是无返回值，但是代码中存在返回值，需要报错（这时只需要看tempFunction是否有返回值就行）
3、 如果是有返回值，且函数最后一个bb有返回值，没有任何问题，funcdef那里也无需其他处理
4、 如果无返回值，且函数到最后一个bb都没有返回值，没有任何问题，但是funcdef那里需要一个ret值  
5、 如何判断返回值是一个问题，关键问题在于这个东西应该存在哪里，是直接存在后端的节点当中吗
还是说我们要重新建立一个散列表专门来表示？
（考虑到如果函数调用时要使用返回值，还是必须一个表来存储，毕竟如果使用遍历复杂度有点高）
 (这个散列表貌似还得建立从string到RawFunction之间的关系散列表，就需要在RawFunction当中存储返回值信息)
*/
void MultCompUnitAST::generateGraph() const{
    for(auto &sinComp : sinCompUnit) {
        sinComp->generateGraph();
    }
} 
//确实这里需要传给下层一个参数表示返回类型
void SinCompUnitAST::generateGraph() const{
    switch(type){
        case COMP_FUNC: {
            int retType;
            funcType->generateGraph(retType);
            funcDef->generateGraph(retType);
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
//访问函数类型
void FuncTypeAST::generateGraph(int &retType) const {
    retType = type;
}
//这里访问的是RawFunction
//这里设置funcs的位置应该提前
void FuncDefAST::generateGraph(int &retType) const{
    string FirstBB = "entry";
    RawFunction* p;
    generateRawFunction(p,ident);
    RawBasicBlock *q;
    generateRawBasicBlock(q,FirstBB);
    PushRawBasicBlock(q);
    setTempBasicBlock(q);
    setFinished(false);
    block->generateGraph();
    if(!getFinished()) {
        RawValueP RetSrc = nullptr;
        generateRawValue(RetSrc);
    }
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
              if(!InWhile()) {
                cerr << "Break or Continue not in while scope" << endl;
                exit(-1);
              }
              else InWhileStmt->generateGraph();
              break;
          default: assert(0);
      }
}
//这里有最后一个小问题就是常数的问题，这个暂且搁置