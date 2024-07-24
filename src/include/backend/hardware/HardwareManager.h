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
    /// @brief 构造函数
    /// @param min 
    /// @param max 

    void LoadRegister(int reg); 
    void SaveRegister(int reg);
    //fregs
    void LoadFRegister(int reg); 
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

typedef enum {
    FSRM_ROUND_TO_NEAREST,
    FSRM_ROUND_TO_ZERO,
    FSRM_ROUND_TO_POSITIVE_INFINITY,
    FSRM_ROUND_TO_NEGATIVE_INFINITY
} frm_mode_t;

typedef struct {
    unsigned int fp_control;
    unsigned int fp_status;
    unsigned int fp_tag;
    unsigned int fp_ip;
    unsigned int fp_cs;
    unsigned int fp_dp;
    unsigned int fp_ds;
    unsigned int fp_op;
} fsr_t;

class RegisterManager
{
public:
    /// @brief 寄存器堆
    static const char *regs[32];
    //32位浮点
    static const char *fregs[32];  
    /// frm 浮点舍入模式寄存器
    frm_mode_t frmReg;
    /// fsr 浮点控制状态寄存器
    fsr_t fsrReg;
    //难点在于什么时候用这些寄存器
    /// @brief 调用者保存寄存器
    static const int callerSave[];
    /// @brief 被调用者保存寄存器
    static const int calleeSave[];
    /// @brief 寄存器表
    map<RawValueP, int> registerLook;
    /// @brief 寄存器表栈
    stack<map<RawValueP, int>> registerStack;
    /// @brief 浮点寄存器表
    unordered_map<RawValueP, int> FregisterLook; //fregs
    /// @brief 寄存器加锁
    bool RegisterLock[32];
    bool FRegisterLock[32];//fregs
    /// @brief 寄存器已满
    bool RegisterFull;
    /// @brief 寄存器是否满的栈
    stack<bool> registerFullStack;

    bool FRegisterFull; //fregs
    /// @brief 未满时，当前可用寄存器
    uint32_t tempRegister;
    /// @brief 未满时，可用寄存器栈
    stack<uint32_t> tempRegisterStack;
    /// @brief 未满时，当前可用浮点存器
    uint32_t tempFRegister;
    /// @brief 构造函数
    RegisterManager() {}

    void PushNewLook() {
        registerStack.push(registerLook);
        registerFullStack.push(RegisterFull);
        tempRegisterStack.push(tempRegister);
    }

    void PopLook() {
        assert(!registerStack.empty() && !registerFullStack.empty() && !tempRegisterStack.empty());
        registerLook = registerStack.top();
        RegisterFull = registerFullStack.top();
        tempRegister = tempRegisterStack.top();
        registerStack.pop();
        registerFullStack.pop();
        tempRegisterStack.pop();
    }

    const char *GetRegister(const RawValueP &value) {
        assert(registerLook.find(value) != registerLook.end());
        int loc = registerLook.at(value);
        //cout << "register loc" << loc << endl;
        return regs[loc];
    }
    //fregs
    const char *GetFRegister(const RawValueP &value) {
        assert(registerLook.find(value) != registerLook.end());
        int loc = registerLook.at(value);
        return fregs[loc];
    }

    bool IsRegister(const RawValueP &value){
        return registerLook.find(value) != registerLook.end();
    }
    //fregs
    bool IsFRegister(const RawValueP &value){
        return FregisterLook.find(value) != FregisterLook.end();
    }

    void addLockRegister(const RawValueP &value) {
        assert(registerLook.find(value) != registerLook.end());
        int loc = registerLook[value];
        RegisterLock[loc] = true;
    }
    //fregs
    void addLockFRegister(const RawValueP &value) {
        assert(FregisterLook.find(value) != FregisterLook.end());
        int loc = FregisterLook.at(value);
        FRegisterLock[loc] = true;
    }

    void LeaseLockRegister(const RawValueP &value) {
        assert(registerLook.find(value) != registerLook.end());
        int loc = registerLook.at(value);
        RegisterLock[loc] = false;
    }
    //fregs
    void LeaseLockFRegister(const RawValueP &value) {
        assert(FregisterLook.find(value) != FregisterLook.end());
        int loc = FregisterLook.at(value);
        RegisterLock[loc] = false;
    }

    void AssignRegister(const RawValueP &value, int loc) {
        registerLook.insert(pair<RawValueP, int>(value, loc));
    }
    //fregs
    void AssignFRegister(const RawValueP &value, int loc) {
        FregisterLook.insert(pair<RawValueP, int>(value, loc));
    }


    void init() {
        for (int i = 0; i < 32; i++){
            RegisterLock[i] = false;
            FRegisterLock[i] = false;
        }
        FRegisterFull = false;
        RegisterFull = false;
        tempRegister = 6;
        tempFRegister = 5;
        registerLook.clear();
        while(!registerStack.empty()) {
            registerStack.pop();
        }
        //FregisterLook.clear();
    }

    bool IsValid(int loc) { 
        //cout << "Register loc" << loc <<  endl;
        return ((loc > 5 && loc < 10) || (loc > 17 && loc < 32)) && !RegisterLock[loc] ;
    }
    //for fregs 不知道加的对不对 //或者在后面再改一个
    bool IsValidF(int loc) { return ((loc > 4 && loc < 10) || (loc > 17 && loc < 32)) && !FRegisterLock[loc] ;}
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
    const char *GetFRegister(const RawValueP &value) { return registerManager.GetFRegister(value);}

    void addLockRegister(const RawValueP &value) { registerManager.addLockRegister(value);}
    void addLockFRegister(const RawValueP &value) { registerManager.addLockFRegister(value);}

    void LeaseLockRegister(const RawValueP &value) { registerManager.LeaseLockRegister(value);}
    void LeaseLockFRegister(const RawValueP &value) { registerManager.LeaseLockFRegister(value);}
    //分配指定寄存器
    void AssignRegister(const RawValueP &value,int loc) {registerManager.AssignRegister(value,loc);}
    void AssignFRegister(const RawValueP &value,int loc) {registerManager.AssignFRegister(value,loc);}

    void LoadFromMemory(const RawValueP &value) ;

    void AllocRegister(const RawValueP &value);

    void StoreReg(int RandSelected);

    bool isValid(int loc) { return registerManager.IsValid(loc);}

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