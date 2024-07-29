#include "../../../include/midend/IR/IRGraph.h"
#include <string>
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include <cassert>
#include <cstring>
#include <iostream>
int32_t convert(float number)
{
    union {
        float f;
        int i;
    } u;
    u.f = number;
    return u.i;
}

HardwareManager hardware;
//处理load运算，由于我们在类型那里处理的调整，这里可能需要多加一个分类讨论
void Visit(const RawLoad &data, const RawValueP &value) {
    
    const auto &src = data.src;
    auto srcPointerTy = src->ty->pointer.base;
    if(src->value.tag == RVT_GLOBAL) {
        hardware.AllocRegister(value);
        const char *TargetReg = hardware.GetRegister(value);
        if(srcPointerTy->tag == RTT_FLOAT)
            cout << "  flw  " << TargetReg << ", " << src->name << ", t0" <<  endl;
        else {
            cout << "  lw  " << TargetReg << ", " << src->name << endl;
        }
    } else if(src->value.tag == RVT_ALLOC){
        hardware.AllocRegister(value);
        const char *TargetReg = hardware.GetRegister(value);
        int srcAddress = hardware.getTargetOffset(src); //这里有点好，直接跳过了visit过程
        if(srcAddress > 2047) {
            cout << "  li   " << "t0, " << srcAddress << endl;
            cout << "  add  " << "t0, sp, t0" << endl;
            if(srcPointerTy->tag == RTT_INT32)
                cout << "  lw  " <<  TargetReg << ", " << 0 << "(t0)" << endl;
            else if(srcPointerTy->tag == RTT_FLOAT) 
                cout << "  flw  " <<  TargetReg << ", " << 0 << "(t0)" << endl;
            else 
                cout << "  ld  " <<  TargetReg << ", " << 0 << "(t0)" << endl; 
        } else {
            if(srcPointerTy->tag == RTT_INT32)
                cout << "  lw   " << TargetReg << ", " << srcAddress << "(sp)" << endl;
            else if(srcPointerTy->tag == RTT_FLOAT) 
                cout << "  flw  " <<  TargetReg << ", " << srcAddress << "(sp)" << endl;
            else 
                cout << "  ld  " <<  TargetReg << ", " << srcAddress << "(sp)" << endl; 
            }
        } else if(src->value.tag == RVT_GET_ELEMENT || src->value.tag == RVT_GET_PTR){
            Visit(src);
            hardware.addLockRegister(src);
            hardware.AllocRegister(value);
            const char *TargetReg = hardware.GetRegister(value);
            const char *ElementReg = hardware.GetRegister(src);
            auto ValueTag = value->ty->tag;
            if(ValueTag == RTT_FLOAT)
                cout << "  flw  " << TargetReg << ", " << 0 << '(' << ElementReg << ')' << endl;
            else  
                cout << "  lw  " << TargetReg << ", " << 0 << '(' << ElementReg << ')' << endl;
            hardware.LeaseLockRegister(src);
    } else assert(0);
}

//全局处理aggregate类型
void Visit(const RawAggregate &aggregate) {
    auto &elements = aggregate.elements;
    for(auto element : elements) {
        auto elementTag = element->value.tag;
        if(elementTag == RVT_AGGREGATE) {
            Visit(element->value.aggregate);
        } else if(elementTag == RVT_INTEGER) {
            auto value = element->value.integer.value;
            cout << "  .word " << value << endl;
        } else if(elementTag == RVT_FLOAT){
            auto value = element->value.floatNumber.value;
            int str = convert(value);
            cout << "  .word " << str << endl;
        }
        else {
            assert(0);
        }
    }
    //cout << endl;
}

