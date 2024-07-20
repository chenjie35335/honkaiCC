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

bool IsVal(RawValue *v) {
    if(v->status == VAL) return true;
    if(v->value.tag == RVT_INTEGER) return true;
    return false;
}

bool IsTop(RawValue *v) {
    if(v->status == TOP) return true;
    return false;
}

bool IsBot(RawValue *v) {
    if(v->status == BOT) return true;
    else return false;
}

void MarkTop(RawValue *value) {
    value->status = TOP;
}

void MarkVal(RawValue *value) {
    value->status = VAL;
}

void AddWorkerCCP(RawFunction *&func) {
    
    auto &bbs = func->basicblock;
    if(bbs.empty()) return;
    Wb.clear();
    Wv.clear();
    auto EntryBB = *bbs.begin();
    MarkExc(EntryBB);//条件2
    Wb.push_back(EntryBB);
    // cout << "Addworker push bb" << EntryBB->name << endl;
}

void branchHandler(RawValue * &value) {
    // cout << "branch handle" << endl;
    auto &br = value->value.branch;
    auto cond = (RawValue *)br.cond;
    auto trueBB = (RawBasicBlock *) br.true_bb;
    auto falseBB = (RawBasicBlock *) br.false_bb;
    if(IsVal(cond)) {
        auto condValue = CCPController.LookValue(cond);
        if(condValue) {
            if(!trueBB->isExec){
                MarkExc(trueBB);
                Wb.push_back(trueBB);
                // cout << "cond true push bb " << trueBB->name << endl;
                for(auto trueFBB : trueBB->fbbs) {
                    if(trueFBB->isExec) {
                        Wb.push_back(trueFBB);
                        // cout << "cond true f push bb " << trueFBB->name << endl;
                    }
                }
            }
        } else {
            if(!falseBB->isExec){
                MarkExc(falseBB);
                Wb.push_back(falseBB);
                // cout << "cond false push bb " << falseBB->name << endl;
                for(auto falseFBB : falseBB->fbbs) {
                    if(falseFBB->isExec) {
                        Wb.push_back(falseFBB);
                        // cout << "cond false f push bb " << falseFBB->name << endl;
                    }

                }
            }
        }
    } else {
        if(!trueBB->isExec){
                MarkExc(trueBB);
                Wb.push_back(trueBB);
                // cout << "cond u true push bb " << trueBB->name << endl;
                for(auto trueFBB : trueBB->fbbs) {
                    if(trueFBB->isExec) {
                        Wb.push_back(trueFBB);
                        // cout << "cond u true f push bb " << trueFBB->name << endl;
                    }
                }
        }
        if(!falseBB->isExec){
                MarkExc(falseBB);
                Wb.push_back(falseBB);
                // cout << "cond u false push bb " << falseBB->name << endl;
                for(auto falseFBB : falseBB->fbbs) {
                    if(falseFBB->isExec) {
                        Wb.push_back(falseFBB);
                        // cout << "cond u false push bb " << falseFBB->name << endl;
                    }
                }
            }
    }
}

int evaluate(int32_t op,RawValueP lhs,RawValueP rhs) {
    auto lValue = CCPController.LookValue((RawValue *)lhs);
    auto rValue = CCPController.LookValue((RawValue *)rhs);
    return calculate(lValue,rValue,op);
}

void binaryHandler(RawValue *&value) {
    auto &binary = value->value.binary;
    auto lhs = (RawValue *)binary.lhs;
    auto rhs = (RawValue *)binary.rhs;
    auto op = binary.op;
    if(IsVal(lhs) && IsVal(rhs)) {
        // cout << "binary val" << endl;
        if(value->status == BOT) Wv.push_back(value);
        value->status = VAL;
        auto result = evaluate(op,lhs,rhs);
        CCPController.MidIntTable.insert(pair<RawValue *,int>(value,result));
    } else if(IsTop(lhs) || IsTop(rhs)) {
        // cout << "binary top" << endl;
        if(IsVal(value) || IsBot(value)) {
            Wv.push_back(value);
        }
        value->status = TOP;
    } else if(IsBot(lhs) || IsBot(rhs)) {
        if(IsBot(value) || IsVal(value)) Wv.push_back(value);
        value->status = TOP;
    }
}

