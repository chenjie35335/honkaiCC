#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/hardwareManager.h"
#include <cassert>
#include <cstring>
#include <random>
#include <iostream>
using namespace std;
/// @brief 栈区域管理
MemoryManager memoryManager;
/// @brief 寄存器管理
RegisterManager registerManager;
/// @brief 分配栈区域
/// @param maxSize
void ManagerAlloc(int maxSize)
{
    memoryManager.maxSize = maxSize;
    memoryManager.Offset = 0;
}
//栈地址合适分配？
/*
    1、 开始时候只有变量分配内存
    2、 当需要换出寄存器时候，分配内存
    3、 当值从内存中取出时，这里为了简化不进行内存删除操作（这里可能是一个需要优化的过程）
*/
/// @brief 分配栈地址给RawValue
/// @param value
int StackAlloc(RawValueP value)
{
    if (memoryManager.Offset > memoryManager.maxSize)
        assert(0);
    else
    {
        memoryManager.StackManager.insert(pair<RawValueP, int>(value, memoryManager.Offset));
        return 4*(memoryManager.Offset++);
    }
}

/// @brief 寄存器管理器
void RegisterManagerAlloc()
{
    for (int i = 0; i < 32; i++)
        registerManager.RegisterLock[i] = false;
    registerManager.RegisterFull = false;
    registerManager.tempRegister = 5;
}

/// @brief 判断分配的寄存器是否合法
/// @param loc
/// @return
bool isValid(int loc)
{
    return loc > 4 && loc != 10 && loc != 11 && !registerManager.RegisterLock[loc] && loc < 32;
}

/// @brief 获取value分配的内存地址偏移
/// @param value
/// @return
int getTargetOffset(const RawValueP &value)
{
    if (memoryManager.StackManager.find(value) != memoryManager.StackManager.end())
    {
        return memoryManager.StackManager[value]*4;
    }
    else
        assert(0);
}

/// @brief 将寄存器值存入内存
/// @param value
/// @return
void StoreReg(int RandSelected)
{
    const char *TargetReg;
    int TargetOffset;
    for (const auto &pair : registerManager.registerLook)
    {
        if (pair.second == RandSelected)
        {
            TargetReg = regs[RandSelected];
            if(IsMemory(pair.first)) {
            TargetOffset = getTargetOffset(pair.first);
            }
            else
            {
            TargetOffset = StackAlloc(pair.first);
            }
            registerManager.registerLook.erase(pair.first);
            break;
        }
    }
    cout << "  sw   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
}

/// @brief 将原始值从内存load到寄存器中
/// @param value
/// @return
void LoadFromMemory(const RawValueP value)
{
    AllocRegister(value);
    const char *reg = GetRegister(value);
    int TargetOffset = getTargetOffset(value);
    cout << "  lw  " << reg << ", " << TargetOffset << "(sp)" << endl;
}

/// @brief 给RawValue分配寄存器
/// @param value
/// @return
void AllocRegister(const RawValueP &value)
{
    //cout << "registerManager.RegisterFull = " << registerManager.RegisterFull << endl;
    if (registerManager.RegisterFull)
    {
        int RandSelected;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(0, 31);
        do
        {
            RandSelected = dis(gen);
        } while (!isValid(RandSelected));
        StoreReg(RandSelected);
        registerManager.registerLook.insert(pair<RawValueP, int>(value, RandSelected));
    }
    else
    {
        uint32_t &RegLoc = registerManager.tempRegister;
        registerManager.registerLook.insert(pair<RawValueP, int>(value, RegLoc));
        do
        {
            RegLoc++;
        } while ((RegLoc == 10 || RegLoc == 11 || registerManager.RegisterLock[RegLoc]) && RegLoc < 32);
        //cout << "RegLoc ==" << RegLoc << endl;
        if(RegLoc == 32) registerManager.RegisterFull = true;
    }
}
// 这里还需要一个查找表的算法
bool IsRegister(const RawValueP &value)
{
    if (registerManager.registerLook.find(value) != registerManager.registerLook.end())
        return true;
    else
        return false;
}

bool IsMemory(const RawValueP &value)
{
    if (memoryManager.StackManager.find(value) != memoryManager.StackManager.end())
        return true;
    else
        return false;
}

// 加锁，使得与同一个RawValue相关的寄存器不被换出
void addLockRegister(const RawValueP &value)
{
    if (registerManager.registerLook.find(value) != registerManager.registerLook.end())
    {
        int loc = registerManager.registerLook.at(value);
        registerManager.RegisterLock[loc] = true;
    }
    else
    {
        assert(0);
    }
}
// 释放锁
void LeaseLockRegister(const RawValueP &value)
{
    if (registerManager.registerLook.find(value) != registerManager.registerLook.end())
    {
        int loc = registerManager.registerLook.at(value);
        registerManager.RegisterLock[loc] = false;
    }
    else
    {
        assert(0);
    }
}

// 获取
const char *GetRegister(const RawValueP &value)
{
    if (registerManager.registerLook.find(value) != registerManager.registerLook.end())
    {
        int loc = registerManager.registerLook.at(value);
        return regs[loc];
    }
    else
    {
        assert(0);
    }
}

//分配0号寄存器
void AllocX0(const RawValueP &value)
{
    registerManager.registerLook.insert(pair<RawValueP, int>(value, 0));
}

int getAllocLen(const RawFunctionP &func) {
    auto &bbs = func->bbs;
    int len = 0;
    for(int i = 0; i < bbs.len;i++) {
        auto ptr = reinterpret_cast<RawBasicBlockP>(bbs.buffer[i]);
        auto &insts = ptr->insts;
        len += insts.len;
    }
    return len*4;
}

int getStackSize() {
    return memoryManager.maxSize;
}

