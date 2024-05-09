#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cassert>
#include <cstring>
#include <random>
#include <iostream>
using namespace std;
extern HardwareManager hardware;

int ValueArea::getTargetOffset(const RawValueP &value) const
{
    if (StackManager.find(value) != StackManager.end())
        return StackManager.at(value);
    else
        assert(0);
}
/// @brief 返回
/// @param value 
/// @return 
int calArrLen(const RawTypeP &value) {
    assert(value->tag == RTT_ARRAY);
    auto ElemTag = value->data.array.base->tag;
    if(ElemTag == RTT_INT32) return value->data.array.len*4;
    else if(ElemTag == RTT_ARRAY) return calArrLen(value->data.array.base) * value->data.array.len;
    else return 0;
}

/// @brief 获取指针所指位置的值
/// @param value 
/// @return 
int calPtrLen(const RawValueP &value) {
    auto TyTag = value->ty->tag;
    //cout << "TyTag" << TyTag << endl;
    if(TyTag == RTT_POINTER) {
        auto PointerTy = value->ty->data.pointer.base;
        auto PointerTyTag = PointerTy->tag;
        if(PointerTyTag == RTT_INT32) return 4;
        else if(PointerTyTag == RTT_ARRAY) return calArrLen(PointerTy);
        else assert(0);
    } else if(TyTag == RTT_ARRAY) {
        return calArrLen(value->ty);
    } else return 0;
}
//这里需要修改
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
                int len = calPtrLen(value);
                //cout << "save len =" << len << endl; 
                LocalLen += len+4;//这里给每个指针值加上一个4字节用于存储指针
                hardware.SaveLen(value,len);
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
        ReserveLen = 14 * 4;//无论有没有，这个我们都保存一下返回地址
        //cout << "Args=" <<  ArgsLen << ",Local=" << LocalLen << ",Reserve=" << ReserveLen << endl;
}

void HardwareManager::init(const RawFunctionP &function)
{
    int ArgsLen = 0, LocalLen = 0, ReserveLen = 0;
    calculateSize(ArgsLen, LocalLen, ReserveLen, function);
    int StackLen = ArgsLen + LocalLen + ReserveLen;
    StackLen = (StackLen + 15)/16*16;
    memoryManager.initStack(StackLen);
    int ArgsMin = 0, ArgsMax = ArgsLen-4;
    memoryManager.initArgsArea(ArgsMin,ArgsMax);
    int LocalMin = ArgsMax+4, LocalMax = LocalMin+LocalLen-4;
    memoryManager.initLocalArea(LocalMin,LocalMax);
    int ReserveMin = LocalMax+4, ReserveMax = ReserveMin+ReserveLen-4;
    memoryManager.initReserveArea(ReserveMin,ReserveMax);
    registerManager.init();
}

void MemoryManager::initArgsArea(int min,int max) {
    //cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    this->argsArea.minAddress = min;
    this->argsArea.maxAddress = max;
    this->argsArea.tempOffset = min;
    this->argsArea.StackManager.clear();
}

void MemoryManager::initReserveArea(int min,int max) {
    //cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    auto &ReserveArea = this->reserveArea;
    ReserveArea.minAddress = min;
    ReserveArea.maxAddress = max;
    ReserveArea.tempOffset = max;
    ReserveArea.StackManager.clear();
}

void MemoryManager::initLocalArea(int min,int max) {
    //cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    auto &LocalArea = this->localArea;
    LocalArea.minAddress = min;
    LocalArea.maxAddress = max;
    LocalArea.tempOffset = min;
    reserveArea.StackManager.clear();
}

void HardwareManager::LoadFromMemory(const RawValueP &value) {
    AllocRegister(value);
    const char *reg = GetRegister(value);
    int TargetOffset = getTargetOffset(value);
    cout << "  lw  " << reg << ", " << TargetOffset << "(sp)" << endl;
}

void HardwareManager::AllocRegister(const RawValueP &value) {
    //cout << "alloc register for " << value->value.tag << endl;
    if (registerManager.RegisterFull){
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
    } else {
        uint32_t &RegLoc = registerManager.tempRegister;
        registerManager.registerLook.insert(pair<RawValueP, int>(value, RegLoc));
        do{
            RegLoc++;
        } while (((RegLoc >= 10 && RegLoc <= 17) || registerManager.RegisterLock[RegLoc]) && RegLoc < 32);
        if(RegLoc == 32) registerManager.RegisterFull = true;
    }
}

void HardwareManager::StoreReg(int RandSelected) {
    const char *TargetReg;
    int TargetOffset;
    for (const auto &pair : registerManager.registerLook)
    {
        if (pair.second == RandSelected){
            auto value = pair.first;
            auto ty = value->ty;
            TargetReg = RegisterManager::regs[RandSelected];
            if(IsMemory(value)) {
            TargetOffset = getTargetOffset(pair.first);
            } else {
            TargetOffset = StackAlloc(pair.first);
            }
            registerManager.registerLook.erase(pair.first);
            if(value->value.tag == RVT_FUNC_ARGS) return;
            if(!ty) return;
            else if(ty->tag == RTT_ARRAY) return;
            else if(ty->tag == RTT_POINTER) {
                auto PointerTy = ty->data.pointer.base;
                auto PointerTag = PointerTy->tag;
                if(PointerTag == RTT_ARRAY) return;
                else cout << "  sw   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
            } else
            cout << "  sw   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
            break;
        }
    }
}

const char *RegisterManager::regs[32] = {
    "x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

const int RegisterManager::callerSave[7] = {
    5,6,7,28,29,30,31
};

const int RegisterManager::calleeSave[12] = {
    8,9,18,19,20,21,22,23,24,25,26,27
};

void RegisterArea::LoadRegister(int reg) {
    assert(StackManager.find(reg) != StackManager.end());
    int offset = StackManager.at(reg);
    if(offset <= 2047) {
    cout << "  lw  " << RegisterManager::regs[reg] << ", " << offset << "(sp)" << endl;
    } else {
    cout << "  li  t0," << offset << endl;
    cout << "  add t0, sp, t0" << endl;   
    cout << "  lw  " << RegisterManager::regs[reg] << ", " << 0 << "(t0)" << endl; 
    }
}

void RegisterArea::SaveRegister(int reg) {
    if(tempOffset <= 2047) {
    cout << "  sw  " << RegisterManager::regs[reg] << ", " << tempOffset << "(sp)" << endl;
    } else {
    cout << "  li  t0," << tempOffset << endl;
    cout << "  add t0, sp, t0" << endl;
    cout << "  sw  " << RegisterManager::regs[reg] << ", " << 0 << "(t0)" << endl;
    }//这个方法虽然蠢但是是正确的
    StackManager.insert(pair<int,int>(reg,tempOffset));
    tempOffset -= 4;
}
// init要做的事：
/*
1、 给被调用者保存寄存器分配内存空间(这个由于在最上层最后弄)
2、 访问所有参数，确定args的空间
3、 访问所有insts,创建local区域的大小
*/