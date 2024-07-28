#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include "../../../include/midend/Optimizer/OptimizeBlockEliminate.h"
#include <algorithm>
#include <cstdlib>
#include <unordered_map>
using namespace std;

void BlockEliminate(RawFunction *&function) {
    auto &blocks = function->basicblock;
    for(auto block = blocks.begin(); block != blocks.end(); ++block){
        // for(auto pbb : (*block)->pbbs) {
        //     cout << "Block " << (*block)->name << " has pre bb " << pbb->name << endl;
        // }
        // for(auto fbb : (*block)->fbbs) {
        //     cout << "Block " << (*block)->name << " has follow bb " << fbb->name << endl;
        // }
        if(block == blocks.begin()) continue;
        if((*block)->pbbs.empty()) {
            (*block)->isDeleted = true;
            for(auto fbb : (*block)->fbbs) {
                fbb->pbbs.remove(*block);
            }
        }
    }
}

void BlockEliminate(RawProgramme *&programme){
    auto &funcs = programme->funcs;
    for(auto func : funcs) {
        BlockEliminate(func);
    }
    for(auto func : funcs) {
        auto &blocks = func->basicblock;
        blocks.erase(remove_if(blocks.begin(), blocks.end(), [](RawBasicBlock *bb) {
    return bb->isDeleted == true;
}), blocks.end());
    }
}