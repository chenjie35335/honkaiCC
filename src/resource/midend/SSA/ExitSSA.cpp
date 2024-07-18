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
//Insert Load这里依旧存在问题，但是不大
RawValue * InsertLoad(RawValue* value){
    //在block的指令之前插入一个load
    auto load = new RawValue();
    auto target = (RawValue *)value->value.phi.target;
    load->value.tag = RVT_LOAD;
    load->value.load.src = target;
    auto ty = new RawType();
    ty->tag = RTT_POINTER;
    ty->pointer.base = target->ty;
    load->ty = ty;
    target->usePoints.push_back(load);
    return load;
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
    // cout << "target tag: " << target->name << endl;
    for(auto phi : phis) {
        auto src = phi.second;
        auto phiTag = src->value.tag;
        auto block = phi.first;
        auto store = new RawValue();
        if(phiTag == RVT_PHI) {
            auto Phitarget = src->value.phi.target;
            if(Phitarget == target) {
                // cout << "skip store" << endl;
                continue;
            }
        }
        // cout << "ready to store " << phiTag << endl; 
        store->value.tag = RVT_STORE;
        store->value.store.dest = target;
        store->value.store.value = src;
        auto ty = new RawType();
        ty->tag = RTT_UNIT;
        store->ty = ty;
        InsertStore(store,block);
    }
}

void exitSSA(RawValue* &value,RawBasicBlock *&block) {
    assert(value->value.tag == RVT_PHI);
    InsertStore(value);
}

void exitSSA(RawBasicBlock *&bb) {
    auto &phis = bb->phi;
    for(auto phi: phis) {
        exitSSA(phi,bb);
    }
}

void InsertLoad(RawBasicBlock *&bb) {
    auto &insts = bb->inst;
    auto it = insts.begin();
    for(;it != insts.end();it++) {
        auto inst = *it;
        auto tag = inst->value.tag;
        switch(tag) {
        case RVT_RETURN: {
            auto &src = inst->value.ret.value;
            if(!src) break;
            if(src->value.tag == RVT_PHI) {
                auto load = InsertLoad((RawValue *)src);
                src = load;
                insts.insert(it,load);
            }
            break;
        }
        case RVT_BINARY: {
            auto &lhs = inst->value.binary.lhs;
            auto &rhs = inst->value.binary.rhs;
            if(lhs->value.tag == RVT_PHI) {
                auto load = InsertLoad((RawValue *)lhs);
                lhs = load;
                insts.insert(it,load);
            }
            if(rhs->value.tag == RVT_PHI) {
                auto load = InsertLoad((RawValue *)rhs);
                rhs = load;
                insts.insert(it,load);
            }
            break;
        }
        case RVT_STORE: {
            auto &src = inst->value.store.value;
            if(src->value.tag == RVT_PHI) {
                // cout << "replace " << src->value.phi.target->name << endl;
                auto load = InsertLoad((RawValue *)src);
                src = load;
                insts.insert(it,load);
            }
            break;
        }
        case RVT_BRANCH: {//branch这里会有额外的优化，不过目前先不考虑
            auto &cond = inst->value.branch.cond;
            if(cond->value.tag == RVT_PHI) {
                auto load = InsertLoad((RawValue *)cond);
                cond = load;
                insts.insert(it,load);
            }
            break;
        }
        case RVT_CALL:{
            auto &params = inst->value.call.args;
            for(auto &param : params) {
                    if(param->value.tag == RVT_PHI) {
                        auto load = InsertLoad((RawValue *)param);
                        param = load;
                        insts.insert(it,load);
                    }
            }
            break;
        }
        case RVT_GET_ELEMENT: {
            auto &index = inst->value.getelement.index;
            if(index->value.tag == RVT_PHI) {
                auto load = InsertLoad((RawValue *)index);
                index = load;
                insts.insert(it,load);
            }
            break;
        }
        case RVT_GET_PTR: {
            auto &index = inst->value.getptr.index;
            if(index->value.tag == RVT_PHI) {
                auto load = InsertLoad((RawValue *)index);
                index = load;
                insts.insert(it,load);
            }
            break;
        }
        default:
            break;
    }
    }
}

void exitSSA(RawFunction * &func) {
    auto &bbs = func->basicblock;
    for(auto bb : bbs) {
        exitSSA(bb);
    }
    for(auto bb : bbs) {
        InsertLoad(bb);
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

//最后确定