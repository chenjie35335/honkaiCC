#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include "../../../include/midend/Optimizer/OptimizeConstCombine.h"
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
using namespace std;
vector<RawValue *> W;
Combine combineController;

void ClearInst(RawFunction *&function);

int calculate(int operand1,int operand2,int tag) {
switch(tag) {
case RBO_ADD:
    {
    return operand1 + operand2;
    break;
    }
case RBO_SUB:
    {
    return operand1 - operand2;
    break;
    }
case RBO_EQ:
    {
    return operand1 == operand2;
    break;
    }
case RBO_NOT_EQ:
    {
    return operand1 != operand2;
    break;
    }
case RBO_MUL:
    {
    return operand1 * operand2;
    break;
    }
case RBO_DIV:
    {
    return operand1 / operand2;
    break;
    }
case RBO_MOD:
    {
    return operand1 % operand2;
    break;
    }
case RBO_LT:
    {
    return operand1 < operand2;
    break;
    }
case RBO_GT:
    {
    return operand1 > operand2;
    break;
    }
case RBO_GE:
    {
    return operand1 >= operand2;
    break;
    }
case RBO_LE:
    {
    return operand1 <= operand2;
    break;
    }   
case RBO_OR:
    {
    return operand1 | operand2;
    break;
    }
case RBO_XOR:{
    return operand1 ^ operand2;
    break;
}
case RBO_AND:
    {
    return operand1 & operand2;
    }
default:
    assert(0);
    }
}

void ReplaceValue(RawValue *&use) {
    auto tag = use->value.tag;
    switch(tag) {
        case RVT_RETURN: {
            auto &src = use->value.ret.value;
            if(combineController.find((RawValue *)src)) {
                auto retValue =  combineController.LookValue((RawValue *)src);
                src = combineController.generateNumber(retValue);
            }
            break;
        }
        case RVT_BINARY: {
            auto &lhs = use->value.binary.lhs;
            auto &rhs = use->value.binary.rhs;
            if(combineController.find((RawValue *)lhs)) {
                auto retValue =  combineController.LookValue((RawValue *)lhs);
                lhs = combineController.generateNumber(retValue);
            }
            if(combineController.find((RawValue *)rhs)) {
                auto retValue =  combineController.LookValue((RawValue *)rhs);
                rhs = combineController.generateNumber(retValue);
            }
            break;
        }
        case RVT_LOAD: {
            auto &src = use->value.load.src;
            if(combineController.find((RawValue *)src)) {
                auto retValue =  combineController.LookValue((RawValue *)src);
                src = combineController.generateNumber(retValue);
            }
            break;
        }
        case RVT_STORE: {
            auto &src = use->value.store.value;
            if(combineController.find((RawValue *)src)) {
                auto retValue =  combineController.LookValue((RawValue *)src);
                src = combineController.generateNumber(retValue);
            }
            break;
        }
        case RVT_BRANCH: {//branch这里会有额外的优化，不过目前先不考虑
            auto &cond = use->value.branch.cond;
            if(combineController.find((RawValue *)cond)) {
                auto retValue =  combineController.LookValue((RawValue *)cond);
                cond = combineController.generateNumber(retValue);
            }
            break;
        }
        case RVT_CALL:{
            auto &params = use->value.call.args;
            for(auto &param : params) {
                if(combineController.find((RawValue *)param)) {
                    auto retValue =  combineController.LookValue((RawValue *)param);
                    param = combineController.generateNumber(retValue);
                }
            }
            break;
        }
        default:
            break;
    }
}

void ReplaceUse(RawValue *&data){
    auto &uses = data->usePoints;
    for(auto use : uses) {
        ReplaceValue(use);
        W.push_back(use);
    }
}

void ReplaceUse(RawValue *&data,RawValue *&S) {
    bool ToRemove = true;
    auto &uses = data->usePoints;
    for(auto use : uses) {
        if(use->value.tag == RVT_PHI) ToRemove = false;
        ReplaceValue(use);
        W.push_back(use);
    }
    S->isDeleted = ToRemove;
}

void AddWorker(RawValue *&data) {
    W.push_back(data);
}

void AddWorker(RawFunction *&function) {
    auto &bbs = function->basicblock;
    for(auto bb : bbs) {
        auto &phis = bb->phi;
        auto &insts = bb->inst;
        for(auto phi : phis) {
            W.push_back(phi);
        }
        for(auto inst : insts) {
            W.push_back(inst);
        }
    }
}

