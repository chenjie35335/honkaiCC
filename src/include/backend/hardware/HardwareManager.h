#include <map>
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
    /// @brief RawValue和栈空间大小的关系
    unordered_map<RawValueP, int> LenTable;
    /// @brief 获取value的存储偏移量
    /// @param value 
    /// @return 
    int getTargetOffset(const RawValueP &value) const override;

    bool IsMemory(const RawValueP &value) { return StackManager.find(value)!= StackManager.end();}
    /// @brief 除此之外，这里还需要添加一个规定长度分配
    /// @param value 
    /// @return 
    int StackAlloc(const RawValueP &value) {
        if(this->tempOffset > this->maxAddress) {
            cerr << "tempOffset " << this->tempOffset  << " exceed the maxAddress " << maxAddress << endl;
            assert(0);
        }
        else {
            
            StackManager.insert(pair<RawValueP, int>(value, this->tempOffset));
            int len = GetLen(value);
            //cout << "alloc tempOffset = " << tempOffset << ", len = " << len << endl;
            this->tempOffset += len;
            return StackManager.at(value);
        }
    }
    /// @brief 保存某个值的长度
    /// @param value 
    /// @param len 
    void SaveLen(const RawValueP value,int len) {
        LenTable.insert(pair<RawValueP, int>(value, len));
    }

    int GetLen(const RawValueP &value) {
        if(LenTable.find(value) != LenTable.end())
        return LenTable.at(value);
        else
        return 8;
    }
};
//可以考虑在计算的时候添一个大小
class RegisterArea : public Area
{
public:
    /// @brief Register和栈空间的对应关系
    unordered_map<int, int> StackManager;
    /// @brief 浮点寄存器和栈空间对应关系
    unordered_map<int,int> FStackManager;

    /// @brief load普通寄存器
    /// @param reg 
    void LoadRegister(int reg); 
    /// @brief 保存普通寄存器
    /// @param reg 
    void SaveRegister(int reg);
    /// @brief load浮点寄存器
    /// @param reg 
    void LoadFRegister(int reg); 
    /// @brief store浮点寄存器
    /// @param reg 
    void SaveFRegister(int reg);
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

    void LoadFRegister(int reg) {
        reserveArea.LoadFRegister(reg);
    }

    void SaveRegister(int reg) {
        reserveArea.SaveRegister(reg);
    }

    void SaveFRegister(int reg) {
        reserveArea.SaveFRegister(reg);
    }

    void SaveLen(const RawValueP value,int len) { localArea.SaveLen(value,len);}

    int GetLen(const RawValueP &value) { return localArea.GetLen(value);}
};

class RegisterManager
{
public:
    /// @brief 寄存器堆
    static const char *regs[32];
    //32位浮点
    static const char *fregs[32];
    /// @brief 调用者保存寄存器
    static const int callerSave[];
    /// @brief 被调用者保存寄存器
    static const int calleeSave[];
    /// @brief 调用者保存浮点寄存器
    static const int callerFSave[];
    /// @brief 被调用者保存浮点寄存器
    static const int calleeFSave[];
    /// @brief Class中的Name域
    int IntName[32];
    /// @brief Class中的Next域
    int IntNext[32];
    /// @brief Class中的Free域
    bool IntFree[32];
    /// @brief Class中的空闲栈
    stack<int> IntStack;
    /// @brief FClass中的Name域
    int FloatName[32];
    /// @brief FClass中的Next域
    int FloatNext[32];
    /// @brief FClass中的Free域
    bool FloatFree[32];
    /// @brief FClass中的空闲栈
    stack<int> FloatStack;
    /// @brief 构造函数
    RegisterManager() {}

    bool IsIntInitFree(int reg) {
        return !((reg >= 10 && reg <= 17) || (reg < 6));
    }

    bool IsFloatInitFree(int reg) {
        return !(reg >= 10 && reg <= 17);
    }

    void init() {
        for(int i = 0; i < 32; i++) {
            IntName[i] = -1;
            IntNext[i] = -1;
            IntFree[i] = true;
            FloatName[i] = -1;
            FloatNext[i] = -1;
            FloatFree[i] = true;
            if(IsIntInitFree(i))  IntStack.push(i);
            if(IsFloatInitFree(i)) FloatStack.push(i);
        }
    }

};

class HardwareManager {
    public:
    /// @brief 内存管理
    MemoryManager memoryManager;
    /// @brief 寄存器管理
    RegisterManager registerManager;
    /// @brief 值编号
    map<RawValue*,int> ValueToIndex;
    /// @brief 编号表
    map<int, RawValue*> IndexToValue;

    HardwareManager() {
    }

    const char *GetRegister(int reg, int tag){
        if(tag == RTT_FLOAT) return RegisterManager::fregs[reg];
        else return RegisterManager::regs[reg];
    }
    //这样只需要从Local空间取即可，不需要管其他空间
    int getTargetOffset(const RawValueP &value) { return memoryManager.getTargetOffset(value); }

    bool IsMemory(const RawValueP &value) {return memoryManager.IsMemory(value); }

    void init(const RawFunctionP &value);

    void AlterNext(int reg,int tag,int target);

    int Ensure(int vr,int tag);

    int AllocRegister(int vr, int tag);

    int AssignRegister(int vr, int reg, int tag);

    void FreeRegister(int reg, int tag);

    void spill(int RandSelected,int type);

    int StackAlloc(const RawValueP &value) { return memoryManager.StackAlloc(value);}

    int getStackSize() {return memoryManager.StackSize;}

    void LoadRegister(int reg) { memoryManager.LoadRegister(reg);}

    void LoadFRegister(int reg) { memoryManager.LoadFRegister(reg);}

    void SaveRegister(int reg) { memoryManager.SaveRegister(reg);}

    void SaveFRegister(int reg) { memoryManager.SaveFRegister(reg);}

    void SaveLen(const RawValueP value,int len) { memoryManager.SaveLen(value,len);}

    int GetLen(const RawValueP &value) {return memoryManager.GetLen(value);}

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

int calBaseLen(const RawValueP &value);



#endif