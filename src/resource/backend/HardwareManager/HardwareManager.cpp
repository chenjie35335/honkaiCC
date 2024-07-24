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
    auto ElemTag = value->array.base->tag;
    if (ElemTag == RTT_INT32 || ElemTag == RTT_FLOAT)
        return value->array.len * 4;
    else if (ElemTag == RTT_ARRAY)
        return calArrLen(value->array.base) * value->array.len;
    else
        return 0;
}
/// @brief 这个是计算数组的一个单元的大小
/// @param value 
/// @return 
int calBaseLen(const RawValueP &value){
    auto PointerTy = value->ty->pointer.base;
    auto PointerTyTag = PointerTy->tag;
    if (PointerTyTag == RTT_INT32 || PointerTyTag == RTT_FLOAT)
        return 4;
    else if (PointerTyTag == RTT_ARRAY)
        return calArrLen(PointerTy);
    else assert(0);
}

/// @brief //这个是单纯计算alloc需要分配的空间
/// @param value
/// @return
int calAllocLen(const RawValueP &value)
{
    auto PointerTy = value->ty->pointer.base;
    auto PointerTyTag = PointerTy->tag;
    if (PointerTyTag == RTT_INT32 || PointerTyTag == RTT_FLOAT)
        return 4;
    else if (PointerTyTag == RTT_ARRAY)
        return calArrLen(PointerTy);
    else if(PointerTyTag == RTT_POINTER)
        return 8;
    else assert(0);
}
// 这里需要修改
void calculateSize(int &ArgsLen, int &LocalLen, int &ReserveLen, const RawFunctionP &function)
{
    bool has_call;
    auto &params = function->params; // 给所有的参数分配空间
    LocalLen += 8*50;//这50个是固定给全局变量分配的空间
    for (auto bb : function->basicblock)
    {
        for (auto value : bb->inst)
        {
            if (value->value.tag == RVT_ALLOC)
            { // alloc 指令分配的内存,大小为4字节
                int len = calAllocLen(value);
                // cout << "save len =" << len << endl;
                LocalLen += len + 8; // 这里给每个指针值加上一个8字节用于存储指针
                hardware.SaveLen(value, len);
            }
            else if (value->ty->tag != RTT_UNIT)
            { // 指令的类型不为unit, 存在返回值，分配内存
                LocalLen += 8;//存储指针
            }
            if (value->value.tag == RVT_CALL)
            {
                has_call = true;
                ArgsLen = max(ArgsLen, int(value->value.call.args.size() - 8)) * 8;
            }
        }
    }
    ReserveLen = 14 * 8; // 无论有没有，这个我们都保存一下返回地址
    // cout << "Args=" <<  ArgsLen << ",Local=" << LocalLen << ",Reserve=" << ReserveLen << endl;
}

void HardwareManager::init(const RawFunctionP &function)
{
    int ArgsLen = 0, LocalLen = 0, ReserveLen = 0;
    calculateSize(ArgsLen, LocalLen, ReserveLen, function);
    int StackLen = ArgsLen + LocalLen + ReserveLen;
    StackLen = (StackLen + 15) / 16 * 16;
    memoryManager.initStack(StackLen);
    int ArgsMin = 0, ArgsMax = ArgsLen - 8;
    memoryManager.initArgsArea(ArgsMin, ArgsMax);
    int LocalMin = ArgsMax + 8, LocalMax = LocalMin + LocalLen - 8;
    memoryManager.initLocalArea(LocalMin, LocalMax);
    int ReserveMin = LocalMax + 8, ReserveMax = ReserveMin + ReserveLen - 8;
    memoryManager.initReserveArea(ReserveMin, ReserveMax);
    registerManager.init();
}

void MemoryManager::initArgsArea(int min, int max)
{
    // cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    this->argsArea.minAddress = min;
    this->argsArea.maxAddress = max;
    this->argsArea.tempOffset = min;
    this->argsArea.StackManager.clear();
}

void MemoryManager::initReserveArea(int min, int max)
{
    // cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    auto &ReserveArea = this->reserveArea;
    ReserveArea.minAddress = min;
    ReserveArea.maxAddress = max;
    ReserveArea.tempOffset = max;
    ReserveArea.StackManager.clear();
}

void MemoryManager::initLocalArea(int min, int max)
{
    // cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    auto &LocalArea = this->localArea;
    LocalArea.minAddress = min;
    LocalArea.maxAddress = max;
    LocalArea.tempOffset = min;
    reserveArea.StackManager.clear();
}

void HardwareManager::LoadFromMemory(const RawValueP &value)
{
    AllocRegister(value);
    const char *reg = GetRegister(value);
    int TargetOffset = getTargetOffset(value);
    if(value->value.tag == RVT_FLOAT){
        cout << "  flw  " << reg << ", " << TargetOffset << "(sp)" << endl;
    } else {
    if(TargetOffset > 2047) {
        cout << "  li   " << "t0, " << TargetOffset << endl;
        cout << "  add  " << "t0, sp, t0" << endl;
        cout << "  ld  " <<  reg << ", " << 0 << "(t0)" << endl; 
    } else 
        cout << "  ld   " << reg << ", " << TargetOffset << "(sp)" << endl;
    }
    
}

