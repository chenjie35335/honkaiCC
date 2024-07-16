#include "../../../include/midend/IR/Function.h"
#include "../../../include/midend/IR/BasicBlock.h"
#include "../../../include/midend/SSA/ExitSSA.h"
#include "../../../include/midend/IR/Programme.h"
#include <assert.h>
#include <stack>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
using namespace std;
//退出SSA这里就没有这么简单的了
//但是首先，需要处理的对象就是具有phi函数的部分
//但是对于引用phi值的应该如何处理？
//首先SSA肯定是只有这一个phi,因此可以在这个phi这里插入一个load,之后所有的这个phi的使用用这个load替代就行
//然后往前的基本块插入store指令

void replaceLoad(RawValue *load,RawValue *phi) {
    for(auto &use : phi->usePoints) {
        auto tag = use->value.tag;
    switch(tag) {
        case RVT_RETURN: {
            auto &src = use->value.ret.value;
            if(src == phi) src = load;
            break;
        }
        case RVT_BINARY: {
            auto &lhs = use->value.binary.lhs;
            auto &rhs = use->value.binary.rhs;
            if(lhs == phi) lhs = load;
            if(rhs == phi) rhs = load;
            break;
        }
        case RVT_STORE: {
            auto &src = use->value.store.value;
            if(src == phi) src = load;
            break;
        }
        case RVT_BRANCH: {//branch这里会有额外的优化，不过目前先不考虑
            auto &cond = use->value.branch.cond;
            if(cond == phi) cond = load;
            break;
        }
        case RVT_CALL:{
            auto &params = use->value.call.args;
            for(auto &param : params) {
                    if(param == phi) param = load;
            }
            break;
        }
        case RVT_PHI: {
            auto &phis = use->value.phi.phi;
            for(auto &phiElem : phis) {
                if(phiElem.second == phi) phiElem.second = load;
            }
        }
        default:
            break;
    }
    }
}

void InsertLoad(RawValue* &value,RawBasicBlock *&block){
    //在block的指令之前插入一个load
    auto load = new RawValue();
    auto target = value->value.phi.target;
    load->value.tag = RVT_LOAD;
    load->value.load.src = target;
    auto ty = new RawType();
    ty->tag = RTT_POINTER;
    ty->pointer.base = target->ty;
    load->ty = ty;
    if(!value->usePoints.empty())
        block->inst.push_front(load);
    replaceLoad(load,value);
}

void InsertStore(RawValue* &value,RawBasicBlock *&block){
    auto &insts = block->inst;
    list<RawValue*>::reverse_iterator rit = insts.rbegin();
    for (; rit != insts.rend(); ++rit) {
        auto inst = *rit;
        if(inst->value.tag != RVT_BRANCH && inst->value.tag != RVT_JUMP && inst->value.tag != RVT_RETURN) {
            break;
        }
    }
    insts.insert(rit.base(),value);
}

void InsertStore(RawValue* &value){//store这里得先获得需要插入的基本块
    auto &phis = value->value.phi.phi;
    auto target = value->value.phi.target;
    for(auto phi : phis) {
        auto src = phi.second;
        auto phiTag = src->value.tag;
        auto block = phi.first;
        if(phiTag != RVT_PHI && phiTag != RVT_VALUECOPY) {
        auto store = new RawValue();
        store->value.tag = RVT_STORE;
        store->value.store.dest = target;
        store->value.store.value = src;
        auto ty = new RawType();
        ty->tag = RTT_UNIT;
        store->ty = ty;
        InsertStore(store,block);
        }
    }
}

void exitSSA(RawValue* &value,RawBasicBlock *&block) {
    assert(value->value.tag == RVT_PHI);
    InsertLoad(value,block);
    InsertStore(value);
}

void exitSSA(RawBasicBlock *&bb) {
    auto &phis = bb->phi;
    for(auto phi: phis) {
        exitSSA(phi,bb);
    }
}

void exitSSA(RawFunction * &func) {
    auto &bbs = func->basicblock;
    for(auto bb : bbs) {
        exitSSA(bb);
    }
}
/// @brief 退出SSA形式 programme层
/// @param programme 
void exitSSA(RawProgramme *&programme) {
    auto &funcs = programme->funcs;
    for(auto func: funcs) {
        exitSSA(func);
    }
}

//在基本块的入口处进行的声明