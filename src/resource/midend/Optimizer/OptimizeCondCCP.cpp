#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include "../../../include/midend/Optimizer/OpimizeConditionCCP.h"
#include <algorithm>
#include <cassert>
using namespace std;
vector<RawValue *> Wv;
vector<RawBasicBlock *> Wb;
CondCCPTabl CCPController;
//现在来确定一下pass的顺序：
//我的想法是：死代码消除、条件传播、复写传播
//这里还是得感觉得这样两个一起：还是这样，先Wv后Wb，然后直到那张表彻底消除就可以
//对于那些不会执行的基本块，也就是多扫描了几遍而已
//如果这个地方我设置的是不可执行的该如何处理？
//如果不可执行，就不访问，可执行再对于该基本块中的变量进行处理
void ClearInst(RawFunction *&function);
int calculate(int operand1,int operand2,int tag);

void AddWorkerCCP(RawValue *&value) {
    Wv.push_back(value);
}

void MarkExc(RawBasicBlock *&bb) {
    bb->isExec = true;
}

void MarkTop(RawValue *value) {
    value->status = TOP;
}

void MarkVal(RawValue *value) {
    value->status = VAL;
}

void AddWorkerCCP(RawFunction *&func) {
    auto &bbs = func->basicblock;
    auto EntryBB = *bbs.begin();
    MarkExc(EntryBB);//条件2
    for(auto bb : bbs) {
        Wb.push_back(bb);
        auto &phis = bb->phi;
        auto &insts = bb->inst;
        for(auto phi : phis) {
            AddWorkerCCP(phi);
        }
        for(auto inst : insts) {
            AddWorkerCCP(inst);
        }
    }
}

void branchHandler(RawValue * &value) {
    auto &br = value->value.branch;
    auto cond = (RawValue *)br.cond;
    auto trueBB = (RawBasicBlock *) br.true_bb;
    auto falseBB = (RawBasicBlock *) br.false_bb;
    if(cond->status == VAL) {
        auto condValue = CCPController.LookValue(cond);
        if(condValue) {
            MarkExc(trueBB);
            Wb.push_back(trueBB);
            value->value.tag = RVT_JUMP;
            value->value.jump.target = trueBB;
        } else {
            MarkExc(falseBB);
            Wb.push_back(falseBB);
            value->value.tag = RVT_JUMP;
            value->value.jump.target = falseBB;
        }
    } else if(cond->status == TOP){
        MarkExc(trueBB);
        MarkExc(falseBB);
        Wb.push_back(trueBB);
        Wb.push_back(falseBB);
    } else return;
}

int evaluate(int32_t op,RawValueP lhs,RawValueP rhs) {
    auto lValue = CCPController.LookValue((RawValue *)lhs);
    auto rValue = CCPController.LookValue((RawValue *)rhs);
    return calculate(lValue,rValue,op);
}

void binaryHandler(RawValue *&value) {
    auto &binary = value->value.binary;
    auto lhs = binary.lhs;
    auto rhs = binary.rhs;
    auto op = binary.op;
    if(lhs->status == TOP || rhs->status == TOP) {
        MarkTop(value);
        Wv.push_back((RawValue *) value);
        return;
    } else if(lhs->status == VAL || rhs->status == VAL) {
        value->status = VAL;
        auto result = evaluate(op,lhs,rhs);
        CCPController.MidIntTable.insert(pair<RawValue *,int>(value,result));
        Wv.push_back((RawValue *) value);
    } else return;
}

void loadHandler(RawValue* &value) {
    auto &load  = value->value.load;
    auto src = load.src;
    if(src->value.tag == RVT_GLOBAL) {
        MarkTop(value);
        Wv.push_back((RawValue *) value);
        return;
    }
    if(src->status == TOP) MarkTop(value);
    else if(src->status == VAL) {
        MarkVal(value);
        auto SrcValue = CCPController.LookValue((RawValue *)src);
        CCPController.MidIntTable.insert(pair<RawValue *,int>(value,SrcValue));
        Wv.push_back((RawValue *) value);
    } else return;
}

void storeHandler(RawStore &store) {
    auto src = store.value;
    auto dest = store.dest;
    if(dest->value.tag == RVT_GLOBAL) return;
    if(src->status == TOP) {
        MarkTop((RawValue *)dest);
        Wv.push_back((RawValue *) dest);
    }
    else if(src->status == VAL) {
        MarkVal((RawValue *)dest);
        auto SrcValue = CCPController.LookValue((RawValue *)src);
        CCPController.MidIntTable.insert(pair<RawValue *,int>((RawValue *)dest,SrcValue));
        Wv.push_back((RawValue *)dest);
    } else return;
}

bool handleCond8(RawValue *&value){
    auto &phi = value->value.phi;
    auto &phiElements = phi.phi;
     for(auto phiElement : phiElements) {
     if(phiElement.second->status == TOP) {
         auto phiElembb = *phiElement.second->defbbs.begin();
         if(phiElembb->isExec) {
             MarkTop(value);
             Wv.push_back((RawValue *) value);
             return true;
         }
     }
 }
    return false;
}

void handleCond9(RawValue *&value) {
    auto &phi = value->value.phi;
    auto &phiElements = phi.phi;
    vector<int> ValData;
    for(auto phiElement : phiElements) {
        if(phiElement.second->status == VAL) {
            auto phiElembb = *phiElement.second->defbbs.begin();
                if(phiElembb->isExec) {
                    auto ElemValue = CCPController.LookValue(phiElement.second);
                    ValData.push_back(ElemValue);
                }
        }
    }
    if(std::all_of(ValData.begin(), ValData.end(), 
                 [&](int value) { return value == ValData[0]; })) {
                    MarkVal(value);
                    CCPController.MidIntTable.insert(pair<RawValue *,int>((RawValue *)value,ValData[0]));
                 } else {
                     MarkTop(value);
                     
                 }
                 Wv.push_back((RawValue *) value);
}

