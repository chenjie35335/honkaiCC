#include "../../../include/midend/SSA/rename.h"
#include "../../../include/midend/IR/Programme.h"
#include "../../../include/midend/IR/Function.h"
#include "../../../include/midend/IR/BasicBlock.h"
#include "../../../include/midend/IR/Value.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cassert>
#include <queue>
#include <iostream>
#include <algorithm>
using namespace std;

void ClearInst(RawFunction *&function);

void ReplaceReg(RawValue *&use,RawValue *reg,RawValue *mem) {
    reg->usePoints.push_back(use);
    auto tag = use->value.tag;
    //cout << "replace tag : " << tag << endl;
    switch(tag) {
        case RVT_RETURN: {
            auto &src = use->value.ret.value;
            if(src == mem) src = reg;
            break;
        }
        case RVT_BINARY: {
            auto &lhs = use->value.binary.lhs;
            auto &rhs = use->value.binary.rhs;
            if(lhs == mem) lhs = reg;
            if(rhs == mem) rhs = reg;
            break;
        }
        case RVT_STORE: {
            auto &src = use->value.store.value;
            if(src == mem) {
                //cout << "store replace" << endl;
                src = reg;
            }
            break;
        }
        case RVT_BRANCH: {//branch这里会有额外的优化，不过目前先不考虑
            auto &cond = use->value.branch.cond;
            if(cond == mem) cond = reg;
            break;
        }
        case RVT_CALL:{
            auto &params = use->value.call.args;
            for(auto &param : params) {
                    if(param == mem) param = reg;
            }
            break;
        }
        case RVT_PHI: {
            auto &phis = use->value.phi.phi;
            for(auto &phi : phis) {
                if(phi.second == mem) phi.second = reg;
            }
        }
        case RVT_GET_ELEMENT: {
            auto &index = use->value.getelement.index;
            if(index == mem) index = reg;
            break;
        }
        case RVT_GET_PTR: {
            auto &index = use->value.getptr.index;
            if(index == mem) index = reg;
            break;
        }
        default:
            break;
    }
}

void renamePhi(RawValue *phi,RawValue * value,RawBasicBlock * frombb) {
    auto &phiElems = phi->value.phi.phi;
    phiElems.push_back(pair<RawBasicBlock *, RawValue *>(frombb,value));
}

// 这个函数用于处理inst的部分内容 
void renameValue(RawValue *&inst,unordered_set<RawValue *>&values,unordered_map<RawValue *,RawValue *>&IncommingValue) {
    auto &kind = inst->value.tag;
    switch(kind) {
        //这里就存在一个问题：如果不考虑alloc这里，那么如果这个值没初值，后面的重新弄就会访问空指针
        case RVT_PHI: {
            // cout << "handle phi" << endl;
            auto target = (RawValue *)inst->value.phi.target;
            IncommingValue[target] = inst;
            break;
        }//这是第一个循环，只考虑
        case RVT_LOAD: {
            // cout << "handle load" << endl;
            auto &load = inst->value.load;
            RawValue* src = (RawValue *)load.src;
            if(values.find(src) != values.end()) {
                inst->isDeleted = true;
                auto reg = IncommingValue[src];
                for(auto use : inst->usePoints) {
                    ReplaceReg(use,reg,inst);
                }
            }
            break;
        }
        case RVT_STORE: {
            // cout << "handle store" << endl;
            auto &store = inst->value.store;
            RawValue *dest = (RawValue *) store.dest;
            RawValue *src = (RawValue *) store.value;
            if(values.find(dest) != values.end()) {
            inst->isDeleted = true;
            IncommingValue[dest] = src; 
            }
            break;
        }
        default: break;
    }
}
//rename这里注意一个问题：phi函数那里我不需要一开始就填充，而是
void renameValue(RawBasicBlock *&bb,unordered_set<RawValue *>&values,unordered_map<RawValue *,RawValue *>&IncommingValue) {
    //cout << "rename BasicBlock " << bb->name << endl;
    auto& phis = bb->phi;
    auto& insts = bb->inst;
    if(!phis.empty()) {
    for(auto &phi : phis) {
        renameValue(phi,values,IncommingValue);
    }
    }
    if(!insts.empty()) {
    for(auto &inst : insts) {
        renameValue(inst,values,IncommingValue);
    }
    }
}
//对于全局变量和函数参数来说，肯定是第一个定义
void renameValue(RawFunction *&function) {
    auto &bbs = function->basicblock;
    auto &values = function->values;
    unordered_map <RawValue *,RawValue *> IncommingValue;
    if(bbs.empty()) return;
    IncommingValue.clear();
    queue<pair<RawBasicBlock *,unordered_map <RawValue *,RawValue *>>> W;
    unordered_set<RawBasicBlock *> VisitedSet;
    auto startbbPtr = *bbs.begin();
    W.push(pair<RawBasicBlock *,unordered_map <RawValue *,RawValue *>>(startbbPtr,IncommingValue));
    while(!W.empty()) {
        auto BB = W.front().first;
        auto IncommingTemp = W.front().second;
        W.pop();
        if(VisitedSet.find(BB) != VisitedSet.end())     continue;
        else     VisitedSet.insert(BB);
        //cout << "renameValueBB:" << BB->name << endl;
        renameValue(BB,values,IncommingTemp);
        for(auto fbb : BB->fbbs) {
            // cout << "handle following BB:" << fbb->name << endl;
            W.push(pair<RawBasicBlock *,unordered_map <RawValue *,RawValue *>>(fbb,IncommingTemp));
            for(auto phi : fbb->phi) {
                auto target = (RawValue *)phi->value.phi.target;
                auto Replace = IncommingTemp[target];
                if(Replace){
                renamePhi(phi,Replace,BB);
                Replace->usePoints.push_back(phi);
                }
            }
        }
    }
    for(auto bb : bbs) {
        auto &insts = bb->inst;
        insts.erase(remove_if(insts.begin(),insts.end(),[](RawValue *data){return (data->value.tag == RVT_ALLOC) && (data->identType == IDENT_VAR);}),insts.end());
    }
    for(auto value : values) {
        auto &sbbInst = startbbPtr->inst;
        sbbInst.push_front(value);
    }
} 

void renameValue(RawProgramme *& programme){
    auto &func = programme->funcs;
    for(auto FuncPtr : func) {
        renameValue(FuncPtr);
    }
    for(auto FuncPtr : func) {
        ClearInst(FuncPtr);
    }
}
//貌似这里还是不能用全局的，必须局部