void HardwareManager::AllocRegister(const RawValueP &value)
{
    // cout << "alloc register for " << value->value.tag << endl;
    if(registerManager.registerLook.find(value) != registerManager.registerLook.end())  return;
    // for(auto pair : registerManager.registerLook) {
        // cout << "before register " << pair.second << " in look" << endl;
    // }
    // cout << "before registerlook size:" << registerManager.registerLook.size() << endl;
    if (registerManager.RegisterFull)
    {
        int RandSelected;
        auto it = registerManager.registerLook.begin();
        do{
            RandSelected = it->second;
            it++;
        }while(!isValid(RandSelected) && it != registerManager.registerLook.end());
        StoreReg(RandSelected);
        //registerManager.registerLook.insert(pair<RawValueP, int>(value, RandSelected));
        registerManager.registerLook[value] = RandSelected;
        // cout << "alloc register :" << RandSelected<<" " << RegisterManager::regs[RandSelected] << endl;
    }
    else
    {
        uint32_t &RegLoc = registerManager.tempRegister;
        registerManager.registerLook[value] = RegLoc;
        // cout << "alloc register " << RegLoc << RegisterManager::regs[RegLoc] << endl;
        do
        {
            RegLoc++;
        } while (((RegLoc >= 10 && RegLoc <= 17) || registerManager.RegisterLock[RegLoc]) && RegLoc < 32);
        if (RegLoc == 32)
            registerManager.RegisterFull = true;
    }
    // for(auto pair : registerManager.registerLook) {
        // cout << "after register " << pair.second << " in look" << endl;
    // }
    // cout << "after registerlook size:" << registerManager.registerLook.size() << endl;
}


void HardwareManager::StoreReg(int RandSelected)
{
    //cout << "spill reg" << RandSelected << endl;
    const char *TargetReg;
    int TargetOffset;
    auto pair = registerManager.registerLook.begin();
    for (;pair != registerManager.registerLook.end();pair++)
        if (pair->second == RandSelected)
            break;
    if(pair == registerManager.registerLook.end()) return;
    auto value = pair->first;
    TargetReg = RegisterManager::regs[RandSelected];
    if (IsMemory(value))
        TargetOffset = getTargetOffset(value);
    else
        TargetOffset = StackAlloc(value);
    registerManager.registerLook.erase(value);
    // for(auto look : registerManager.registerLook) {
        // cout << "erase register " << look.second << " in look" << endl;
    // }
    // cout << "erase registerlook size:" << registerManager.registerLook.size() << endl;
    if(value->value.tag == RVT_ALLOC || value->value.tag == RVT_GLOBAL) return;
    if(TargetOffset > 2047) {
        cout << "  li   " << "t0, " << TargetOffset << endl;
        cout << "  add  " << "t0, sp, t0" << endl;
        cout << "  sd  " <<  TargetReg << ", " << 0 << "(t0)" << endl; 
    } else
        cout << "  sd   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
}


const char *RegisterManager::regs[32] = {
    "x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

const char *RegisterManager::fregs[32] = {
    "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7", //temporary variables
    "fs0", "fs1",  //saved variables
    "fa0", "fa1",  //function arguments / return values
    "fa2", "fa3", "fa4", "fa5", "fa6", "fa7", //function arguments
    "fs2", "fs3", "fs4", "fs5", "fs6", "fs7", "fs8", "fs9", "fs10", "fs11", //savd variables
    "ft8", "ft9", "ft10", "ft11"  //temporary variables
    //之前写的是寄存器序号，不是寄存器名字，难崩
    //这样写是按寄存器的序号排列的
};

const int RegisterManager::callerSave[7] = {
    5, 6, 7, 28, 29, 30, 31};

const int RegisterManager::calleeSave[12] = {
    8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};

void RegisterArea::LoadRegister(int reg)
{
    assert(StackManager.find(reg) != StackManager.end());
    int offset = StackManager.at(reg);
    if (offset <= 2047)
    {
        cout << "  ld  " << RegisterManager::regs[reg] << ", " << offset << "(sp)" << endl;
    }
    else
    {
        cout << "  li  t0," << offset << endl;
        cout << "  add t0, sp, t0" << endl;
        cout << "  ld  " << RegisterManager::regs[reg] << ", " << 0 << "(t0)" << endl;
    }
}

//fregs
void RegisterArea::LoadFRegister(int reg)
{
    assert(StackManager.find(reg) != StackManager.end());
    int offset = StackManager.at(reg);
    if (offset <= 2047)
    {
        cout << "  flw  " << RegisterManager::fregs[reg] << ", " << offset << "(sp)" << endl;
    }
    else
    {
        cout << "  li  t0," << offset << endl;
        cout << "  add t0, sp, t0" << endl;
        cout << "  flw  " << RegisterManager::fregs[reg] << ", " << 0 << "(t0)" << endl;
    }
}


void RegisterArea::SaveRegister(int reg)
{
    if (tempOffset <= 2047)
    {
        cout << "  sd  " << RegisterManager::regs[reg] << ", " << tempOffset << "(sp)" << endl;
    }
    else
    {
        cout << "  li  t0," << tempOffset << endl;
        cout << "  add t0, sp, t0" << endl;
        cout << "  sd  " << RegisterManager::regs[reg] << ", " << 0 << "(t0)" << endl;
    } // 这个方法虽然蠢但是是正确的
    StackManager.insert(pair<int, int>(reg, tempOffset));
    tempOffset -= 8;
}

//fregs
void RegisterArea::SaveFRegister(int reg)
{
    if (tempOffset <= 2047)
    {
        cout << "  fsw  " << RegisterManager::fregs[reg] << ", " << tempOffset << "(sp)" << endl;
    }
    else
    {
        cout << "  li  t0," << tempOffset << endl;
        cout << "  add t0, sp, t0" << endl;
        cout << "  fsw  " << RegisterManager::fregs[reg] << ", " << 0 << "(t0)" << endl;
    } // 这个方法虽然蠢但是是正确的
    StackManager.insert(pair<int, int>(reg, tempOffset));
    tempOffset -= 4;
}

// init要做的事：
/*
1、 给被调用者保存寄存器分配内存空间(这个由于在最上层最后弄)
2、 访问所有参数，确定args的空间
3、 访问所有insts,创建local区域的大小
*/