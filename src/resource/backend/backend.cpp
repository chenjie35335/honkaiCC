#include"../../include/backend/Generator/generator.h"
#include "../../include/midend/IR/IRGraph.h"
#include <map>
#include <set>
using namespace std;

void backend(RawProgramme *& programme) {
    generateASM(programme);
}

void InsertNumber(RawBasicBlock *block,RawBasicBlock *entry) {
    auto &insts = block->inst;
    auto &entryInsts = entry->inst;
    set<RawValue *> HasInserted;
    for(auto it = insts.begin(); it != insts.end();it++) {
        auto value = *it;
        switch(value->value.tag) {
            case RVT_BINARY: {
                auto lhs = (RawValue *)value->value.binary.lhs;
                auto rhs = (RawValue *)value->value.binary.rhs;
                if(lhs->value.tag == RVT_FLOAT || lhs->value.tag == RVT_INTEGER) {
                    insts.insert(it,lhs);
                }
                if(rhs->value.tag == RVT_FLOAT || rhs->value.tag == RVT_INTEGER) {
                    insts.insert(it,rhs);
                }
                break;
            }
            case RVT_BRANCH: {
                auto cond = (RawValue *) value->value.branch.cond;
                if(cond->value.tag == RVT_FLOAT || cond->value.tag == RVT_INTEGER) {
                    insts.insert(it,cond);
                }
                break;
            }
            case RVT_CALL: {
                auto &params = value->value.call.args;
                for(auto param : params) {
                    if(param->value.tag == RVT_FLOAT || param->value.tag == RVT_INTEGER) {
                        insts.insert(it, param);
                    }
                }
                break;
            }
            case RVT_GET_ELEMENT: {
                auto index = (RawValue *) value->value.getelement.index;
                auto src = (RawValue *) value->value.getelement.src;
                if(index->value.tag == RVT_FLOAT || index->value.tag == RVT_INTEGER) {
                    insts.insert(it, index);
                }
                if(src->value.tag == RVT_GLOBAL && HasInserted.find(src) == HasInserted.end()) {
                    entryInsts.push_front(src);
                    HasInserted.insert(src);
                }
                break;
            }
            case RVT_GET_PTR: {
                auto index = (RawValue *) value->value.getptr.index;
                auto src = (RawValue *) value->value.getptr.src;
                if(index->value.tag == RVT_FLOAT || index->value.tag == RVT_INTEGER) {
                    insts.insert(it, index);
                }
                if(src->value.tag == RVT_GLOBAL && HasInserted.find(src) == HasInserted.end()) {
                    entryInsts.push_front(src);
                }
                break;
            }
            case RVT_STORE: {
                auto src = (RawValue *) value->value.store.value;
                if(src->value.tag == RVT_FLOAT || src->value.tag == RVT_INTEGER) {
                    insts.insert(it, src);
                }
                break;
            }
            default: break;
        }
    }
}

void InsertNumber(RawFunction *func) {
    auto &bbs = func->basicblock;
    auto &params = func->params;
    auto &entry = *bbs.begin();
    for(auto param : params) {
        entry->inst.push_front(param);
    }
    for(auto bb : bbs) {
        InsertNumber(bb,entry);
    }
}

void InsertNumber(RawProgramme *& program) {
    auto funcs = program->funcs;
    for(auto &func : funcs) {
        InsertNumber(func);
    }
}