#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
int IfNum = 0;
extern unordered_map <string,RawValueP> MidVarTable;
extern IdentTableNode *t;


void IfStmtAST::generateGraph() const {
    switch(type) {
        case IFSTMT_SIN: sinIfStmt->generateGraph();break;
        case IFSTMT_MUL: multElseStmt->generateGraph();break;
        default: assert(0);
    }
}
//这里该如何处理？
/*
1、每一个基本块会以br ret或者jump语句结束，这里会进行一个标记，用于标记基本块的
结束
2、对于BlockItem的遍历过程中，如果检测到某个块结束了，就要创建新的基本块
*/
void SinIfStmtAST::generateGraph() const {
    auto tempFunction = getTempFunction();
    auto &bbs = tempFunction->bbs;
    string ExpSign,ThenSign,EndSign;
    exp->generateGraph(ExpSign);
    RawValueP cond;
    generateRawValue(cond,ExpSign);
    ThenSign = "then"+to_string(IfNum);
    EndSign = "end"+to_string(IfNum);IfNum++;
    RawBasicBlock *Thenbb = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
    RawBasicBlock *Endbb  = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
    Thenbb->name = ThenSign; Endbb->name = EndSign;
    auto &ThenInsts = Thenbb->insts;auto &EndInsts = Endbb->insts;
    ThenInsts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
    EndInsts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
    ThenInsts.kind = RSK_BASICVALUE;
    EndInsts.kind = RSK_BASICVALUE;
    ThenInsts.len = 0;
    EndInsts.len = 0;
    generateRawValue(cond,Thenbb,Endbb);
    setTempBasicBlock(Thenbb);setFinished(false);
    bbs.buffer[bbs.len++] = (const void *)Thenbb;
    stmt->generateGraph();
    if(!getFinished()) {
    generateRawValue(Endbb);
    }
    setTempBasicBlock(Endbb);setFinished(false);
    bbs.buffer[bbs.len++] = (const void *)Endbb;
}

void MultElseStmtAST::generateGraph() const{
auto tempFunction = getTempFunction();
auto &bbs = tempFunction->bbs;
string ExpSign,ThenSign,ElseSign,EndSign;
exp->generateGraph(ExpSign);
RawValueP cond;
generateRawValue(cond,ExpSign);
ThenSign = "then"+to_string(IfNum);
ElseSign = "else"+to_string(IfNum);
EndSign = "end"+to_string(IfNum);IfNum++;
RawBasicBlock *Thenbb = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
RawBasicBlock *Elsebb = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
RawBasicBlock *Endbb  = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
Thenbb->name = ThenSign; Endbb->name = EndSign;Elsebb->name = ElseSign;
auto &ThenInsts = Thenbb->insts;
auto &EndInsts = Endbb->insts;
auto &ElseInsts = Elsebb->insts;
ThenInsts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
EndInsts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
ElseInsts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
ThenInsts.kind = RSK_BASICVALUE;
EndInsts.kind = RSK_BASICVALUE;
ElseInsts.kind = RSK_BASICVALUE;
ThenInsts.len = 0;
EndInsts.len = 0;
ElseInsts.len = 0;
generateRawValue(cond,Thenbb,Elsebb);
setTempBasicBlock(Thenbb);setFinished(false);
bbs.buffer[bbs.len++] = (const void *)Thenbb;
if_stmt->generateGraph();
if(!getFinished()) {
generateRawValue(Endbb);
}
setTempBasicBlock(Elsebb);setFinished(false);
bbs.buffer[bbs.len++] = (const void *)Elsebb;
else_stmt->generateGraph();
if(!getFinished()) {
generateRawValue(Endbb);
}
setTempBasicBlock(Endbb);setFinished(false);
bbs.buffer[bbs.len++] = (const void *)Endbb;
}