void Visit(const RawStore &data, const RawValueP &value) {
    // cout << "handle store" << endl;
    //cout << endl;
    const auto &src = data.value;
    const auto &dest= data.dest;
    if(dest->value.tag == RVT_GLOBAL) {
        Visit(src);
        hardware.addLockRegister(src);
        hardware.AllocRegister(dest);
        hardware.LeaseLockRegister(src);
        const char *SrcReg = hardware.GetRegister(src);
        const char * DestReg = hardware.GetRegister(dest);
        if(src->ty->tag == RTT_FLOAT) 
            cout << "  fsw  " << SrcReg << ", " << dest->name << ", "<< DestReg << endl;
        else
            cout << "  sw  " << SrcReg << ", " << dest->name << ", "<< DestReg << endl;
    } else if(dest->value.tag == RVT_ALLOC){
        Visit(src);
        const char *SrcReg = hardware.GetRegister(src);
        int srcAddress = hardware.getTargetOffset(dest);
        auto destPointerTy = dest->ty->pointer.base;
        if(srcAddress > 2047) {
            cout << "  li   " << "t0, " << srcAddress << endl;
            cout << "  add  " << "t0, sp, t0" << endl;
            if(destPointerTy->tag == RTT_INT32)
                cout << "  sw  " <<  SrcReg << ", " << 0 << "(t0)" << endl;
            else if(destPointerTy->tag == RTT_FLOAT){
                cout << "  fsw  " <<  SrcReg << ", " << 0 << "(t0)" << endl;
            } 
            else 
                cout << "  sd  " <<  SrcReg << ", " << 0 << "(t0)" << endl; 
        } else {
            if(destPointerTy->tag == RTT_INT32)
                cout << "  sw   " << SrcReg << ", " << srcAddress << "(sp)" << endl;
            else if(destPointerTy->tag == RTT_FLOAT){
                cout << "  fsw  " <<  SrcReg << ", " << srcAddress << "(sp)" << endl;
            } 
            else 
                cout << "  sd  " <<  SrcReg << ", " << srcAddress << "(sp)" << endl; 
            }
    } else if(dest->value.tag == RVT_GET_ELEMENT || dest->value.tag == RVT_GET_PTR) {
        hardware.addLockRegister(dest);
        Visit(src);
        hardware.LeaseLockRegister(dest);
        const char *SrcReg = hardware.GetRegister(src);
        const char *ElementReg = hardware.GetRegister(dest);
        auto srcTag = src->ty->tag;
        if(srcTag == RTT_FLOAT)
            cout << "  fsw  " << SrcReg << ", " << 0 << '(' << ElementReg << ')' << endl;
        else  // int
            cout << "  sw  " << SrcReg << ", " << 0 << '(' << ElementReg << ')' << endl;
    } else assert(0);
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

    //release
    hardware.LeaseLockRegister(lhs);
    hardware.LeaseLockRegister(rhs);
    //这里需要根据类型判断他是在哪个寄存器里面
    const char *LhsRegister;
    const char *RhsRegister;
    const char *ValueRegister;
    //lhs
    LhsRegister = hardware.GetRegister(lhs);
    //rhs
    RhsRegister = hardware.GetRegister(rhs);
    //value
    ValueRegister = hardware.GetRegister(value);
    switch(op) {
        case RBO_ADD:
            cout << "  addw  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_SUB:
            cout << "  subw  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
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
            cout << "  mulw  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_DIV:
            cout << "  divw  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_MOD:
            cout << "  remw  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_LT:
            cout << "  sext.w  " << LhsRegister << ", " << LhsRegister << endl;
            cout << "  sext.w  " << RhsRegister << ", " << RhsRegister << endl;
            cout << "  slt  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_GT:
            cout << "  sext.w  " << LhsRegister << ", " << LhsRegister << endl;
            cout << "  sext.w  " << RhsRegister << ", " << RhsRegister << endl;
            cout << "  slt  " <<ValueRegister<<", "<< RhsRegister << ", " << LhsRegister <<endl;
            break;
        case RBO_GE:
            cout << "  sext.w  " << LhsRegister << ", " << LhsRegister << endl;
            cout << "  sext.w  " << RhsRegister << ", " << RhsRegister << endl;
            cout << "  slt  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            cout << "  seqz " << ValueRegister <<", "<< ValueRegister  <<endl;
            break;
        case RBO_LE:
            cout << "  sext.w  " << LhsRegister << ", " << LhsRegister << endl;
            cout << "  sext.w  " << RhsRegister << ", " << RhsRegister << endl;
            cout << "  sgt  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            cout << "  seqz " << ValueRegister <<", "<< ValueRegister  <<endl;
            break;   
        case RBO_OR:{
            cout << "  or   " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        }       
        case RBO_XOR:
            cout << "  xor  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_AND:
            cout << "  and  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_FADD:
            cout << "  fadd.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_FSUB:
            cout << "  fsub.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_FMUL:
            cout << "  fmul.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_FDIV:
            cout << "  fdiv.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_FGE://no
            cout << "  fge.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_FGT://no
            cout << "  fgt.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_FLE:
            cout << "  fle.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_FLT:
            cout << "  flt.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            break;
        case RBO_NOT_FEQ: //no
            cout << "  feq.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
            cout << "  not  " << ValueRegister << ", " << ValueRegister << endl;
            break;
        case RBO_FEQ:
            cout << "  feq.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
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
    auto &params = data.args;
    for(int i = 0; i < params.size(); i++) {
        auto ptr = reinterpret_cast<RawValueP>(params[i]);
        Visit(ptr);
        if(i < 8) {
            const char *reg = hardware.GetRegister(ptr);
            hardware.StoreReg(10+i,ptr->ty->tag);
            if(ptr->ty->tag == RTT_FLOAT)
                cout << "  fmv.s  " << RegisterManager::fregs[10+i] << ", " << reg << endl;
            else 
                cout << "  mv  " << RegisterManager::regs[10+i] << ", " << reg << endl;
        } else {
            const char *reg = hardware.GetRegister(ptr);
            int offset = (i-8)*8;
            if(offset > 2047) {
                cout << "  li  t0, " << offset << endl;
                cout << "  add  t0, t0, sp" << endl;
                if(ptr->ty->tag == RTT_FLOAT)
                    cout << "  fsd  " << reg << ", " << 0 << "(t0)" << endl;
                else 
                    cout << "  sd  " << reg << ", " << 0 << "(t0)" << endl;
            } else{
                if(ptr->ty->tag == RTT_FLOAT)
                    cout << "  fsd  " << reg << ", " << offset << "(sp)" << endl;
                else 
                    cout << "  sd  " << reg << ", " << offset << "(sp)" << endl;
            }
        }
    }
     for(int i = 0;i < 7;i++) {
         hardware.StoreReg(RegisterManager::callerSave[i],RTT_INT32);
     }
     for(int i = 0;i < 12;i++) {
         hardware.StoreReg(RegisterManager::callerFSave[i],RTT_FLOAT);
     }
    cout<<"  call "<<data.callee->name<<endl;
    hardware.AllocRegister(value);
    hardware.StackAlloc(value);
    const char *retReg = hardware.GetRegister(value);
    if(value->ty->tag == RTT_FLOAT)
        cout << "  fmv.s  " << retReg << ", fa0" << endl;
    else
        cout << "  mv  " << retReg << ", a0" << endl;
}
//这里不需要分配寄存器，直接默认在a的几个寄存器中，读出来后直接分配栈空间
void Visit(const RawFuncArgs &data,const RawValueP &value) {
    int index = data.index;
    hardware.StackAlloc(value);
    if(index < 8) 
        hardware.AssignRegister(value,10+index);//这里直接分配a寄存器
    else {
        // cout << "funcargs" << endl;
        hardware.AllocRegister(value);
        const char *reg = hardware.GetRegister(value);
        int StackSize = hardware.getStackSize();
        int offset = StackSize+(index-8)*8;
        if(offset > 2047) {
            cout << "  li  t0, " << offset << endl;
            cout << "  add t0, t0, sp" << endl;
            if(value->ty->tag == RTT_FLOAT)
                cout << "  fld  " << reg << ", " << 0 << "(t0)" << endl;
            else 
                cout << "  ld  " << reg << ", " << 0 << "(t0)" << endl;
        } else {
            if(value->ty->tag == RTT_FLOAT)
                cout << "  fld  " << reg << ", " << offset << "(sp)" << endl;
            else
                cout << "  ld  " << reg << ", " << offset << "(sp)" << endl;
        }
    }
}

//这里需要写一个浮点IEEE754转换函数


//处理Global Alloc变量
void Visit(const RawGlobal &data,const RawValueP &value) {
    int tag = data.Init->value.tag;
    if(tag == RVT_INTEGER) {
        int Init = data.Init->value.integer.value;
        if(Init == 0) {
            cout << "  .zero 4" << endl;
        } else {
            cout << "  .word " << Init << endl; 
        }
    }  else if(tag == RVT_FLOAT){
        float Init = data.Init->value.floatNumber.value;
        int32_t str = convert(Init);
        cout << "  .word " << str << endl; 
    } else if(tag == RVT_ZEROINIT) {
       int len =  calBaseLen(value);
       cout << "  .zero " << len << endl;
    } else if(tag == RVT_AGGREGATE) {
        Visit(data.Init->value.aggregate);
    } else assert(0);
}

void Visit(const RawGetPtr &data, const RawValueP &value) {
    //cout << "parse getptr" << endl;
    auto src = data.src;
    auto index = data.index;
    const char *srcAddrReg;
    Visit(src);
    srcAddrReg = hardware.GetRegister(src);
    hardware.addLockRegister(src);
    Visit(index);
    hardware.addLockRegister(index);
    const char *IndexReg = hardware.GetRegister(index);
    int elementLen = calBaseLen(src);
    hardware.AllocRegister(value);
    hardware.LeaseLockRegister(src);
    hardware.LeaseLockRegister(index);
    const char *ptrReg = hardware.GetRegister(value);
    if(elementLen == 4) {
        cout << "  slli " << IndexReg << ", " << IndexReg << ", " << 2 << endl;
    } else {
        cout << "  li  " << ptrReg << ", " << elementLen << endl;
        cout << "  mul " << IndexReg << ", " << IndexReg << ", " << ptrReg << endl;
    }
    cout << "  add  " << ptrReg << ", " << srcAddrReg << ", " << IndexReg << endl;
}

//处理getElement类型变量,
//这里的核心要务是不能Visit！这里要单独处理src和index
//对于src,首先判断是不是寄存器类型，如果是的话直接使用寄存器的值作为地址基址
//如果不是寄存器类型，是global或者是其他类型，就需要给这个值分配寄存器用于存储地址值，然后进行下一步操作
//最后对于每一个getelement变量赋予一个寄存器的值（貌似需要加锁）
void Visit(const RawGetElement &data,const RawValueP &value) {
     //cout << "Visit GetElement" << endl;
     auto &src = data.src;
     auto &index = data.index;
     const char *srcAddrReg;
     if(src->value.tag == RVT_GLOBAL) {
        hardware.AllocRegister(src);
        srcAddrReg = hardware.GetRegister(src);
        cout << "  la  " << srcAddrReg << ", " << src->name << endl;
     } else if(src->value.tag == RVT_ALLOC){//这里包含了参数值的问题
        hardware.AllocRegister(src);
        srcAddrReg = hardware.GetRegister(src);
        int srcAddr = hardware.getTargetOffset(src);
        cout << "  li  " << srcAddrReg << ", " << srcAddr << endl; 
        cout << "  add " << srcAddrReg << ", sp, " << srcAddrReg << endl;  
     } else if(src->value.tag == RVT_GET_ELEMENT || src->value.tag == RVT_GET_PTR){ 
        Visit(src);
        srcAddrReg = hardware.GetRegister(src);
     }
     hardware.addLockRegister(src);
    //  cout << "visit index" << endl;
     Visit(index);
     hardware.addLockRegister(index);
     const char *IndexReg = hardware.GetRegister(index);
     //这个地方应该乘的是单个元素的长度，这里先解决的是一维数组的问题
     //cout << "calptrlen = " << calPtrLen(src) << ", elementlen" << (src->ty->data.array.len) << endl;
     int elementLen = calBaseLen(src)/(src->ty->pointer.base->array.len);
     hardware.AllocRegister(value);
     hardware.LeaseLockRegister(src);
     hardware.LeaseLockRegister(index);
     const char *ptrReg = hardware.GetRegister(value);
     if(elementLen == 4) {
        cout << "  slli " << IndexReg << ", " << IndexReg << ", " << 2 << endl;
    } else {
        cout << "  li  " << ptrReg << ", " << elementLen << endl;
        cout << "  mul " << IndexReg << ", " << IndexReg << ", " << ptrReg << endl;
    }
    cout << "  add  " << ptrReg << ", " << srcAddrReg << ", " << IndexReg << endl;
    cout << endl;
}

void Visit(const RawTriple &data,const RawValueP &value) 
{
    const auto &hs1 = data.hs1;
    const auto &hs2 = data.hs2;
    const auto &hs3 = data.hs3;
    const auto &op  = data.op;
    Visit(hs1);
    hardware.addLockRegister(hs1);

    Visit(hs2);
    hardware.addLockRegister(hs2);

    Visit(hs3);
    hardware.addLockRegister(hs3);

    hardware.AllocRegister(value);

    //release
    hardware.LeaseLockRegister(hs1);
    hardware.LeaseLockRegister(hs2);
    hardware.LeaseLockRegister(hs3);
    //这里需要根据类型判断他是在哪个寄存器里面
    const char *hs1Register;
    const char *hs2Register;
    const char *hs3Register;
    const char *ValueRegister;
    //hs1
    hs1Register = hardware.GetRegister(hs1);
    //hs2
    hs2Register = hardware.GetRegister(hs2);
    //hs3
    hs3Register = hardware.GetRegister(hs3);
    //value
    ValueRegister = hardware.GetRegister(value);
    switch(op) {
        case RTO_FMADD:{
            cout << "  fmadd.s  " << ValueRegister << ", " << hs1Register << ", " << hs2Register << ", " << hs3Register << endl;
            break;
        }
        case RTO_FMSUB:{
            cout << "  fmsub.s  " << ValueRegister << ", " << hs1Register << ", " << hs2Register << ", " << hs3Register << endl;
            break;
        }
        case RTO_FNMADD:{
            break;
        }
        case RTO_FNMSUB:{
            break;
        }
        default: assert(0);
    }
}

/// visit convert
void Visit(const RawConvert &data, const RawValueP &value)
{
    //最近舍入模式“四舍五入” rne
    //向0舍入 rtz
    // fcvt.w.s  word to single
    // fcvt.s.w  single to word
    // convert dest, src, mode
    auto SrcType = data.src->ty->tag;
    if(SrcType == RTT_INT32){
        const char *srcReg;
        Visit(data.src);
        srcReg = hardware.GetRegister(data.src);
        hardware.addLockRegister(data.src);
        hardware.AllocRegister(value);
        hardware.LeaseLockRegister(data.src);
        const char *TReg = hardware.GetRegister(value);
        cout<<"  fcvt.s.w " << TReg << ", " << srcReg << ", " << "rtz" << endl;
    } else if(SrcType == RTT_FLOAT) {
        const char*srcReg;
        Visit(data.src);
        srcReg = hardware.GetRegister(data.src);
        hardware.addLockRegister(data.src);
        hardware.AllocRegister(value);
        hardware.LeaseLockRegister(data.src);
        const char *TReg = hardware.GetRegister(value);
        cout<< "  fcvt.w.s " << TReg << ", "<< srcReg << ", " << "rtz" << endl;
    }
}


//这个Value是重点，如果value已经被分配了寄存器，直接返回
//如果存在内存当中，调用loadreg后直接返回
//如果这个处于未分配时，这时应该是遍历的时候访问的，分配内存和寄存器
//这个Visit的方法就是要将RawValue值存到寄存器中，至于具体如何访问无需知道
//现在可能需要做一个约定：凡是遇到全局变量或者函数参数
void Visit(const RawValueP &value) {    
    const auto& kind = value->value;
    if(hardware.IsRegister(value)) {
        return;
    }  else if(hardware.IsMemory(value)) {
        hardware.LoadFromMemory(value);
        return;
    }
    else {
    switch(kind.tag) {
    case RVT_RETURN: {
        const auto& ret = kind.ret.value; 
        if(ret != nullptr) {
        Visit(ret);
        const char *RetRegister = hardware.GetRegister(ret);
        if(ret->ty->tag != RTT_FLOAT && strcmp(RetRegister,"a0")) {
            cout << "  mv   a0, "<< RetRegister << endl;
        }
        if(ret->ty->tag == RTT_FLOAT && strcmp(RetRegister,"fa0")) {
            cout << "  fmv.s   fa0, "<< RetRegister << endl;
        }
        }
        hardware.LoadRegister(1);
        for(int i = 0; i < 12;i++) {
            hardware.LoadRegister(RegisterManager::calleeSave[i]);
        }
        for(int i = 0; i < 12;i++) {
            hardware.LoadFRegister(RegisterManager::calleeFSave[i]);
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
        const auto& integer = kind.integer.value;
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
    case RVT_FLOAT:{
        const auto& floatNumber = kind.floatNumber.value;
        hardware.AllocRegister(value);
        const char *reg = hardware.GetRegister(value);
        int32_t str = convert(floatNumber);
        cout << "  li  " << "t0 " << ", " << str << endl;
        cout << "  fmv.w.x " <<  reg << ", " << "t0" << endl;
        // if(floatNumber >= 0 && floatNumber <= 0) { //判断浮点数为0，看起来很蠢
            // hardware.AssignRegister(value,0);
        // } else {
            // hardware.AllocFRegister(value);
            // const char *reg = hardware.GetFRegister(value);
            //浮点数的load操作很复杂，这里先不处理
            // int32_t str = convert(floatNumber);
            // Visit(str);
            // cout << "  lui  "  <<  reg  << ", "  << "LC" << LC_Number[current_LC] << endl;
        // }
        break;
    }
    case RVT_BINARY: {
        const auto &binary = kind.binary;
        Visit(binary,value);
        //cout << endl;
        break;
    }
    case RVT_ALLOC: {
        hardware.StackAlloc(value); 
        break;
    }
    case RVT_LOAD: {
        const auto &load = kind.load;
        Visit(load,value);
        break;
    }
    case RVT_STORE: {
        const auto &store = kind.store;
        Visit(store,value);
        break;
    }
    case RVT_BRANCH: {
        const auto &branch = kind.branch;
        Visit(branch,value);
        break;
    }
    case RVT_JUMP: {
        const auto &jump = kind.jump;
        Visit(jump,value);
        break;
    }
    case RVT_CALL: {
        const auto &call = kind.call;
        Visit(call,value);
        break;
    }
    case RVT_FUNC_ARGS:{
        const auto &args = kind.funcArgs;
        Visit(args,value);
        break;
    }
    case RVT_GLOBAL: {
        const auto &global = kind.global;
        cout << "  .globl " << value->name <<  endl;
        cout << value->name << ":" << endl;
        Visit(global,value);
        cout << endl;
        break;
    }
    case RVT_AGGREGATE: {//这个貌似没有单独出现，都是依附于alloc之类的存在的
        break;
    }
    case RVT_GET_PTR: {
        const auto &getptr = kind.getptr;
        Visit(getptr,value);
        break;
    }
    case RVT_GET_ELEMENT: {//对于这种element类型的变量，直接分配空间就行
    //貌似对于多维数组来说，基地址已经存进寄存器中了，其他的只需要调用就行
        const auto &getElement = kind.getelement;
        Visit(getElement,value);
        break;
    }
    case RVT_CONVERT: {
        const auto &convert = kind.Convert;
        Visit(convert,value);
        break;
    }
    case RVT_TRIPE: {
        const auto &triple = kind.triple;
        Visit(triple,value);
        break;
    }
    default:{
        cerr << "unknown kind: " << kind.tag << endl;
        assert(false); 
    }
    }
}
    //cout <<"End Visit kind" << kind.tag << endl;
}

// Visit RawBlock
void Visit(const RawBasicBlockP &bb){
     if(strcmp(bb->name,"entry")){
     cout << endl;
     cout << bb->name << ":" << endl;
     }
     auto &insts = bb->inst;
     for(auto inst : insts)
     Visit(inst);
} 

void Visit_bb(const RawBasicBlockP &bb) {
    hardware.registerManager.PushNewLook();
    Visit(bb);
    auto &domains = bb->domains;
    for(auto domain : domains) {
        Visit_bb(domain);
    }
    hardware.registerManager.PopLook();
}

// Visit RawFunction
void Visit(const RawFunctionP &func)
{
        auto &bbs = func->basicblock;
        auto &params = func->params;
        int bbsLen = bbs.size();
        //cerr << "bbsLen " << bbsLen << endl;
        if(bbsLen != 0) {
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
         for(int i = 0; i < 12;i++) {
            hardware.SaveFRegister(RegisterManager::calleeFSave[i]);
         }
        for(auto param : params)
         Visit(param);
        auto entryBB = *bbs.begin();
        Visit_bb(entryBB);
        // for(auto bb : bbs)
        //  Visit(bb);
        cout << endl;
        }
}

void generateASM(RawProgramme *& programme) {
    cout << "  .data" << endl;
    auto &values = programme->values;
    auto &funcs = programme->funcs;
    for(auto value : values)
        Visit(value);
    cout << "  .text" << endl;
    for(auto func : funcs)
        Visit(func);
}