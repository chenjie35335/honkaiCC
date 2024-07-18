#include "../../../include/midend/IR/Function.h"
#include "../../../include/midend/IR/BasicBlock.h"
#include "../../../include/midend/SSA/PHI.h"
#include "../../../include/midend/IR/Programme.h"
#include <assert.h>
#include <stack>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
using namespace std;
//这里需要考虑的是，如果我使用的是基本块常量的话，该如何处理
void AddPhi(RawBasicBlock *&bb,RawValue *&data) {
    //cout << " insert value " << data->name << " into block " << bb->name << endl;
    auto &phis = bb->phi;
    RawValue* phi = new RawValue();
    phi->value.tag = RVT_PHI;
    phi->value.phi.target = data;
    phis.push_back(phi);
    data->copiesValues.push_back(phi);
    phi->defPoints.push_back(phi);
    phi->defbbs.push_back(bb);
    bb->defs.insert(data);
}

// 这里该怎么处理这个defsite集合？
// 首先使用hash表那样弄不一定是一件好事
void AddPhi(RawFunction *& func) {
    auto &bbs = func->basicblock;
    auto &values = func->values;
    values.clear();
    for(RawBasicBlock *bb : bbs) {
        auto &bbDef = bb->defs;
        for(RawValue* value : bbDef) {
            if(value->value.tag == RVT_ALLOC && value->identType == IDENT_VAR){
            value->defbbs.push_back(bb);
            values.insert(value);
            }
        }
    }
    // for(RawValue *value : values) {
    //     cout << func->name << " use value as follows: " << value->value.tag << endl;
    // }
    for(RawValue *value : values) {
        //cout << "handle value " << value->name << endl;
        vector<RawBasicBlock *> W;
        W = value->defbbs;
        //copy(value->defbbs.begin(),value->defbbs.end(),W.begin());
        while(!W.empty()) {
            RawBasicBlock *n = W.back();
            W.pop_back();
            //computeDF(n);
            for(const RawBasicBlock *Y : n->df) {
                RawBasicBlock* y = (RawBasicBlock *) Y;
                if(y->NessPhi.find(value) == y->NessPhi.end()) {
                    AddPhi(y,value);
                    y->NessPhi.insert((RawValueP)value);
                    if(find(value->defbbs.begin(),value->defbbs.end(),y) == value->defbbs.end()) {
                        W.push_back(y);
                    }
                }
            }
        }
    }
}

void AddPhi(RawProgramme *& programme) {
    auto &funcs = programme->funcs;
    //cerr << "func.len:" << funcs.len << endl;
    for(RawFunction* func : funcs) {
        AddPhi(func);
    }
}