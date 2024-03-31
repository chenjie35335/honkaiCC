#include "../../../include/midend/IR/IRGraph.h"
#include <string>
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include <cassert>
#include <cstring>
#include <iostream>

HardwareManager hardware;
//处理load运算
void Visit(const RawLoad &data, const RawValueP &value) {
    const auto &src = data.src;
    if(!hardware.IsMemory(src)) {
        assert(0);
    }
    else {
        hardware.AllocRegister(value);
        const char *TargetReg = hardware.GetRegister(value);
        int srcAddress = hardware.getTargetOffset(src); //这里有点好，直接跳过了visit过程
        cout << "  lw  " << TargetReg << ", " << srcAddress << "(sp)" << endl;
    }
}
//处理store运算
void Visit(const RawStore &data, const RawValueP &value) {
    const auto &src = data.value;
    const auto &dest= data.dest;
    if(!hardware.IsMemory(dest)) assert(0);
    else {
        const char *SrcReg = hardware.GetRegister(src);
        int srcAddress = hardware.getTargetOffset(dest);
        cout << "  sw  " << SrcReg << ", " << srcAddress << "(sp)" << endl;
    }
}

//处理二进制运算
void Visit(const RawBinary &data,const RawValueP &value) {
    const auto &lhs = data.lhs;
    const auto &rhs = data.rhs;
    const auto &op  = data.op;
    Visit(lhs);
    hardware.addLockRegister(lhs);
    Visit(rhs);
    hardware.addLockRegister(rhs);
    hardware.AllocRegister(value);
    hardware.LeaseLockRegister(lhs);hardware.LeaseLockRegister(rhs);
    const char *LhsRegister = hardware.GetRegister(lhs);
    const char *RhsRegister = hardware.GetRegister(rhs);
    const char *ValueRegister = hardware.GetRegister(value);
    switch(op) {
        case RBO_ADD:
            cout << "  add  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_SUB:
            cout << "  sub  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_EQ:
            cout << "  xor  " << ValueRegister <<", "<< LhsRegister << ", " << RhsRegister <<endl;
            cout << "  seqz " << ValueRegister <<", "<< ValueRegister  <<endl;
            break;
        case RBO_NOT_EQ:
            cout << "  xor  " << ValueRegister <<", "<< LhsRegister << ", " << RhsRegister <<endl;
            cout << "  snez "  << ValueRegister <<", "<< ValueRegister  <<endl;
            break;
        case RBO_MUL:
            cout << "  mul  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_DIV:
            cout << "  div  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_MOD:
            cout << "  rem  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_LT:
            cout << "  slt  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_GT:
            cout << "  slt  " <<ValueRegister<<", "<< RhsRegister << ", " << LhsRegister <<endl;
            break;
        case RBO_GE:
            cout << "  slt  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            cout << "  seqz " << ValueRegister <<", "<< ValueRegister  <<endl;
            break;
        case RBO_LE:
            cout << "  sgt  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            cout << "  seqz " << ValueRegister <<", "<< ValueRegister  <<endl;
            break;   
        case RBO_OR:
            cout << "  or   " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_XOR:
            cout << "  xor  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_AND:
            cout << "  and  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        default: assert(0);
    }
}

//处理branch指令
void Visit(const RawBranch &data, const RawValueP &value){
    Visit(data.cond);
    string CondRegister = hardware.GetRegister(data.cond);
    string TrueBB = data.true_bb->name;
    string FalseBB = data.false_bb->name;
    cout << "  bnez  " << CondRegister << ", " << TrueBB << endl;
    cout << "  j  " << FalseBB << endl;
}

