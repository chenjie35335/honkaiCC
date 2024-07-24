#ifndef FUTUROW_LOOP_OPT
#define FUTUROW_LOOP_OPT
#include <iostream>
#include <map>
#include <unordered_set>
#include <set>
#include "../IR/Programme.h"
#include "../IR/BasicBlock.h"
using namespace std;
class Loop{
    public:
    RawBasicBlock * outloopNode;//循环前置节点
    RawBasicBlock * head;//循环头部
    unordered_set<RawBasicBlock *> backEdgeStart;//回边起点(可能有多个共用一个循环头)
    unordered_set<RawBasicBlock *> body;//循环的所有基本块
    unordered_set<RawBasicBlock *> exitNode;
    unordered_set<Loop *> fatherLoops;//父循环集合
    unordered_map<RawValue *,RawBasicBlock *> loopValues;//循环的所有value
    list<RawValue * > inVar;//循环的所有不变量

    Loop(){
        head=nullptr;
    }
    Loop(RawBasicBlock* loop_head,RawBasicBlock* startNode){
        this->head = loop_head;
        this->backEdgeStart.insert(startNode);
    }
    void printBackEdge(){
        for(auto start:this->backEdgeStart){
            cout<<"回边："<<start->name<<"--->"<<this->head->name<<endl;
        }
    }
};
//判断从start基本块到end基本块的边是否是回边 
bool isBackEdge(const RawBasicBlockP start,const RawBasicBlockP end);
//计算start->end回边对应的循环的节点集合
void cal_loop( RawBasicBlock* &start, RawBasicBlock* &end,unordered_set<RawBasicBlock *> &loop);
//计算循环中的所有value
void cal_loopValues(Loop * &loop);
//计算所有的出口节点
void cal_exitNodes(Loop * loop);
//计算活跃性集合
void cal_actVal(RawFunction*func,map<RawBasicBlock*,unordered_set<RawValue*>>&actValIn,map<RawBasicBlock*,unordered_set<RawValue*>>&actValOut);
//判断是否是循环不变量(常数)
bool isInVar(RawBasicBlock* bb,RawValue * value,Loop * loop,unordered_map<RawValue *,bool> v_values,map<RawBasicBlock *,set<RawValue *>> in);
//计算循环不变量
void cal_inVar(Loop * &loop,map<RawBasicBlock *,set<RawValue *>> in);
//判断A是否支配B,或者B的必经结点是否是A
bool AisdomB(RawBasicBlock * A, RawBasicBlock * B);
//判断是否满足外提条件
bool judgeCondition(RawValue * value,Loop * loop,map<RawBasicBlock*,unordered_set<RawValue*>> actValIn,map<RawBasicBlock*,unordered_set<RawValue*>> actValOut);
//不变量外提
void move_inVar(Loop * &loop,map<RawBasicBlock*,unordered_set<RawValue*>> actValIn,map<RawBasicBlock*,unordered_set<RawValue*>> actValOut);
//查找所有回边 
void findBackEdges(const RawFunctionP func,vector<Loop *> &Loops);
//创建循环前置节点
void addLoopPreNode(RawBasicBlock * &bb,RawFunction* &func,Loop * &loop);
//将子循环的前置结点添加到父循环中
void addPreBBToLoop(vector<Loop *> natureLoops);
//定值到达分析
void ReachDef(RawFunction* func,map<RawBasicBlock *,set<RawValue *>> &in,map<RawBasicBlock *,set<RawValue *>> &out);
//循环优化
void OptimizeLoop(RawProgramme *IR);
#endif