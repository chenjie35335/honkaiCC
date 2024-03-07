#include <unordered_map>
#include "../../midend/IR/IRGraph.h"
#ifndef STORMY_HARDWARE
#define STORMY_HARDWARE
//注意：这里我们只考虑一个函数，所以这里我们只需要关心一个MemoryManager
//之后遇到多函数时候要使用栈式方式管理存储

/// @brief 用于管理栈空间
typedef struct MemoryManager{
    /// @brief RawValue和栈空间的对应关系
    unordered_map <const RawValue *,int> StackManager;
    /// @brief 当前可用的栈地址偏移位置
    uint32_t Offset;
    /// @brief 表示分配栈的最大值
    uint32_t maxSize;
}MemoryManager;

/// @brief 寄存器名称
static const char *regs[] = {
  "x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

/// @brief 寄存器管理（为了方便，这里寄存器分配不使用LRU,之后直接改成图着色或者其他方法）
typedef struct RegisterManager{
    /// @brief 考虑到访问次数的问题，替换的时候直接删除寄存器所在的那一列，插入新列就可以了
    unordered_map <const RawValue*,int> registerLook;
    /// @brief 寄存器锁
    bool RegisterLock[32];
    /// @brief 表示寄存器已满
    bool RegisterFull;
    /// @brief 表示未满时候当前可用寄存器
    uint32_t tempRegister;
}RegisterManager;

/// @brief 用于将RawValue和栈空间绑定
int StackAlloc(const RawValue *value);

/// @brief 分配MemoryManager对象
void ManagerAlloc(int maxSize);

/// @brief 分配RegisterManager对象
void RegisterManagerAlloc();

/// @brief 给RawValue分配寄存器
/// @param value 
/// @return 
void AllocRegister(const RawValueP &value);

/// @brief 从内存中取出值
/// @param value 
/// @return 
void LoadFromMemory(const RawValueP value);

/// @brief 判断是否已经分配内存 
/// @param value 
/// @return 
bool IsMemory(const RawValueP &value);

/// @brief 判断是否已经在寄存器中
/// @param value 
/// @return 
bool IsRegister(const RawValueP &value);

/// @brief 对于任意一个需要中间变量的value来说（也就是说需要别的RawValue访问的变量）一定在内存中有映射
/// 这里找的是已经分配的但是只在内存中RawValue
/// @param value 
/// @return 
bool IsMemory(const RawValueP &value);

/// @brief 给寄存器加锁
/// @param value 
void addLockRegister(const RawValueP &value);

/// @brief 释放寄存器锁
/// @param value 
void LeaseLockRegister(const RawValueP &value);

/// @brief 获取RawValue所在的寄存器的名称
/// @param value 
/// @return 
const char *GetRegister(const RawValueP &value);

/// @brief 分配零号寄存器
/// @param value 
/// @return 
void AllocX0(const RawValueP &value);

/// @brief 获取栈地址
/// @param value 
/// @return 
int getTargetOffset(const RawValueP &value);
#endif