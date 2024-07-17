#include "../../../include/midend/SSA/rename.h"
#include "../../../include/midend/IR/Programme.h"
#include "../../../include/midend/IR/Function.h"
#include "../../../include/midend/IR/BasicBlock.h"
#include "../../../include/midend/IR/Value.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cassert>
#include <iostream>
using namespace std;
void renamePhi(RawBasicBlock * &bb,RawBasicBlock * frombb) {
    auto &phis = bb->phi;
    for(auto phi : phis) {
        assert(phi->value.tag == RVT_PHI);
        auto &phiData = phi->value.phi;
        RawValue *target = (RawValue *)phiData.target;
        if(!target->tempCopy.empty()) {
        RawValue* NewName = (RawValue *)target->tempCopy.top();
        auto &phiElements = phiData.phi;
        phiElements.push_back(pair<RawBasicBlock *, RawValue *>(frombb,NewName)) ;
        NewName->usePoints.push_back(phi);
        }
    }
}

// 这个函数用于处理inst的部分内容 
void renameValue(RawValue *&inst) {
    auto &kind = inst->value.tag;
    switch(kind) {
        //这里就存在一个问题：如果不考虑alloc这里，那么如果这个值没初值，后面的重新弄就会访问空指针
        case RVT_PHI: {
            auto &phi = inst->value.phi;
            auto target = (RawValue *) phi.target;
            //cout << "push " << target->name << endl;
            target->tempCopy.push (inst);
            break;
        }//这是第一个循环，只考虑
        case RVT_ALLOC: {
            inst->tempCopy.push(inst);
            break;
        }
        case RVT_LOAD: {
            auto &load = inst->value.load;
            RawValue* src = (RawValue *)load.src;
            auto SrcTag = src->value.tag;
            if(src->identType != IDENT_VAR) break;
            if(SrcTag == RVT_ALLOC || SrcTag == RVT_VALUECOPY) {
            load.src = src->tempCopy.top();
            src->tempCopy.top()->usePoints.push_back(inst);
            }
            break;
        }
        case RVT_STORE: {
            auto &store = inst->value.store;
            RawValue *dest = (RawValue *) store.dest;
            auto DestTag = dest->value.tag;
            if(dest->identType != IDENT_VAR) break;
            if(DestTag == RVT_ALLOC || DestTag == RVT_VALUECOPY) {
            RawValue *copy = new RawValue();
            copy->value.tag = RVT_VALUECOPY;
            copy->value.valueCop.target = dest;
            copy->identType = IDENT_VAR;
            //cout << "push " << copy->value.valueCop.target->name << endl;
            dest->tempCopy.push (copy);
            dest->copiesValues.push_back(copy);
            copy->defPoints.push_back(inst);
            store.dest = copy;

            }
            break;
        }
        default: break;
    }
}
//rename这里注意一个问题：phi函数那里我不需要一开始就填充，而是
void renameValue(RawBasicBlock *&bb) {
    //cout << "rename BasicBlock " << bb->name << endl;
    auto& phis = bb->phi;
    auto& insts = bb->inst;
    auto& fbbs = bb->fbbs;
    auto& domains = bb->domains;
    auto& defsites = bb->defs;
    for(auto PhiPtr : phis) {
        renameValue(PhiPtr);
    }
    for(auto InstPtr : insts) {
        renameValue(InstPtr);
    }
    for(auto Fbbptr : fbbs) {
        //cout << "rename SubBasicBlock " << Fbbptr->name << endl;
        renamePhi(Fbbptr,bb);
    }
    for(auto DomainPtr : domains) {
        renameValue(DomainPtr);
    }
    for(auto def : defsites) {
        //cout << "top " << def->tempCopy.top()->name << endl;
        def->tempCopy.pop();
        //cout << "pop top " << def->name << endl;
    }
}
//对于全局变量和函数参数来说，肯定是第一个定义
void renameValue(RawFunction *&function) {
    auto &bbs = function->basicblock;
    auto &params = function->params;
    for(auto ParamPtr : params) {
        ParamPtr->tempCopy.push(ParamPtr);
    }
        if(bbs.size() == 0) return;
        auto startbbPtr = *bbs.begin();
        renameValue(startbbPtr);
} 

void renameValue(RawProgramme *& programme){
    auto &func = programme->funcs;
    auto &values = programme->values;
    for(auto ValuePtr: values) {
        ValuePtr->tempCopy.push(ValuePtr);
    }
    for(auto FuncPtr : func) {
        renameValue(FuncPtr);
    }
}