void CombineBinary(RawBinary &S,RawValue *&data) {
    auto tag = S.op;
    auto operandKind1 = S.lhs->value.tag;
    auto operandKind2 = S.rhs->value.tag;
    if(operandKind1 != RVT_INTEGER || operandKind2 != RVT_INTEGER) return;
    auto operand1 = S.lhs->value.integer.value;
    auto operand2 = S.rhs->value.integer.value;
    auto value = calculate(operand1,operand2,tag);
    data->isDeleted = true;
    combineController.AddIntValue(data,value);
    ReplaceUse(data);
}

void ReplaceVar(RawValue *&use, RawValue *& target,RawValue *S) {
    auto tag = use->value.tag;
    if(tag == RVT_LOAD) {
        auto &src = use->value.load.src;
        src = target;
    } else if(tag == RVT_PHI) {
        auto &phis = use->value.phi.phi;
        for(auto &phi : phis) {
            if(phi.second == S) phi.second = target;
        }
    } else assert(0);
}
//现在的主要问题就是这里，我在思考这里应该如何处理
//首先可以先做一件事情：如果一个值的使用中出现了phi语句的话，这个定义首先不能删掉
//当然如果一个值没有出现在phi语句中则可以删除
//然后等到处理phi语句时候，如果phi语句可以删除然后使用传播算法，再进行删除也不迟
void CombinePhi(RawValue *& S) {//可以先实现复写传播的第一部分：phi函数
    auto &phis = S->value.phi.phi;
    if(phis.size() <= 0 )return;
    //复写传播
    if(phis.size() == 1) {
        auto phi = (*phis.begin()).second;
        phi->usePoints.remove(S);
        S->isDeleted = true;
        for(auto use : S->usePoints) {
            ReplaceVar(use,phi,S);
        }
    }
    vector<int> phiValues;
    for(auto phi : phis) {
        if(combineController.find(phi.second))
            phiValues.push_back(combineController.LookValue(phi.second));
        else return;
    }//这里应该首先删除掉所有的定植语句
    if(std::all_of(phiValues.begin(), phiValues.end(), 
                     [&](int value) { return value == phiValues[0]; })) {
        for(auto phi: phis) {
            auto &defPoints = phi.second->defPoints;
            for(auto defPoint : defPoints) {
                defPoint->isDeleted = true;
            }
        }
        S->isDeleted = true;
        combineController.AddIntValue(S,phiValues[0]);
        ReplaceUse(S);
    }
}

void CombineStore(RawStore &S,RawValue *&data) {
    auto src = (RawValue *)S.value;
    auto SrcTag = src->value.tag;
    auto dest = (RawValue *)S.dest;
    auto DestTag = dest->value.tag;
    if(SrcTag != RVT_INTEGER) return;
    if(DestTag != RVT_ALLOC && DestTag != RVT_VALUECOPY) return;
    auto value = src->value.integer.value;
    data->isDeleted = false;
    combineController.AddIntValue(dest,value);
    ReplaceUse(dest,data);
}

void CombineLoad(RawLoad &S,RawValue *&data) {
    auto tag = S.src->value.tag;
    if(tag != RVT_INTEGER) return;
    auto value = S.src->value.integer.value;
    data->isDeleted = true;
    combineController.AddIntValue(data,value);
    ReplaceUse(data);
}

/*
    对于load和store来说，需要干的事情有，load的话只能委屈说让src成为integer类型了，但是在这里遍历的时候，
    也相当于给临时变量赋值，store的话照旧
*/
void CombineAssign(RawValue *& S) {
    auto tag = S->value.tag;
    switch(tag) {
        case RVT_BINARY: {
            auto &binary = S->value.binary;
            CombineBinary(binary,S);
            break;
        }
        case RVT_LOAD: {
            auto &load = S->value.load;
            CombineLoad(load,S);
            break;
        }
        case RVT_STORE: {
            auto &store = S->value.store;
            CombineStore(store,S);
            break;
        }//对于call等其他的类型，在其他优化的地方处理
        default: break;
    }
}

void ConstCombine()
{
    while(!W.empty()) {
        auto S = W.back();
        W.pop_back();
        auto tag = S->value.tag;
        if(tag == RVT_PHI) {
            CombinePhi(S);
        } else {
            CombineAssign(S);
        }
}
}

void OptimizeConstCombine(RawProgramme *&programme) {
    auto &values = programme->values;
    auto &funcs = programme->funcs;
    for(auto value : values)
    AddWorker(value);
    for(auto func : funcs)
    AddWorker(func);
    ConstCombine();
    for(auto func : funcs) {
        ClearInst(func);
    }
}