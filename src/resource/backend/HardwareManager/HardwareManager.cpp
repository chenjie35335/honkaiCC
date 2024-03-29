#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cassert>
#include <cstring>
#include <random>
#include <iostream>
using namespace std;

int ValueArea::getTargetOffset(const RawValueP &value) const
{
    if (StackManager.find(value) != StackManager.end())
        return StackManager.at(value);
    else
        assert(0);
}

void calculateSize(int &ArgsLen, int &LocalLen, int &ReserveLen, const RawFunctionP &function)
{
    bool has_call;
    auto& params = function->params;//给所有的参数分配空间
    LocalLen += 4*params.len; 
    for (size_t i = 0; i < function->bbs.len; ++i)
    {
        assert(function->bbs.kind == RSK_BASICBLOCK);
        auto bb = (RawBasicBlockP)function->bbs.buffer[i];
        for (size_t j = 0; j < bb->insts.len; ++j)
        {
            assert(bb->insts.kind == RSK_BASICVALUE);
            auto value = (RawValueP)bb->insts.buffer[j];
            if (value->value.tag == RVT_ALLOC)
            { // alloc 指令分配的内存,大小为4字节
                LocalLen += 4;
            }
            else if (value->ty->tag != RTT_UNIT)
            { // 指令的类型不为unit, 存在返回值，分配内存
                LocalLen += 4;
            }
            if (value->value.tag == RVT_CALL)
            {
                has_call = true;
                ArgsLen = max(ArgsLen, int(value->value.data.call.args.len - 8)) * 4;
            }
        }
    }
    if (has_call)
        ReserveLen = 14 * 4;
    else
        ReserveLen = 13 * 4;//这里和我们寄存器调度算法相关
}

void HardwareManager::init(const RawFunctionP &function)
{
    int ArgsLen = 0, LocalLen = 0, ReserveLen = 0;
    calculateSize(ArgsLen, LocalLen, ReserveLen, function);
    int StackLen = ArgsLen + LocalLen + ReserveLen;
    StackLen = (StackLen + 15)/16*16;
    memoryManager->initStack(StackLen);
    int ArgsMin = 0, ArgsMax = ArgsLen-4;
    memoryManager->initArgsArea(ArgsMin,ArgsMax);
    int ReserveMin = ArgsMax+4, ReserveMax = ReserveMin+ReserveLen-4;
    memoryManager->initReserveArea(ReserveMin,ReserveMax);
    int LocalMin = ReserveMax+4, LocalMax = LocalMin+LocalLen-4;
    memoryManager->initLocalArea(LocalMin,LocalMax);
}

void MemoryManager::initArgsArea(int min,int max) {
    auto ArgsArea = this->argsArea;
    ArgsArea->minAddress = min;
    ArgsArea->maxAddress = max;
    ArgsArea->tempOffset = min;
}

void MemoryManager::initReserveArea(int min,int max) {
    auto ReserveArea = this->reserveArea;
    ReserveArea->minAddress = min;
    ReserveArea->maxAddress = max;
    ReserveArea->tempOffset = min;
}

void MemoryManager::initLocalArea(int min,int max) {
    auto LocalArea = this->localArea;
    LocalArea->minAddress = min;
    LocalArea->maxAddress = max;
    LocalArea->tempOffset = min;
}

void HardwareManager::LoadFromMemory(const RawValueP &value) {
    AllocRegister(value);
    const char *reg = GetRegister(value);
    int TargetOffset = getTargetOffset(value);
    cout << "  lw  " << reg << ", " << TargetOffset << "(sp)" << endl;
}

void HardwareManager::AllocRegister(const RawValueP &value) {
    if (registerManager->RegisterFull){
        int RandSelected;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(0, 31);
        do
        {
            RandSelected = dis(gen);
        } while (!isValid(RandSelected));
        StoreReg(RandSelected);
        registerManager->registerLook.insert(pair<RawValueP, int>(value, RandSelected));
    }
}

void HardwareManager::StoreReg(int RandSelected) {
    const char *TargetReg;
    int TargetOffset;
    for (const auto &pair : registerManager->registerLook)
    {
        if (pair.second == RandSelected){
            TargetReg = registerManager->regs[RandSelected];
            if(IsMemory(pair.first)) {
            TargetOffset = getTargetOffset(pair.first);
            }
            else
            {
            TargetOffset = StackAlloc(pair.first);
            }
            registerManager->registerLook.erase(pair.first);
            break;
        }
    }
    cout << "  sw   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
}

const char *RegisterManager::regs[32] = {
    "x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};
// init要做的事：
/*
1、 给被调用者保存寄存器分配内存空间(这个由于在最上层最后弄)
2、 访问所有参数，确定args的空间
3、 访问所有insts,创建local区域的大小
*/