#include "../../../include/midend/IR/IRGraph.h"
#include <string>
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/hardwareManager.h"
#include <cassert>
#include <iostream>
//处理load运算
void Visit(const RawLoad &data, const RawValueP &value) {
    const auto &src = data.src;
    if(!IsMemory(src)) {
        assert(0);
    }
    else {
        AllocRegister(value);
        const char *TargetReg = GetRegister(value);
        int srcAddress = getTargetOffset(src); 
        cout << "  lw  " << TargetReg << ", " << srcAddress << "(sp)" << endl;
    }
}
//处理store运算
void Visit(const RawStore &data, const RawValueP &value) {
    const auto &src = data.value;
    const auto &dest= data.dest;
    if(!IsMemory(dest)) assert(0);
    else {
        const char *SrcReg = GetRegister(src);
        int srcAddress = getTargetOffset(dest);
        cout << "  sw  " << SrcReg << ", " << srcAddress << "(sp)" << endl;
    }
}

//处理二进制运算
void Visit(const RawBinary &data,const RawValueP &value) {
    const auto &lhs = data.lhs;
    const auto &rhs = data.rhs;
    const auto &op  = data.op;
    Visit(lhs);Visit(rhs);
    addLockRegister(lhs);addLockRegister(rhs);
    AllocRegister(value);
    LeaseLockRegister(lhs);LeaseLockRegister(rhs);
    const char *LhsRegister = GetRegister(lhs);
    const char *RhsRegister = GetRegister(rhs);
    const char *ValueRegister = GetRegister(value);
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
    const char *CondRegister = GetRegister(data.cond);
    const char *TrueBB = data.true_bb->name;
    const char *FalseBB = data.false_bb->name;
    cout << "  bnez  " << CondRegister << ",  " << TrueBB << endl;
    cout << "  j  " << FalseBB << endl;
}

//处理jump运算
void Visit(const RawJump &data, const RawValueP &value){
    const char *TargetBB = data.target->name;
    cout << "  j  " << TargetBB << endl;
}



//这个Value是重点，如果value已经被分配了寄存器，直接返回
//如果存在内存当中，调用loadreg后直接返回
//如果这个处于未分配时，这时应该是遍历的时候访问的，分配内存和寄存器
//这个Visit的方法就是要将RawValue值存到寄存器中，至于具体如何访问无需知道
void Visit(const RawValueP &value) {    
    if(IsRegister(value) || IsMemory(value)) {
        return;
    }  
    else {
    const auto& kind = value->value;
    switch(kind.tag) {
    case RVT_RETURN: {
        const auto& ret = kind.data.ret.value; 
        Visit(ret);
        const char *RetRegister = GetRegister(ret);
        cout << "  mv   a0, "<< RetRegister << endl;
        cout << "  addi sp, sp, 256" <<  endl;
        cout << "  ret" << endl;
        break;
    }
    case RVT_INTEGER: {
        const auto& integer = kind.data.integer.value;
        if(integer == 0) {
            AllocX0(value);
        } else {
            AllocRegister(value);
            const char *reg = GetRegister(value);
            cout << "  li   "  <<  reg  << ", "  << integer << endl;
        }
        break;
    }
    case RVT_BINARY: {
        const auto &binary = kind.data.binary;
        Visit(binary,value);
        break;
    }
    case RVT_ALLOC: {
        StackAlloc(value); 
        break;
    }
    case RVT_LOAD: {
        const auto &load = kind.data.load;
        Visit(load,value);
        break;
    }
    case RVT_STORE: {
        const auto &store = kind.data.store;
        Visit(store,value);
        break;
    }
    case RVT_BRANCH: {
        const auto &branch = kind.data.branch;
        Visit(branch,value);
    }
    case RVT_JUMP: {
        const auto &jump = kind.data.jump;
        Visit(jump,value);
    }
    default:
        assert(false);
    }
}
}

// Visit RawBlock
void Visit(const RawBasicBlockP &bb){
    //const &RawSlice *value = bb->insts;
     Visit(bb->insts);
} 
// Visit RawFunction
void Visit(const RawFunctionP &func)
{
         printf("  .globl %s\n",func->name);
         RegisterManagerAlloc();
         ManagerAlloc(255);
         printf("%s:\n",func->name);
         cout << "  addi sp, sp, -256" <<  endl;
         Visit(func->bbs);
       
}
//Visit RawSlice
void Visit(const RawSlice &slice){
    for(size_t i = 0; i < slice.len; i++) {
        auto ptr = slice.buffer[i];
        switch(slice.kind) {
            case RSK_FUNCTION:
                Visit(reinterpret_cast<RawFunctionP>(ptr));
                break;
            case RSK_BASICBLOCK:
                Visit(reinterpret_cast<RawBasicBlockP>(ptr));break;
            case RSK_BASICVALUE:
                Visit(reinterpret_cast<RawValueP>(ptr));break;
            default:
                assert(false);
        }
    }
}

void generateASM(const RawProgramme& value) {
    cout << "  .text" << endl;
    Visit(value.Funcs);
}









