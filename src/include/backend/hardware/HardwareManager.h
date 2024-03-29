#include <unordered_map>
#include "../../midend/IR/IRGraph.h"
#ifndef STORMY_HARDWARE
#define STORMY_HARDWARE
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
    Area(uint32_t min,uint32_t max) : minAddress(min),maxAddress(max),tempOffset(min){}
};

class ValueArea : public Area
{
public:
    /// @brief RawValue和栈空间的对应关系
    unordered_map<RawValueP, int> StackManager;
    /// @brief 构造函数
    /// @param min 
    /// @param max 
    ValueArea(uint32_t min,uint32_t max) : Area(min,max) {}
};

class RegisterArea : public Area
{
public:
    /// @brief Register和栈空间的对应关系
    unordered_map<int, int> StackManager;
    /// @brief 构造函数
    /// @param min 
    /// @param max 
    RegisterArea(uint32_t min,uint32_t max) : Area(min,max) {}
};

class MemoryManager
{
public:
    /// @brief 局部变量
    Area* localArea;
    /// @brief 保存寄存器
    Area* reserveArea; // 这个打算给个固定值，
    /// @brief 参数区域
    Area* argsArea;
    /// @brief 构造函数
    MemoryManager() {
        localArea = new ValueArea(0,0);
        reserveArea = new RegisterArea(0,0);
        localArea = new ValueArea(0,0);
    }
    /// @brief 析构函数
    ~MemoryManager() {
        delete localArea;
        delete reserveArea;
        delete argsArea;
    }

} MemoryManager;

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
    RegisterManager() {
        for (int i = 0; i < 32; i++)
            RegisterLock[i] = false;
        RegisterFull = false;
        tempRegister = 5;
    }
};

const char *RegisterManager::regs[32] = {
    "x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

class HardwareManager {
    public:
    /// @brief 内存管理
    class MemoryManager *memoryManager;
    /// @brief 寄存器管理
    class RegisterManager *registerManager;

    HardwareManager() {
        memoryManager = new class MemoryManager();
        registerManager = new class RegisterManager();
    }
};

// 存在寄存器中的参数该如何处理？这里助教直接在这里弄了一个store还是可以的，
// 但是这里要我们手动store了，也就是说LocalArea也需要预留给参数的空间
// 调用者保存的寄存器肯定和RawValue挂钩，所以，直接可以保存LocalArea
// 但是调用保存寄存器不知道是否分配，需要建立寄存器和内存地址之间的关系
// 关于那几个传参数的，访问参数的时候会和其绑定，无需关心
// 这些偏移相对于sp就行
#endif