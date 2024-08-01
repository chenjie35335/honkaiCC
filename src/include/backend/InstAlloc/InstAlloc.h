#ifndef INSTALLOC
#define INSTALLOC
#include "../../midend/IR/BasicBlock.h"
#include "../../midend/IR/Function.h"
#include "../../midend/IR/Programme.h"
class InstAlloc{
    public:
        /// @brief 当前基本块
        RawBasicBlock *tempBlock;
        /// @brief 当前函数
        RawFunction *tempFunction;
        InstAlloc() {}
};

void InstAllocProgramme(RawProgramme *programme);

void InstAllocFunction(RawFunction *function);

void InstAllocBlock(RawBasicBlock *block);

#endif