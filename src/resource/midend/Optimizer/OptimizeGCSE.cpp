#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include "../../../include/midend/Optimizer/OptimizeGCSE.h"
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
using namespace std;
stack<unordered_set<RawValue *>> gcse;
unordered_set<RawValue *> expressions;
void ClearInst(RawFunction *&function);
bool IsExp(RawValue *value);
bool ExpCompare(RawValue *value1, RawValue *value2);
void ReplaceExp(RawValue *&use,RawValue *reg,RawValue *mem);

void global_exp_eliminate(RawValue *inst){
    for(auto expr : expressions) {//如果这个遍历到最后，就说明没有找到过
        if(expr->value.tag != inst->value.tag) continue;
        else {
            if(ExpCompare(expr,inst)) {
                inst->isDeleted = true;
                for(auto use : inst->usePoints) {
                    ReplaceExp(use,expr,inst);
                    expr->usePoints.push_back(use);
                }//这里只考虑使用的情况
                return;
            } else continue;
        }
    }
    expressions.insert(inst);//这里insert一定要保证进来的tag满足条件
}

void global_inst_eliminate(RawBasicBlock *block) {
    auto &insts = block->inst;
    for(auto inst : insts) {
        if(IsExp(inst))
            global_exp_eliminate(inst);
    }
}

void global_bb_eliminate(RawBasicBlock *bb) {
    gcse.push(expressions);
    global_inst_eliminate(bb);
    auto &domains = bb->domains;
    for(auto domain : domains) {
        global_bb_eliminate(domain);
    }
    expressions = gcse.top();
    gcse.pop();
}

void global_func_eliminate(RawFunction *function) {
    auto &bbs = function->basicblock;
    if(bbs.empty()) return;
    auto entrybb = *bbs.begin();
    global_bb_eliminate(entrybb);
}

void OptimizeGCSE(RawProgramme *programme) {
    auto &funcs = programme->funcs;
    auto &values = programme->values;
    for(auto func : funcs) {
        global_func_eliminate(func);
    }
    for(auto func : funcs) {
        ClearInst(func);
    }
}


//对于每个func建立一个set存储Exp,对于一个basicblock,先访问加入、替换，
//然后访问
