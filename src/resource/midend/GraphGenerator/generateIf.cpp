#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
int IfNum = 0;
RawFunction *tempFunction = nullptr;
unordered_map <string,RawBasicBlockP> MidBbTable;
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
    string ExpSign,ThenSign,EndSign;
    exp->generateGraph(ExpSign);
    RawValueP cond;
    generateRawValue(cond,ExpSign);
    ThenSign = "%then"+to_string(IfNum);
    EndSign = "%end"+to_string(IfNum);IfNum++;
    RawBasicBlock *Thenbb = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
    RawBasicBlock *Endbb  = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
    Thenbb->name = nullptr; Endbb->name = nullptr;
    auto &bbs = tempFunction->bbs;
    bbs.buffer[bbs.len++] = (const void *)Thenbb;
    bbs.buffer[bbs.len++] = (const void *)Endbb;
    
    
}