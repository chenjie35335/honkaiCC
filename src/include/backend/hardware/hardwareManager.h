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
const char *regs[] = {
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
void StackAlloc(const RawValue *value);

/// @brief 分配MemoryManager对象
void ManagerAlloc(int maxSize);

/// @brief 分配RegisterManager对象
void RegisterManagerAlloc();

/// @brief 给RawValue分配寄存器
/// @param value 
/// @return 
const char * AllocRegister(const RawValue *value);

#endif