//phi函数这里也就条件8比较好判断
void phiHandler(RawValue *&value) {
    //条件8
    if(handleCond8(value)) return;
    //条件9
    handleCond9(value);
}

void Valuehandler(RawValue *&value) {
    auto tag = value->value.tag;
    switch(tag) {
        case RVT_BINARY:{
            binaryHandler(value);
            break;
        }
        case RVT_LOAD: {
            loadHandler(value);
            break;
        }
        case RVT_STORE: {
            auto &store = value->value.store;
            storeHandler(store);
            break;
        }
        case RVT_BRANCH: {
            branchHandler(value);
            break;
        }
        case RVT_CALL: {
            MarkTop(value);
            break;
        }
        case RVT_PHI: {
            phiHandler(value);
            break;
        }
        default: break;
        }
}

//对于变量来说，这里可以直接操作
//不用关心，因为只需要清空就行
void Handler(RawValue *&Sv) {
    if(Sv->status != VAL) return;
    auto &uses = Sv->usePoints;
    for(auto use : uses) {
        Valuehandler(use);
    }
}

void Handler(RawBasicBlock *&Sb) {
    if(!Sb->isExec) return;
    auto &fbbs = Sb->fbbs;
    if(fbbs.size() == 1) {
        for(auto fbb : fbbs) {
            Wb.push_back(fbb);
            fbb->isExec = true;
        }
    }//条件3
    auto &phis = Sb->phi;
    auto &insts = Sb->inst;
    for(auto phi : phis) {
        Valuehandler(phi);
    }
    for(auto inst : insts) {
        Valuehandler(inst);
    }
}
//现在这个算法其实不是让人很能理解：
//什么叫做或，两个中选一个？
//要是我的话就像这样
void CondCCPHandler() {
    while(!Wb.empty() || !Wv.empty()) {
        auto Sv = Wv.back();
        Wv.pop_back();//一个比较神奇的是，这个
        auto Sb = Wb.back();
        Wb.pop_back();
        Handler(Sv);
        Handler(Sb);
    }
}

void CondReplaceValue(RawValue *value) {
switch(value->value.tag) {
    case RVT_RETURN: {
        auto &src = value->value.ret.value;
        if(src->status == VAL) {
            auto SrcValue = CCPController.LookValue((RawValue *)src);
            src = CCPController.generateNumber(SrcValue);
        }
        break;
    }
    case RVT_BINARY:{
        if(value->status == VAL)
            value->isDeleted = true;
        else {
            auto &lhs = value->value.binary.lhs;
            auto &rhs = value->value.binary.rhs;
            if(lhs->status == VAL) {
                auto SrcValue = CCPController.LookValue((RawValue *)lhs);
                lhs = CCPController.generateNumber(SrcValue);
            }
            if(rhs->status == VAL) {
                auto SrcValue = CCPController.LookValue((RawValue *)rhs);
                rhs = CCPController.generateNumber(SrcValue);
            }
        }
        break;
    }
    case RVT_LOAD: {
        if(value->status == VAL)
            value->isDeleted = true;
        break;
    }
    case RVT_STORE: {
        auto &store = value->value.store;
        auto dest = (RawValue *)store.dest;
        if(dest->status == VAL) {
            dest->isDeleted = true;
        }
        break;
    }
    //branch这里还是直接修改比较好
    case RVT_CALL: {
        auto &call = value->value.call;
        auto &args = call.args;
        for(auto &arg : args) {
            if(arg->status == VAL) {
                auto SrcValue = CCPController.LookValue((RawValue *)arg);
                arg = CCPController.generateNumber(SrcValue);
            }
        }
        break;
    }
    default:
        return;

    }
}

void MarkDeletedValue(RawBasicBlock *&bb) {
    auto &phis = bb->phi;
    auto &insts = bb->inst;
    for(auto phi : phis) {
        CondReplaceValue(phi);
    }
    for(auto inst : insts) {
        CondReplaceValue(inst);
    }
}
//标记是否删除
//考虑一下jump和br的问题
//首先jump后面应该不会出现说不能执行的问题，除非本身该项目不能执行
//branch中的问题就比较容易了，但是要记录的一个问题就是fbbs
void MarkDeletedBB(RawProgramme *&programme) {
    auto &funcs = programme->funcs;
    for(auto func : funcs) {
        auto &bbs = func->basicblock;
        for(auto bb : bbs) {
            if(!bb->isExec) {
                bb->isDeleted = true;
                auto &fbbs = bb->fbbs;
                auto &pbbs = bb->pbbs;
                for(auto fbb : fbbs) {
                    fbb->pbbs.remove(bb);
                }
                for(auto pbb : pbbs) {
                    pbb->fbbs.remove(bb);
                }
            } else {
                MarkDeletedValue(bb);
            }
        }
    }
}

void CondCCP(RawProgramme *&programme) {
    auto &values = programme->values;
    auto &funcs = programme->funcs;
    for(auto value : values){
    AddWorkerCCP(value);
    value->status = TOP;//条件1
    }
    for(auto func : funcs)
    AddWorkerCCP(func);
    CondCCPHandler();
    MarkDeletedBB(programme);
    for(auto func : funcs) {
        ClearInst(func);
    }
}