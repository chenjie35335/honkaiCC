#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
using namespace std;
/// 这个还是比较恶心！
void ClearInst(RawFunction *&function) {
    auto &bbs = function->basicblock;
    bbs.erase(remove_if(bbs.begin(), bbs.end(), [](RawBasicBlock *bb) {
    return bb->isDeleted == true;
}), bbs.end());
    for(auto bb : bbs) {
        auto &insts = bb->inst;
        auto &phis = bb->phi;
        insts.erase(remove_if(insts.begin(), insts.end(), [](RawValue *data) {
        return data->isDeleted == true;
    }), insts.end());
        phis.erase(remove_if(phis.begin(), phis.end(), [](RawValue *data) {
    return data->isDeleted == true;
}), phis.end());
    }
}


bool hasImpact(RawValue *&S) {
    auto tag = S->value.tag;
    switch(tag) {
        case RVT_CALL:{
            return true;
        }
        case RVT_STORE: {
            auto dest = S->value.store.dest;
            if(dest->value.tag == RVT_GLOBAL) return true;
        }
        default: return false;
    }
    
}

void DCE_use(RawValue *&S, vector<RawValue *> &W){
    auto tag = S->value.tag;
    switch(tag) {
case RVT_BINARY: {
    auto src1 = (RawValue *)S->value.binary.lhs;
    auto src2 = (RawValue *)S->value.binary.rhs;
    if(src1->value.tag != RVT_INTEGER) {
    src1->usePoints.remove(S);
    W.push_back(src1);
    }
    if(src2->value.tag != RVT_INTEGER){
    src2->usePoints.remove(S);
    W.push_back(src2);
    }
    break;
}
case RVT_LOAD: {
    auto src = (RawValue *)S->value.load.src;
    src->usePoints.remove(S);
    break;
}
case RVT_STORE: {
    auto src = (RawValue *)S->value.store.value;
    if(src->value.tag != RVT_INTEGER) {
    src->usePoints.remove(S);
    W.push_back(src);
    }
    break;
}
case RVT_PHI: {
    auto &phis = S->value.phi.phi;
    for(auto phi : phis) {
        phi.second->usePoints.remove(S);
    }
    break;
}
default:
    assert(false);
}

}

void DCE(RawFunction *&function) {//目前来看这里没有办法直接删除，还是得遍历一遍后删除
    if(function->basicblock.size() == 0) return;
    vector<RawValue *> W;
    auto &values = function->values;
    for(auto value : values) {
        auto valueTag = value->value.tag;
        if(valueTag == RVT_ALLOC) {
            auto &copies = value->copiesValues;
            for(auto copy : copies) {
                // if(copy->value.tag == RVT_VALUECOPY) {
                // auto target = copy->value.valueCop.target;
                // cout << target->name << "phi" << endl;
                // } else if(copy->value.tag == RVT_PHI) {
                //     auto target = copy->value.phi.target;
                //     cout << target->name << "copy" << endl;
                // }
                W.push_back(copy);
            }
        }//这里只放入所有alloc类型的值，然后其相关的再删掉
    }
    while(!W.empty()) {
        auto value = W.back();
        W.pop_back();
        if(value->usePoints.empty()) {
            auto S = *value->defPoints.begin();//对于SSA来说，这个应该只有一个值
            if(S && !hasImpact(S)) {
            S->isDeleted = true;
            DCE_use(S,W);
            }
        }
    }
    ClearInst(function);
}

void OptimizeDCE(RawProgramme *&programme) {
    auto &funcs = programme->funcs;
    for(auto func : funcs) {
        DCE(func);
    }
}