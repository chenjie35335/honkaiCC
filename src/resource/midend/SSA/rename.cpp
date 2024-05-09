#include "../../../include/midend/SSA/rename.h"
#include "../../../include/midend/IR/Programme.h"
#include "../../../include/midend/IR/Function.h"
#include "../../../include/midend/IR/BasicBlock.h"
#include "../../../include/midend/IR/Value.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cassert>
//这里rename需要干一个事情：
//首先是对于全局变量、函数参数，都要将自己本身入栈
//对于基本块参数来说，这个不可能是第一个定义，所以处理方式会有些不同
void renameValue(RawBasicBlock *&bb) {
    auto& params = bb->params; 
    auto& insts = bb->insts;
    for(int i = 0; i < params.len;i++) {
        auto ParamPtr = (RawValue *) params.buffer[i];
        assert(ParamPtr->value.tag == RVT_BLOCK_ARGS);
        auto target = (RawValue *)ParamPtr->value.data.blockArgs.target;
        target->tempCopy = ParamPtr;
    }
    for(int i = 0; i < insts.len;i++) {
        auto InstPtr = (RawValue*) insts.buffer[i];
        
    }
}
//对于全局变量和函数参数来说，肯定是第一个定义
void renameValue(RawFunction *&function) {
    auto &bbs = function->bbs;
    auto &params = function->params;
    for(int i = 0; i < params.len;i++) {
        auto ParamPtr = (RawValue *) params.buffer[i];
        ParamPtr->tempCopy = ParamPtr;
    }
    for(int i = 0; i < bbs.len;i++) {
        auto bbPtr = (RawBasicBlock *) bbs.buffer[i];
        renameValue(bbPtr);
    }
} 

void renameValue(RawProgramme *& programme){
    auto &func = programme->Funcs;
    auto &values = programme->Value;
    for(int i = 0; i < values.len;i++) {
        auto ValuePtr = (RawValue *) values.buffer[i];
        ValuePtr->tempCopy = ValuePtr;
    }
    for(int i = 0; i < func.len; i++) {
        auto FuncPtr = (RawFunction *) func.buffer[i];
        renameValue(FuncPtr);
    }
}