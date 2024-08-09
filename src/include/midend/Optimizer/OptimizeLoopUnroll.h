#ifndef FUTUROW_LOOP_Unroll
#define FUTUROW_LOOP_Unroll
#include <cassert>
#include <iostream>
#include <map>
#include <list>
#include <stack>
#include  <algorithm>
#include <unordered_set>
#include "../IR/Programme.h"
#include "../IR/BasicBlock.h"
#define UNROLLFACTOR 2
enum class LoopType {
    NotJudge = -1,
    LoopNoneEnd,
    LoopValueEnd,
    LoopVarEnd
};
class natureloop{
    public:
    RawFunction* func;
    LoopType loopType;
    RawBasicBlock * head;//循环头
    unordered_set<RawBasicBlock *> body;//循环体
    map<RawValue*,int> loopIncreaseValue;//自增变量
    natureloop(RawBasicBlock* head){
        this->head=head;
        loopType = LoopType::NotJudge;
    }
    //计算循环节点集合
    void cal_loop(RawBasicBlock * start,RawBasicBlock * end);
    //判断循环类型
    void determineLoopType();
    //计算循环自增变量
    void cal_loopIncreaseValue();
    //固定次数的循环展开
    void unrollingValueLoop();
    //根据循环因子展开
    void unrollingVarLoop(int unRollingFactor);
    //计算展开次数
    int loopTimes(RawValue* condVal,RawValue* cond);
};
class LoopUnrolling{
    public:
    RawFunction *func;
    map<RawBasicBlock*,unordered_set<RawBasicBlock*>>domains;
    list<natureloop*> Loops;

    LoopUnrolling(){
        func = nullptr;
    }
    //初始化RawFunction指针
    void setFunc(RawFunction * newFunc){
        this->func = newFunc;
    }
    //计算每个基本块支配的节点
    void cal_domians();
    //计算RawFunction中的自然循环
    void find_loops();
};
//循环优化
void OptimizeLoopUnroll(RawProgramme *IR);
#endif