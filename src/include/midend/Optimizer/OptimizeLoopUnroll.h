#ifndef FUTUROW_LOOP_Unroll
#define FUTUROW_LOOP_Unroll
#include <cassert>
#include <iostream>
#include <map>
#include <stack>
#include <unordered_set>
#include "../IR/Programme.h"
#include "../IR/BasicBlock.h"
class natureloop{
    public:
    RawBasicBlock * head;//循环头
    unordered_set<RawBasicBlock *> body;//循环体
    natureloop(RawBasicBlock* head){
        this->head=head;
    }
    void cal_loop(RawBasicBlock * start,RawBasicBlock * end);
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