//处理jump运算
void Visit(const RawJump &data, const RawValueP &value){
    string TargetBB = data.target->name;
    cout << "  j  " << TargetBB << endl;
}
//处理RawCall对象，a0和a1寄存器直接不使用
//这里有个严重问题
void Visit(const RawCall &data,const RawValueP &value) {
    cout << endl;
    auto &params = data.args;
    for(int i = 0; i < params.len; i++) {
        auto ptr = reinterpret_cast<RawValueP>(params.buffer[i]);
        Visit(ptr);
        if(i < 8) {
            const char *reg = hardware.GetRegister(ptr);
            hardware.StoreReg(10+i);
            const char* paramReg = RegisterManager::regs[10+i];
            cout << "  mv  " << paramReg << ", " << reg << endl;
        } else {
            const char *reg = hardware.GetRegister(ptr);
            int offset = (i-8)*4;
            cout << "  sw  " << reg << ", " << offset << "(sp)" << endl;
        }
    }
    cout << endl;
     for(int i = 0;i < 7;i++) {
         hardware.StoreReg(RegisterManager::callerSave[i]);
     }
     cout << endl;
    cout<<"  call "<<data.callee->name<<endl;
    if(value->ty->tag == RTT_INT32){
        hardware.AssignRegister(value,10);
    }
}
//这里不需要分配寄存器，直接默认在a的几个寄存器中，读出来后直接分配栈空间
void Visit(const RawFuncArgs &data,const RawValueP &value) {
    int index = data.index;
    int stackLen = hardware.getStackSize();
    int addr = hardware.StackAlloc(value);
    if(index < 8) {
        int regAddr = 10+index;//10是a0号寄存器
        const char *reg = RegisterManager::regs[regAddr];
        cout << "  sw  " << reg << ", " << addr << "(sp)" << endl;
    } else {
        int srcAddr = stackLen+(index-8)*4;
        cout << "  lw  " << "t0"<< "," << srcAddr << "(sp)" << endl;
        cout << "  sw  " << "t0"<< "," << addr << "(sp)" << endl;
    }
}
//这个Value是重点，如果value已经被分配了寄存器，直接返回
//如果存在内存当中，调用loadreg后直接返回
//如果这个处于未分配时，这时应该是遍历的时候访问的，分配内存和寄存器
//这个Visit的方法就是要将RawValue值存到寄存器中，至于具体如何访问无需知道
void Visit(const RawValueP &value) {    
    if(hardware.IsRegister(value)) {
        return;
    }  else if(hardware.IsMemory(value)) {
        hardware.LoadFromMemory(value);
        return;
    }
    else {
    const auto& kind = value->value;
    switch(kind.tag) {
    case RVT_RETURN: {
        const auto& ret = kind.data.ret.value; 
        if(ret != nullptr) {
        Visit(ret);
        const char *RetRegister = hardware.GetRegister(ret);
        if(strcmp(RetRegister,"a0")) {
        cout << "  mv   a0, "<< RetRegister << endl;
        }
        }
        hardware.LoadRegister(1);
        for(int i = 0; i < 12;i++) {
            hardware.LoadRegister(RegisterManager::calleeSave[i]);
        }
        int StackSize = hardware.getStackSize();
        if(StackSize <= 2047) {
        cout << "  addi sp, sp, " << StackSize  <<  endl;
        } else {
        cout << "  li t0, " << StackSize << endl;
        cout << "  add sp, sp, t0" << endl;
        }
        cout << "  ret" << endl;
        break;
    }
    case RVT_INTEGER: {
        const auto& integer = kind.data.integer.value;
        if(integer == 0) {
            hardware.AssignRegister(value,0);
        } else {
            hardware.AllocRegister(value);
            const char *reg = hardware.GetRegister(value);
            cout << "  li   "  <<  reg  << ", "  << integer << endl;
        }
        //cout << endl;
        break;
    }
    case RVT_BINARY: {
        const auto &binary = kind.data.binary;
        Visit(binary,value);
        //cout << endl;
        break;
    }
    case RVT_ALLOC: {
        hardware.StackAlloc(value); 
        //cout << endl;
        break;
    }
    case RVT_LOAD: {
        const auto &load = kind.data.load;
        Visit(load,value);
        //cout << endl;
        break;
    }
    case RVT_STORE: {
        const auto &store = kind.data.store;
        Visit(store,value);
        //cout << endl;
        break;
    }
    case RVT_BRANCH: {
        const auto &branch = kind.data.branch;
        Visit(branch,value);
        break;
        //cout << endl;
    }
    case RVT_JUMP: {
        const auto &jump = kind.data.jump;
        Visit(jump,value);
        break;
        //cout << endl;
    }
    case RVT_CALL: {
        //cout << "parsing call" << endl;
        const auto &call = kind.data.call;
        Visit(call,value);
        break;
    }
    case RVT_FUNC_ARGS:{
        const auto &args = kind.data.funcArgs;
        Visit(args,value);
        break;
    }
    default:
        assert(false);
    }
}
}

// Visit RawBlock
void Visit(const RawBasicBlockP &bb){
     if(strcmp(bb->name,"entry")){
     cout << endl;
     cout << bb->name << ":" << endl;
     }
     Visit(bb->insts);
} 
// Visit RawFunction
void Visit(const RawFunctionP &func)
{
         hardware.init(func);
         printf("  .globl %s\n",func->name);
         printf("%s:\n",func->name);
         int StackSize = hardware.getStackSize();
         if( StackSize <= 2048) {
            cout << "  addi sp, sp, " << -StackSize << endl;
         } else {
            cout << "  li t0, " << -StackSize << endl;
            cout << "  add sp, sp, t0" << endl;
         }
         hardware.SaveRegister(1);
         for(int i =0 ; i < 12;i++) {
            hardware.SaveRegister(RegisterManager::calleeSave[i]);
         }
         Visit(func->params);
         Visit(func->bbs);
         cout << endl;
       
}
//Visit RawSlice
void Visit(const RawSlice &slice){
    for(size_t i = 0; i < slice.len; i++) {
        // cout << "i = " << i << ", len = " << slice.len << endl;
        auto ptr = slice.buffer[i];
        switch(slice.kind) {
            case RSK_FUNCTION:
               // cout << "parsing function" << endl;
                Visit(reinterpret_cast<RawFunctionP>(ptr));
                break;
            case RSK_BASICBLOCK:
                // cout << "parsing BasicBlock" << endl;
                Visit(reinterpret_cast<RawBasicBlockP>(ptr));
                break;
            case RSK_BASICVALUE:
                // cout << "parsing Value" << endl;
                Visit(reinterpret_cast<RawValueP>(ptr));
                break;
            default:
                assert(false);
        }
    }
}

void generateASM(RawProgramme *& value) {
    cout << "  .text" << endl;
    Visit(value->Funcs);
}