void loadHandler(RawValue* &value) {//在mem2reg的情况下不会发生这样的事情，因此load我可以认为直接判为TOP
    auto &load  = value->value.load;
    if(IsBot(value)) Wv.push_back(value);
    value->status = TOP;
}

void storeHandler(RawStore &store) {
    auto src = store.value;
    auto dest = store.dest;
    return;
}

bool handleCond8(RawValue *&value){
    auto &phi = value->value.phi;
    auto &phiElements = phi.phi;
    // cout << "handleCond8: " << endl;
     for(auto phiElement : phiElements) {
     if(IsTop(phiElement.second) && phiElement.first->isExec) {
            if(IsBot(value) || IsVal(value))
                Wv.push_back((RawValue *) value);
            MarkTop(value);
            // cout << "cond 8 return true" << endl;
             return true;
         
     }
 }
    // cout << "cond 8 return false" << endl;
    return false;
}

void handleCond9(RawValue *&value) {
    auto &phi = value->value.phi;
    auto &phiElements = phi.phi;
    // cout << "handle phi target: " << phi.target->name << " status: " << value->status << endl;
    vector<int> ValData;
    // cout << "begin to handleCon9" << endl;
    for(auto phiElement : phiElements) {
        // cout << "handle ones" << endl;
        auto phiElembb = phiElement.first;
        // cout << "phi bb:" << phiElembb->name << " is Exec? " << phiElembb->isExec << endl;
        // cout << "phi value: " << phiElement.second->status << endl;
        if(IsVal(phiElement.second) && phiElembb->isExec) {
             auto ElemValue = CCPController.LookValue(phiElement.second);
            //  cout << "push back: " << ElemValue << endl;
             ValData.push_back(ElemValue);
        }
    }
    if(ValData.empty()) {
            
            return;
    }
    if(std::all_of(ValData.begin(), ValData.end(), 
                 [&](int value) { return value == ValData[0]; })) {
                    //  cout << "Mark Value" << endl;
                     if(IsBot(value)) Wv.push_back((RawValue *) value);
                    MarkVal(value);
                    CCPController.MidIntTable.insert(pair<RawValue *,int>((RawValue *)value,ValData[0]));
                    // cout << "Insert success" << endl;
                 } else {
                    //  cout << "Mark Top" << endl;
                     if(IsBot(value) || IsVal(value)){
                        Wv.push_back(value);
                     }
                     MarkTop(value);
                 }
                 
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
            // cout <<" value binary" << endl;
            binaryHandler(value);
            break;
        }
        case RVT_LOAD: {
            // cout <<" value load" << endl;
            loadHandler(value);
            break;
        }
        case RVT_STORE: {
            // cout <<" value store" << endl;
            auto &store = value->value.store;
            storeHandler(store);
            break;
        }
        case RVT_BRANCH: {
            // cout <<" value branch" << endl;
            branchHandler(value);
            break;
        }
        case RVT_CALL: {
            // cout <<" value call" << endl;
            if(IsBot(value) || IsVal(value)) Wv.push_back(value);
            MarkTop(value);
            break;
        }
        case RVT_PHI: {
            // cout <<" value phi" << endl;
            phiHandler(value);
            break;
        }
        default: {
            // cout << "unhandled value type: " << tag << endl;
            break;
        }
        }
}

//对于变量来说，这里可以直接操作
//不用关心，因为只需要清空就行
void Handler(RawValue *&Sv) {
    // cout << "begin handle value" << endl;
    auto &uses = Sv->usePoints;
    // cout << "begin handle value tag: " << Sv->value.tag << endl;
    for(auto use : uses) {
        // cout << "begin handle use tag: " << use->value.tag << endl;
        Valuehandler(use);
    }
    // cout << "end handle value" << endl;
}

