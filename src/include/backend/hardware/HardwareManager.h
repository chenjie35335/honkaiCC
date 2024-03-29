#include <unordered_map>
#include <iostream>
#include <cassert>
#include "../../midend/IR/IRGraph.h"
using namespace std;
#ifndef STORMY_HARDWARE
#define STORMY_HARDWARE
class RegisterManager;
enum
{
    /// @brief 被调用者保存寄存器
    CALLEE_SAVED,
    /// @brief 调用者保存寄存器
    CALLER_SAVED,
    /// @brief 其他寄存器
    RESERVED
} RegisterType;

class Area
{
    public:
    /// @brief 分配栈空间的最小偏移量（字节）
    uint32_t minAddress;
    /// @brief 分配栈空间的最大偏移量（字节）
    uint32_t maxAddress;
    /// @brief 当前可分配的偏移
    uint32_t tempOffset;
    /// @brief 构造函数
    /// @param minAddress 
    /// @param maxAddress 

    virtual int getTargetOffset(const RawValueP &value) const { return 0;}
};

class ValueArea : public Area
{
public:
    /// @brief RawValue和栈空间的对应关系
    unordered_map<RawValueP, int> StackManager;
    /// @brief 获取value的存储偏移量
    /// @param value 
    /// @return 
    int getTargetOffset(const RawValueP &value) const override;

    bool IsMemory(const RawValueP &value) { return StackManager.find(value)!= StackManager.end();}

    int StackAlloc(const RawValueP &value) {
        if(tempOffset > maxAddress) assert(0);
        else {
            StackManager.insert(pair<RawValueP, int>(value, tempOffset));
            tempOffset += 4;
            return StackManager.at(value);
        }
    }
};

class RegisterArea : public Area
{
public:
    /// @brief Register和栈空间的对应关系
    unordered_map<int, int> StackManager;
    /// @brief 构造函数
    /// @param min 
    /// @param max 

    void LoadRegister(int reg); 

    void SaveRegister(int reg);
};

class MemoryManager
{
public:
    /// @brief 栈分配的空间大小
    int StackSize;
    /// @brief 局部变量
    ValueArea localArea;
    /// @brief 保存寄存器
    RegisterArea reserveArea; // 这个打算给个固定值，
    /// @brief 参数区域
    ValueArea argsArea;
    /// @brief 构造函数
    MemoryManager(){
    }
    /// @brief 获取某个值的地址
    /// @param value 
    /// @return 
    int getTargetOffset(const RawValueP &value) { return localArea.getTargetOffset(value);}

    void initStack(int StackLen) {this->StackSize = StackLen;}

    void initArgsArea(int min,int max);

    void initReserveArea(int min,int max);

    void initLocalArea(int min,int max);

    bool IsMemory(const RawValueP &value) {return localArea.IsMemory(value);}

    int StackAlloc(const RawValueP &value) {
       return localArea.StackAlloc(value);
    }

    void LoadRegister(int reg) {
        reserveArea.LoadRegister(reg);
    }

    void SaveRegister(int reg) {
        reserveArea.SaveRegister(reg);
    }
};

class RegisterManager
{
public:
    /// @brief 寄存器堆
    static const char *regs[32];
    /// @brief 寄存器表
    unordered_map<RawValueP, int> registerLook;
    /// @brief 寄存器加锁
    bool RegisterLock[32];
    /// @brief 寄存器已满
    bool RegisterFull;
    /// @brief 未满时，当前可用寄存器
    uint32_t tempRegister;
    /// @brief 构造函数
    RegisterManager() {}

    const char *GetRegister(const RawValueP &value) {
        assert(registerLook.find(value) != registerLook.end());
        int loc = registerLook.at(value);
        return regs[loc];
    }

    bool IsRegister(const RawValueP &value){
        return registerLook.find(value) != registerLook.end();
    }

    void addLockRegister(const RawValueP &value) {
        assert(registerLook.find(value) == registerLook.end());
        int loc = registerLook.at(value);
        RegisterLock[loc] = true;
    }

    void LeaseLockRegister(const RawValueP &value) {
        assert(registerLook.find(value) == registerLook.end());
        int loc = registerLook.at(value);
        RegisterLock[loc] = false;
    }

    void AssignRegister(const RawValueP &value, int loc) {
        registerLook.insert(pair<RawValueP, int>(value, loc));
    }

    void init() {
        for (int i = 0; i < 32; i++)
            RegisterLock[i] = false;
        RegisterFull = false;
        tempRegister = 5;
        registerLook.clear();
    }

    bool IsValid(int loc) { return loc > 4 && loc != 10 && loc != 11 && !RegisterLock[loc] && loc < 32;}
};

class HardwareManager {
    public:
    /// @brief 内存管理
    MemoryManager memoryManager;
    /// @brief 寄存器管理
    RegisterManager registerManager;

    HardwareManager() {
    }

    //这样只需要从Local空间取即可，不需要管其他空间
    int getTargetOffset(const RawValueP &value) { return memoryManager.getTargetOffset(value); }

    bool IsMemory(const RawValueP &value) {return memoryManager.IsMemory(value); }
    
    bool IsRegister(const RawValueP &value) {return registerManager.IsRegister(value);}

    void init(const RawFunctionP &value);

    const char *GetRegister(const RawValueP &value) { return registerManager.GetRegister(value);}

    void addLockRegister(const RawValueP &value) { registerManager.addLockRegister(value);}

    void LeaseLockRegister(const RawValueP &value) { registerManager.LeaseLockRegister(value);}
    //分配指定寄存器
    void AssignRegister(const RawValueP &value,int loc) {registerManager.AssignRegister(value,loc);}

    void LoadFromMemory(const RawValueP &value) ;

    void AllocRegister(const RawValueP &value);

    void StoreReg(int RandSelected);

    bool isValid(int loc) { return registerManager.IsValid(loc);}

    int StackAlloc(const RawValueP &value) { return memoryManager.StackAlloc(value);}

    int getStackSize() {return memoryManager.StackSize;}

    void LoadRegister(int reg) { memoryManager.LoadRegister(reg);}

    void SaveRegister(int reg) { memoryManager.SaveRegister(reg);}
};

/*
方法是添加某些功能使得满足以下条件：
1、 所有的寄存器可以被直接使用
2、 所有的内存直接从local中取
为了满足以下的特点，就需要做以下内容
1、 对于所有的参数，分配a0-a7寄存器和local内存
2、 对于所有被调用者调用寄存器，函数开始时单独存，函数结束后取出
3、 ra寄存器在调用前保存至相应位置
*/





#endif