#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include "../../../include/midend/Optimizer/OptimizeBlockEliminate.h"
#include <algorithm>
#include <cassert>
#include <unordered_set>
using namespace std;
unordered_set<RawBasicBlock *> visited;
//我的想法是这里的基本块要利用起来，不需要两个全部删除，删除掉本来的，将所有的指令合并到下一个
//而且初始基本块也不需要判断，因为可以考虑将这个基本块进行合并，如果基本块合并了，就可以考虑改名
void BlockMerge(RawBasicBlock *block,RawBasicBlock *entry) {
    visited.insert(block);
    for(auto fbb : block->fbbs) {
        if(visited.find(fbb) == visited.end()) BlockMerge(fbb, entry);
    }
    if(block->fbbs.size() == 1) {
        auto fbb = *block->fbbs.begin();
        if(fbb->pbbs.size() == 1) {
            auto &insts = block->inst;
            insts.pop_back();
            for(auto inst : insts) {
                fbb->inst.push_front(inst);
            }
            block->isDeleted = true;
            for(auto pbb : block->pbbs) {
                pbb->fbbs.remove(block);pbb->fbbs.push_back(fbb);
                fbb->pbbs.remove(block); fbb->pbbs.push_back(pbb);
            }
        }
    }
}

void BlockMerge(RawFunction *function){
    auto &bbs = function->basicblock;
    auto entry = *bbs.begin();
    BlockMerge(entry,entry);
    bbs.erase(remove_if(bbs.begin(), bbs.end(), [](RawBasicBlock *bb) {
        return bb->isDeleted == true;
    }), bbs.end());
}

bool isEmptyBlock(RawBasicBlock *block) {
    auto &insts = block->inst;
    if(insts.size() == 0) assert(0);
    if(insts.size() == 1) {
        auto inst = *insts.begin();
        if(inst->value.tag == RVT_JUMP) return true;
    }
    return false;
}

void EmptyBlockElimination(RawBasicBlock *entry, RawBasicBlock *block) {
    visited.insert(block);
    for(auto fbb : block->fbbs) {
        if(visited.find(fbb) == visited.end()) EmptyBlockElimination(entry,fbb);
    }
    if(block->fbbs.size() == 1 && isEmptyBlock(block)){
        block->isDeleted = true;
        auto fbb = *block->fbbs.begin();
        if(block == entry) fbb->name = "entry";
        for(auto pbb : block->pbbs) {
            auto &pbbInsts = pbb->inst;
            auto pbbInst = *pbbInsts.rbegin();
            if(pbbInst->value.tag == RVT_JUMP) {
                auto &jump = pbbInst->value.jump;
                if(jump.target == block) {
                    jump.target = fbb;
                }
            } else if(pbbInst->value.tag == RVT_BRANCH) {
                auto &branch = pbbInst->value.branch;
                if(branch.true_bb == block) branch.true_bb = fbb;
                if(branch.false_bb == block) branch.false_bb = fbb;
            } else continue;
            pbb->fbbs.remove(block);pbb->fbbs.push_back(fbb);
            fbb->pbbs.remove(block); fbb->pbbs.push_back(pbb);
        }
    }
}

//空块条件：fbb中只有一个元素，其仅存在jump语句
void EmptyBlockElimination(RawFunction *function){
    auto &bbs = function->basicblock;
    auto entry = *bbs.begin();
    EmptyBlockElimination(entry,entry);
    bbs.erase(remove_if(bbs.begin(), bbs.end(), [](RawBasicBlock *bb) {
        return bb->isDeleted == true;
    }), bbs.end());
}

void ReachableVisit(RawBasicBlock *basicblock) {
    visited.insert(basicblock);
    for(auto fbb : basicblock->fbbs) {
        if(visited.find(fbb) == visited.end()) 
            ReachableVisit(fbb);
    }
}

//不可达基本块删除
void UnreachableBlockElimination(RawFunction *function) {
    auto &bbs = function->basicblock;
    auto entry = *bbs.begin();
    ReachableVisit(entry);
    for(auto bb : bbs) {
        if(visited.find(bb) == visited.end()) {
            bb->isDeleted = true;
            for(auto fbb : bb->fbbs) {
                fbb->pbbs.remove(bb);
            }
            for(auto pbb : bb->pbbs) {
                bb->fbbs.remove(bb);
            }
        }
    }
    bbs.erase(remove_if(bbs.begin(), bbs.end(), [](RawBasicBlock *bb) {
        return bb->isDeleted == true;
    }), bbs.end());
}

void BlockEliminateRun(RawProgramme *programme){
    visited.clear();
    auto &funcs = programme->funcs;
    for(auto func : funcs) {
        UnreachableBlockElimination(func);
        visited.clear();
        EmptyBlockElimination(func);
        visited.clear();
        BlockMerge(func);
    }
}