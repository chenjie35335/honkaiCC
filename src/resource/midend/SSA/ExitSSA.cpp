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

void exitSSALoad(RawLoad &load) {
    //cerr << "enter exitSSALoad" << endl;
    auto src = (RawValueP) load.src;
    switch(src->value.tag) {
        case RVT_PHI: {
            load.src = src->value.phi.target;
            break;
        }
        case RVT_VALUECOPY: {
            load.src = src->value.valueCop.target;
            break;
        }
        default: break;
    }
}

void exitSSAStore(RawStore &store) {
    auto dest = (RawValueP) store.dest;
    switch(dest->value.tag) {
        case RVT_PHI: {
            store.dest = dest->value.phi.target;
            break;
        }
        case RVT_VALUECOPY: {
            store.dest = dest->value.valueCop.target;
            break;
        }
        default: break;
    }
}


void exitSSA(RawValue* &value) {
    auto& kind = value->value;
    switch(kind.tag) {
        case RVT_LOAD: {
            auto &load = value->value.load;
            exitSSALoad(load);
            break;
        }
        case RVT_STORE: {
            auto &store = value->value.store;
            exitSSAStore(store);
            break;
        }
        default: break;
    }
}

void exitSSA(const RawBasicBlockP &bb) {
    auto &insts = bb->inst;
    for(auto inst: insts) {
    exitSSA(inst);
    }
}

void exitSSA(const RawFunctionP &func) {
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