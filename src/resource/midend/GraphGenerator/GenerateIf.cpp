#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
int IfNum = 0;
int WhileNum = 0;
void IfStmtAST::generateGraph() const
{
    switch (type)
    {
    case IFSTMT_SIN:
        sinIfStmt->generateGraph();
        break;
    case IFSTMT_MUL:
        multElseStmt->generateGraph();
        break;
    default:
        assert(0);
    }
}
// 这里该如何处理？
/*
1、每一个基本块会以br ret或者jump语句结束，这里会进行一个标记，用于标记基本块的
结束
2、对于BlockItem的遍历过程中，如果检测到某个块结束了，就要创建新的基本块
*/
/*
如果if语句中出现了break和continue怎么办？
首先肯定得宣告某个basicblock结束，
其次最难的问题在于这个jump应该跳向哪里的问题
*/
void SinIfStmtAST::generateGraph() const
{
    string ExpSign, ThenSign, EndSign;
    RawBasicBlock *Thenbb, *Endbb;
    exp->generateGraph(ExpSign);
    RawValueP cond;
    getMidVarValue(cond, ExpSign);
    ThenSign = "then" + to_string(IfNum);
    EndSign = "end" + to_string(IfNum);
    IfNum++;
    generateRawBasicBlock(Thenbb,ThenSign.c_str());
    generateRawBasicBlock(Endbb,EndSign.c_str());
    generateRawValue(cond, Thenbb, Endbb);
    PushFollowBasieBlock(Thenbb,Endbb);
    PushRawBasicBlock(Thenbb);
    setTempBasicBlock(Thenbb);
    setFinished(false);
    stmt->generateGraph();
    if (!getFinished())
    {
        generateRawValue(Endbb);
        PushFollowBasieBlock(Endbb);
    }
    setTempBasicBlock(Endbb);
    setFinished(false);
    PushRawBasicBlock(Endbb);
}

void MultElseStmtAST::generateGraph() const
{
    string ExpSign, ThenSign, ElseSign, EndSign;
    exp->generateGraph(ExpSign);
    RawValueP cond;
    getMidVarValue(cond, ExpSign);
    ThenSign = "then" + to_string(IfNum);
    ElseSign = "else" + to_string(IfNum);
    EndSign = "end" + to_string(IfNum);
    IfNum++;
    RawBasicBlock *Thenbb,*Elsebb,*Endbb;
    generateRawBasicBlock(Thenbb,ThenSign.c_str());
    generateRawBasicBlock(Elsebb,ElseSign.c_str());
    generateRawBasicBlock(Endbb,EndSign.c_str());
    generateRawValue(cond, Thenbb, Elsebb);
    PushFollowBasieBlock(Thenbb,Elsebb);
    setTempBasicBlock(Thenbb);
    PushRawBasicBlock(Thenbb);
    setFinished(false);
    if_stmt->generateGraph();
    if (!getFinished())
    {
        generateRawValue(Endbb);
        PushFollowBasieBlock(Endbb);
    }
    setTempBasicBlock(Elsebb);
    PushRawBasicBlock(Elsebb);
    setFinished(false);
    else_stmt->generateGraph();
    if (!getFinished())
    {
        generateRawValue(Endbb);
        PushFollowBasieBlock(Endbb);
    }
    PushRawBasicBlock(Endbb);
    setTempBasicBlock(Endbb);
    setFinished(false);
}

void WhileStmtHeadAST::generateGraph() const
{
    WhileHead->generateGraph();
}

void WhileStmtAST::generateGraph() const
{
    string ExpSign;
    string EntrySign, BodySign, EndSign;
    EntrySign = "while_entry" + to_string(WhileNum);
    BodySign = "while_body" + to_string(WhileNum);
    EndSign = "while_end" + to_string(WhileNum);
    WhileNum++;
    RawBasicBlock *Entrybb,*Bodybb,*Endbb;
    generateRawBasicBlock(Entrybb,EntrySign.c_str());
    generateRawBasicBlock(Bodybb,BodySign.c_str());
    generateRawBasicBlock(Endbb,EndSign.c_str());
    generateRawValue(Entrybb);
    PushFollowBasieBlock(Entrybb);
    setTempBasicBlock(Entrybb);
    PushRawBasicBlock(Entrybb);
    setFinished(false);
    exp->generateGraph(ExpSign);
    RawValueP cond;
    getMidVarValue(cond, ExpSign);
    generateRawValue(cond, Bodybb, Endbb);
    PushFollowBasieBlock(Bodybb,Endbb);
    setTempBasicBlock(Bodybb);
    PushRawBasicBlock(Bodybb);
    setFinished(false);
    pushTempWhileEntry(Entrybb);
    pushTempWhileEnd(Endbb);
    stmt->generateGraph();
    if (!getFinished())
    {
        generateRawValue(Entrybb);
        PushFollowBasieBlock(Entrybb);
    }
    setTempBasicBlock(Endbb);
    PushRawBasicBlock(Endbb);
    setFinished(false);
    popTempWhileEntry();
    popTempWhileEnd();
}

void InWhileAST::generateGraph() const
{
    switch (type)
    {
    case STMTAST_BREAK:
    {
        RawBasicBlock *end = getTempWhileEnd();
        generateRawValue(end);
        PushFollowBasieBlock(end);
        setFinished(true);
        break;
    }
    case STMTAST_CONTINUE:
    {
        RawBasicBlock *entry = getTempWhileEntry();
        generateRawValue(entry);
        PushFollowBasieBlock(entry);
        setFinished(true);
        break;
    }
    default:
        assert(0);
    }
}

// 关于这个break和continue的jump应该跳向哪里
// 两个方案：