void Handler(RawBasicBlock *&Sb) {
    // cout << "begin handle bb: " << Sb->name << endl;
    auto &fbbs = Sb->fbbs;
    //条件3
    if(Sb->isExec == true && fbbs.size() == 1) {
        auto fbb = fbbs.front();
        if(!fbb->isExec) {
            fbb->isExec = true;
            Wb.push_back(fbb);
            // cout << "3 push bb " << fbb->name << endl;
            for(auto ffbb : fbb->fbbs) {
                if(ffbb->isExec == true) {
                    Wb.push_back(ffbb);
                    // cout << "3 f push bb " << ffbb->name << endl;
                }
            }
        }
    }
    auto &phis = Sb->phi;
    auto &insts = Sb->inst;
    for(auto phi : phis) {
        // cout << "visit phis" << endl;
        Valuehandler(phi);
    }
    for(auto inst : insts) {
        // cout << "visit insts" << endl;
        Valuehandler(inst);
    }
}
//就是每次取一个基本块，然后每次从Sv中取一个
void CondCCPHandler() {
    while(!Wb.empty() || !Wv.empty()) {
        if(!Wb.empty()){
            auto Sb = Wb.back();
            Wb.pop_back();
            // cout << "CCP block: " << Sb->name << endl;
            Handler(Sb);
        }
        if(!Wv.empty()) {
            auto Sv = Wv.back();
            Wv.pop_back();//一个比较神奇的是，这个
            Handler(Sv);
        }
        
    }
}

void CondReplaceValue(RawValue *value) {
switch(value->value.tag) {
    case RVT_RETURN: {
        auto &src = value->value.ret.value;
        if(!src) break;
        if(IsVal((RawValue *)src)) {
            auto SrcValue = CCPController.LookValue((RawValue *)src);
            // cout << "Replace return in value" << SrcValue << endl;
            src = CCPController.generateNumber(SrcValue);
        }
        break;
    }
    case RVT_BINARY:{
        if(IsVal(value))
            value->isDeleted = true;
        else {
            auto &lhs = value->value.binary.lhs;
            auto &rhs = value->value.binary.rhs;
            if(IsVal((RawValue *)lhs)) {
                auto SrcValue = CCPController.LookValue((RawValue *)lhs);
                lhs = CCPController.generateNumber(SrcValue);
            }
            if(IsVal((RawValue *)rhs)) {
                auto SrcValue = CCPController.LookValue((RawValue *)rhs);
                rhs = CCPController.generateNumber(SrcValue);
            }
        }
        break;
    }
    case RVT_STORE:{
        auto &src = value->value.store.value;
        if(IsVal((RawValue *)src)) {
            auto SrcValue = CCPController.LookValue((RawValue *)src);
            // cout << "Replace return in value" << SrcValue << endl;
            src = CCPController.generateNumber(SrcValue);
        }
        break;
    }
    //branch这里还是直接修改比较好
    case RVT_CALL: {
        auto &call = value->value.call;
        auto &args = call.args;
        for(auto &arg : args) {
            if(IsVal(arg)) {
                auto SrcValue = CCPController.LookValue((RawValue *)arg);
                arg = CCPController.generateNumber(SrcValue);
            }
        }
        break;
    }
    case RVT_PHI: {
        auto &phis = value->value.phi.phi;
        for(auto &phi : phis) {
            if(IsVal(phi.second)) {
                auto SrcValue = CCPController.LookValue((RawValue *)phi.second);
                phi.second = CCPController.generateNumber(SrcValue);
            }
        }
        break;
    }
    case RVT_BRANCH: {
        auto &cond = value->value.branch.cond;
        auto &trueBB = value->value.branch.true_bb;
        auto &falseBB = value->value.branch.false_bb;
        int jumpCond;
        if(IsVal((RawValue *)cond)) {
            jumpCond = CCPController.LookValue((RawValue *)cond);
        } else break;
        if(jumpCond) {
            value->value.tag = RVT_JUMP;
            value->value.jump.target = trueBB;
        } else {
            value->value.tag = RVT_JUMP;
            value->value.jump.target = falseBB;
        }
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
    for(auto func : funcs){
    AddWorkerCCP(func);
    CondCCPHandler();
    }
    MarkDeletedBB(programme);
    for(auto func : funcs) {
        ClearInst(func);
    }
}