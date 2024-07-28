#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
//我觉得这里可以不用考虑删除
//目前只考虑乘累加，因为貌似只出现了乘累加
void InstMerge(RawValue *&value) {
    auto &lhs = value->value.binary.lhs;
    auto &rhs = value->value.binary.rhs;
    auto op = value->value.binary.op;
    // cerr << "Inst lhs tag: " << lhs->ty->tag << " rhs tag: " << rhs->ty->tag << endl;
    if(lhs->ty->tag != RTT_FLOAT || rhs->ty->tag != RTT_FLOAT) return;
    if(op != RBO_FADD && op != RBO_FSUB) return;
    auto ltag = lhs->value.tag;
    auto rtag = rhs->value.tag;
    if(lhs->value.binary.op == RBO_FMUL && rhs->value.binary.op == RBO_FMUL) return;
    if(ltag == RVT_BINARY && lhs->value.binary.op == RBO_FMUL)  {
        if(op == RBO_FADD){
        auto llhs = lhs->value.binary.lhs;
        auto rlhs = lhs->value.binary.rhs;
        value->value.tag = RVT_TRIPE;
        value->value.triple.hs1 = llhs;
        value->value.triple.hs2 = rlhs;
        value->value.triple.hs3 = rhs;
        value->value.triple.op = RTO_FMADD;
        }
        // else if(op == RBO_FSUB)
        //     value->value.triple.op = RTO_FMSUB;
        return;
    }
    if(rtag == RVT_BINARY && rhs->value.binary.op == RBO_FMUL) {
        if(op == RBO_FADD) {
        auto lrhs = rhs->value.binary.lhs;
        auto rrhs = rhs->value.binary.rhs;
        value->value.tag = RVT_TRIPE;
        value->value.triple.hs1 = lrhs;
        value->value.triple.hs2 = rrhs;
        value->value.triple.hs3 = lhs;
        value->value.triple.op = RTO_FMADD;
        } 
        // else if(op == RBO_FSUB) {
        //     value->value.triple.op = RTO_FNMSUB;
        // }
        return;
    }
}

void InstMerge(RawBasicBlock *block) {
    auto &insts = block->inst;
    for(auto &inst : insts) {
        if(inst->value.tag == RVT_BINARY) {
            InstMerge(inst);
        } else continue;
    }
}

void InstMerge(RawFunction *&func) {
    auto &bbs = func->basicblock;
    for(auto &bb : bbs) {
        InstMerge(bb);
    }
}

void InstMerge(RawProgramme *&programmer)
{
    auto &funcs = programmer->funcs;
    for(auto &func : funcs) {
        InstMerge(func);
    }
}