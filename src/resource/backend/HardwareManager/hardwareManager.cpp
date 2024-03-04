#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/hardwareManager.h"
#include <cassert>
#include <cstring>
#include <random>
using namespace std;
/// @brief 栈区域管理
MemoryManager memoryManager;
/// @brief 寄存器管理
RegisterManager registerManager;
/// @brief 分配栈区域
/// @param maxSize 
void ManagerAlloc(int maxSize) {
    memoryManager.maxSize = maxSize;
    memoryManager.Offset = 0;
}
/// @brief 分配栈地址给RawValue
/// @param value 
void StackAlloc(const RawValue *value) {
    if(memoryManager.Offset > memoryManager.maxSize)  assert(0);
    else {
        memoryManager.StackManager.insert(pair<const RawValue *,int>(value,memoryManager.Offset));
    }
}

/// @brief 寄存器管理器
void RegisterManagerAlloc() {
    for(int i = 0; i < 32; i++) registerManager.RegisterLock[i] = false;
    registerManager.RegisterFull = false;
    registerManager.tempRegister = 5;
}
/// @brief 判断分配的寄存器是否合法
/// @param loc 
/// @return 
bool isValid(int loc) {
    return loc != 10 && loc != 11 && !registerManager.RegisterLock[loc];
}

/// @brief 将寄存器值存入内存
/// @param value 
/// @return 
void StoreReg(int RandSelected){
    const RawValue* value;
    for(const auto &pair : registerManager.registerLook) {
        if(pair.second == RandSelected) 
        {
            value = pair.first;
            break;
        }
    }
    
}

/// @brief 给RawValue分配寄存器
/// @param value 
/// @return 
const char *AllocRegister(const RawValue *value) {
    if(registerManager.RegisterFull) {
        int RandSelected;
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<int> dis(0, 31);
        do {
            RandSelected = dis(gen);
        } while(isValid(RandSelected));
        StoreReg(RandSelected);
        registerManager.registerLook.insert(pair<const RawValue *,int>(value,RandSelected));
        return regs[RandSelected];
    }
    else {
        uint32_t &RegLoc = registerManager.tempRegister;
        registerManager.registerLook.insert(pair<const RawValue *,int>(value,RegLoc));
        do {
            RegLoc++;
        } while(RegLoc == 10 || RegLoc == 11 || registerManager.RegisterLock[RegLoc]);
        return regs[RegLoc];
    }
}