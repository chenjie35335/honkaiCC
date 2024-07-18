#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
// 关心三个事情： basicblock上的use def value的定值点，使用点和定值基本块

void MarkUseDef(RawBasicBlock *&bb)
{
    auto &insts = bb->inst;
    for (auto &inst : insts)
    {
        auto tag = inst->value.tag;
        switch (tag)
        {
        case RVT_RETURN://src的use
        {
            auto src = (RawValue *)inst->value.ret.value;
            if(src) {
            src->usebbs.push_back(bb);
            src->usePoints.push_back(inst);
            bb->uses.insert(inst);
            }
            break;
        }
        case RVT_BINARY://lhs,rhs的use ，inst的def
        {
            auto lhs = (RawValue *)inst->value.binary.lhs;
            auto rhs = (RawValue *) inst->value.binary.rhs;
            lhs->usePoints.push_back(inst);
            lhs->usebbs.push_back(bb);
            rhs->usePoints.push_back(inst);
            rhs->usebbs.push_back(bb);
            inst->defPoints.push_back(inst);
            inst->defbbs.push_back(bb);
            bb->defs.insert(inst);
            bb->uses.insert(lhs);
            bb->uses.insert(rhs);
            break;
        }
        case RVT_LOAD://src的use，inst的def
        {
            auto src = (RawValue *)inst->value.load.src;
            src->usePoints.push_back(inst);
            src->usebbs.push_back(bb);
            bb->uses.insert(src);
            inst->defPoints.push_back(inst);
            inst->defbbs.push_back(bb);
            bb->defs.insert(inst);
            break;
        }
        case RVT_STORE://src的use,dest的def
        {
            auto src = (RawValue *) inst->value.store.value;
            auto dest = (RawValue *) inst->value.store.dest;
            src->usePoints.push_back(inst);
            src->usebbs.push_back(bb);
            bb->uses.insert(src);
            dest->defPoints.push_back(inst);
            dest->defbbs.push_back(bb);
            bb->defs.insert(dest);
            break;
        }
        case RVT_BRANCH://cond的use
        {
            auto cond = (RawValue *)inst->value.branch.cond;
            cond->usePoints.push_back(inst);
            cond->usebbs.push_back(bb);
            bb->uses.insert(cond);
            break;
        }
        case RVT_CALL://call的def,param的use
        {
            auto &params = inst->value.call.args;
            for(auto param : params) {
                param->usePoints.push_back(inst);
                param->usebbs.push_back(bb);
                bb->uses.insert(param);
            }
            inst->defPoints.push_back(inst);
            inst->defbbs.push_back(bb);
            bb->defs.insert(inst);
            break;
        }
        case RVT_GET_PTR://这个唯一有用的就是index的use,其他没用
        {
            auto index = (RawValue *) inst->value.getptr.index;
            index->usePoints.push_back(inst);
            index->usebbs.push_back(bb);
            bb->uses.insert(index);
            break;
        }
        case RVT_GET_ELEMENT:
        {
            auto index = (RawValue *) inst->value.getelement.index;
            index->usePoints.push_back(inst);
            index->usebbs.push_back(bb);
            bb->uses.insert(index);
            break;
        }
        case RVT_PHI:
        {
            auto &phis = inst->value.phi.phi;
            for(auto &phi : phis) {
                auto phiElem = phi.second;
                phiElem->usebbs.push_back(bb);
                phiElem->usePoints.push_back(inst);
                bb->uses.insert(phiElem);
            }
            inst->defPoints.push_back(inst);
            inst->defbbs.push_back(bb);
            bb->defs.insert(inst);
            break;
        }
        case RVT_CONVERT://目前没用上,先暂时不考虑
        {
           
            break;
        }
        default:
           break;
        }
    }
}

void MarkUseDef(RawFunction *&function)
{
    auto &params = function->params;
    auto &bbs = function->basicblock;
    for (auto &bb : bbs)
    {
        MarkUseDef(bb);
    }
}

void MarkUseDef(RawProgramme *&programmer)
{
    auto &values = programmer->values;
    auto &functions = programmer->funcs;
    for (auto function : functions)
    {
        MarkUseDef(function);
    }
}