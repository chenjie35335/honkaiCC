//局部公共子表达式删除
#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include "../../../include/midend/Optimizer/OptimizeLCSE.h"
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
#include <unordered_set>
#include <queue>
LCSE_Builder lcse;
void ClearInst(RawFunction *&function);

void ReplaceExp(RawValue *&use,RawValue *reg,RawValue *mem) {
    reg->usePoints.push_back(use);
    auto tag = use->value.tag;
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
        case RVT_LOAD: {
            auto &src = use->value.load.src;
            if(src == mem) src = reg;
            break;
        }
        case RVT_STORE: {
            auto &src = use->value.store.value;
            auto &dest = use->value.store.dest;
            if(src == mem)  src = reg;
            if(dest == mem) dest = reg;
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
            auto &src = use->value.getelement.src;
            if(index == mem) index = reg;
            if(src == mem)  src = reg;
            break;
        }
        case RVT_GET_PTR: {
            auto &index = use->value.getptr.index;
            auto &src = use->value.getptr.src;
            if(index == mem) index = reg;
            if(src == mem)  src = reg;
            break;
        }
        default:
            break;
    }
}


bool IsExp(RawValue *value) {
    auto tag = value->value.tag;
    switch(tag) {
        case RVT_BINARY: 
        case RVT_GET_ELEMENT:
        case RVT_GET_PTR:
            return true;
        default: 
            return false;
        
    }
}

bool ValueCompare(RawValue *value1, RawValue *value2) {
    if(value1 == value2) return true;
    else {
        auto tag1 = value1->value.tag;
        auto tag2 = value2->value.tag;
        if(tag1 == tag2){
            if(tag1 == RVT_INTEGER) {
                return value1->value.integer.value == value2->value.integer.value;
            } else if(tag1 == RVT_FLOAT) {
                return value1->value.floatNumber.value== value2->value.floatNumber.value;
            } 
        } 
    }
    return false;
}

bool ExpCompare(RawValue *value1, RawValue *value2) {
    auto tag = value1->value.tag;
    switch(tag) {
        case RVT_BINARY: {
            auto lhs1 = (RawValue *)value1->value.binary.lhs;
            auto rhs1 = (RawValue *)value1->value.binary.rhs;
            auto lhs2 = (RawValue *)value2->value.binary.lhs;
            auto rhs2 = (RawValue *)value2->value.binary.rhs;
            auto op1 = value1->value.binary.op;
            auto op2 = value2->value.binary.op;
            if(op1 != op2) return false;
            else if(!ValueCompare(lhs1,lhs2) || !ValueCompare(rhs1,rhs2)) {
                //cout << "op: " << op1 << endl;
                return false;
            }
            else return true;
            break;
        }
        case RVT_GET_ELEMENT: {
            auto src1   = (RawValue *)value1->value.getelement.src;
            auto src2   = (RawValue *)value2->value.getelement.src;
            auto index1 = (RawValue *)value1->value.getelement.index;
            auto index2 = (RawValue *)value2->value.getelement.index;
            if(src1 == src2 && ValueCompare(index1,index2)) return true;
            else return false;
            break;
        }
        case RVT_GET_PTR: {
            auto src1   = (RawValue *)value1->value.getptr.src;
            auto src2   = (RawValue *)value2->value.getptr.src;
            auto index1 = (RawValue *)value1->value.getptr.index;
            auto index2 = (RawValue *)value2->value.getptr.index;
            if(src1 == src2 && ValueCompare(index1,index2)) return true;
            else return false;
            break;
        }
        default: {
            return false;
        }
    }
}

void exp_eliminate(RawValue *inst) {
    for(auto expr : lcse.Expressions) {//如果这个遍历到最后，就说明没有找到过
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
    //最后加入到gcse当中
    lcse.Expressions.insert(inst);//这里insert一定要保证进来的tag满足条件
}

//这个函数我们和学长的很不一样，需要重新设计
void inst_eliminate(RawBasicBlock *block) {
    lcse.Expressions.clear();
    auto &insts = block->inst;//对于phi函数来说不需要替换
    for(auto inst : insts) {          //这里我们不同的地方是我们可以直接比较内存，不需要重新编号
        if(IsExp(inst))
            exp_eliminate(inst);
    }
}

void func_eliminate(RawFunction *func) {
    auto &bbs = func->basicblock;
    if(bbs.empty()) return;
    unordered_set<RawBasicBlock *>visited;
    queue<RawBasicBlock *> q;
    q.push(*bbs.begin());
    while(!q.empty()){
        RawBasicBlock * now=q.front();
        q.pop();
        inst_eliminate(now);
        for(auto nowfbb : now->fbbs) {
            if(visited.find(nowfbb)==visited.end()) {
                q.push(nowfbb);
                visited.insert(nowfbb);
            }
        }
    }
}

void OptimizeLCSE(RawProgramme *programme) {
    auto &funcs = programme->funcs;
    auto &values = programme->values;
    for(auto func : funcs) {
        func_eliminate(func);
    }
    for(auto func : funcs) {
        ClearInst(func);
    }
}
