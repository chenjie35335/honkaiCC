#include "../../../include/midend/IR/IRGraph.h"
#include <string>
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include <cassert>
#include <cstring>
#include <iostream>

HardwareManager hardware;
//处理load运算，由于我们在类型那里处理的调整，这里可能需要多加一个分类讨论
void Visit(const RawLoad &data, const RawValueP &value) {
    const auto &src = data.src;
    if(src->value.tag == RVT_GLOBAL) {
        hardware.AllocRegister(value);
        const char *TargetReg = hardware.GetRegister(value);
        cout << "  la  " << TargetReg << ", " << src->name << endl;
        cout << "  lw  " << TargetReg << ", " << 0 << '(' << TargetReg << ')' << endl;
    } else if(hardware.IsMemory(src)){
        hardware.AllocRegister(value);
        const char *TargetReg = hardware.GetRegister(value);
        int srcAddress = hardware.getTargetOffset(src); //这里有点好，直接跳过了visit过程
        cout << "  lw  " << TargetReg << ", " << srcAddress << "(sp)" << endl;
    } else if(src->value.tag == RVT_GET_ELEMENT || src->value.tag == RVT_GET_PTR){
        hardware.addLockRegister(src);
        hardware.AllocRegister(value);
        const char *TargetReg = hardware.GetRegister(value);
        const char *ElementReg = hardware.GetRegister(src);
        cout << "  lw  " << TargetReg << ", " << 0 << '(' << ElementReg << ')' << endl;
        hardware.LeaseLockRegister(src);
    } else assert(0);
}
//处理aggregate类型//首先是先计算地址然后store？
void Visit(const RawAggregate &aggregate,const char *src,const char *dest,int &index) {
    auto &elements = aggregate.elements;
    for(int i = 0;i < elements.len;i++) {
        auto element = (RawValueP) elements.buffer[i];
        auto elementTag = element->value.tag;
        if(elementTag == RVT_AGGREGATE) {
            Visit(element->value.data.aggregate,src,dest,index);
        } else if(elementTag == RVT_INTEGER) {
            auto value = element->value.data.integer.value;
            //int offset = index *4;
            cout << "  li  " << src << ", " << value << endl;
            cout << "  sw  " << src << ", " << 0 << '(' << dest << ')' << endl;
            cout << "  addi " << dest <<  ", " << dest << ", " << 4 << endl;
            index++;
        } else {
            assert(0);
        }
    }
    //cout << endl;
}

//全局处理aggregate类型
void Visit(const RawAggregate &aggregate) {
    auto &elements = aggregate.elements;
    for(int i = 0;i < elements.len;i++) {
        auto element = (RawValueP) elements.buffer[i];
        auto elementTag = element->value.tag;
        if(elementTag == RVT_AGGREGATE) {
            Visit(element->value.data.aggregate);
        } else if(elementTag == RVT_INTEGER) {
            auto value = element->value.data.integer.value;
            cout << "  .word " << value << endl;
        } else {
            assert(0);
        }
    }
    //cout << endl;
}

//那里还是存在bug
//处理store运算
//store这个地方就是一个巨大的隐患：我们给这个分配了寄存器
void Visit(const RawStore &data, const RawValueP &value) {//store这个地方的加锁问题一直很大
    //cout << endl;
    const auto &src = data.value;
    const auto &dest= data.dest;
    if(src->value.tag != RVT_AGGREGATE && dest->ty->tag != RTT_ARRAY) {
    if(dest->value.tag == RVT_GLOBAL) {
        Visit(src);
        hardware.addLockRegister(src);
        hardware.AllocRegister(dest);
        hardware.LeaseLockRegister(src);
        const char *SrcReg = hardware.GetRegister(src);
        const char * DestReg = hardware.GetRegister(dest);
        cout << "  la  " << DestReg << ", " << dest->name << endl;
        cout << "  sw  " << SrcReg << ", " << 0 << '(' << DestReg << ')' << endl;
        //首先全局变量会被当成寄存器使用吗？
    } else if(hardware.IsMemory(dest)){
        Visit(src);
        const char *SrcReg = hardware.GetRegister(src);
        int srcAddress = hardware.getTargetOffset(dest);
        cout << "  sw  " << SrcReg << ", " << srcAddress << "(sp)" << endl;
    } else if(dest->value.tag == RVT_GET_ELEMENT || dest->value.tag == RVT_GET_PTR) {
        hardware.addLockRegister(dest);
        Visit(src);
        hardware.LeaseLockRegister(dest);
        const char *SrcReg = hardware.GetRegister(src);
        const char *ElementReg = hardware.GetRegister(dest);
        cout << "  sw  " << SrcReg << ", " << 0 << '(' << ElementReg << ')' << endl;
    } else assert(0);
    } else if(src->value.tag == RVT_AGGREGATE && dest->ty->tag == RTT_ARRAY){//这里貌似还是没有什么好办法
        hardware.AllocRegister(src);
        hardware.addLockRegister(src);
        hardware.AllocRegister(dest);
        hardware.LeaseLockRegister(src);
        const char *SrcReg = hardware.GetRegister(src);
        const char *DestReg = hardware.GetRegister(dest);//可以考虑把这个DestReg分配给中间变量
        int srcAddress = hardware.getTargetOffset(dest);
        if(srcAddress >= 2048) {
            cout << "  li  " << DestReg << ", " << srcAddress << endl;
            cout << "  add " << DestReg << ", sp ," << DestReg << endl;
        } else { 
            cout << "  addi  " << DestReg << ", " << "sp" << ", " << srcAddress << endl;
        }
        int index = 0;
        Visit(src->value.data.aggregate,SrcReg,DestReg,index);
    } else {
        cerr << "src tag:" << src->value.tag << ", dest tag: " << dest->value.tag << endl;
        cerr << "an aggregate value can't be assigned to a non-aggregate value" << endl;
        assert(0);
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
     for(int i = 0;i < 7;i++) {
         hardware.StoreReg(RegisterManager::callerSave[i]);
     }
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
        cout << "  sw  " << reg << ", " << addr << "(sp)" << endl;//如果访问这个RawFuncArgs的话，单纯是分配内存空间
        //但是存在问题，这里会把这个内存空间
    } else {
        int srcAddr = stackLen+(index-8)*4;
        cout << "  lw  " << "t0"<< "," << srcAddr << "(sp)" << endl;
        cout << "  sw  " << "t0"<< "," << addr << "(sp)" << endl;
    }
}
//处理Global Alloc变量
void Visit(const RawGlobal &data,const RawValueP &value) {
    int tag = data.Init->value.tag;
    if(tag == RVT_INTEGER) {
        int Init = data.Init->value.data.integer.value;
        if(Init == 0) {
            cout << "  .zero 4" << endl;
        } else {
            cout << "  .word " << Init << endl; 
        }
    } else if(tag == RVT_ZEROINIT) {
       int len =  calPtrLen(value);
       cout << "  .zero " << len << endl;
    } else if(tag == RVT_AGGREGATE) {
        Visit(data.Init->value.data.aggregate);
    } else assert(0);
}

void Visit(const RawGetPtr &data, const RawValueP &value) {
    //cout << "parse getptr" << endl;
    auto &src = data.src;
    auto &index = data.index;
    const char *srcAddrReg;
    Visit(src);
    srcAddrReg = hardware.GetRegister(src);
    hardware.addLockRegister(src);
    Visit(index);
    hardware.addLockRegister(index);
    const char *IndexReg = hardware.GetRegister(index);
    int elementLen = calPtrLen(src);
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
     } else if(hardware.IsMemory(src)){//这里包含了参数值的问题
        hardware.AllocRegister(src);
        srcAddrReg = hardware.GetRegister(src);
        int srcAddr = hardware.getTargetOffset(src);
        cout << "  li  " << srcAddrReg << ", " << srcAddr << endl; 
        cout << "  add " << srcAddrReg << ", sp, " << srcAddrReg << endl;  
     } else {
        srcAddrReg = hardware.GetRegister(src);
     }
     hardware.addLockRegister(src);
     Visit(index);
     hardware.addLockRegister(index);
     const char *IndexReg = hardware.GetRegister(index);
     //这个地方应该乘的是单个元素的长度，这里先解决的是一维数组的问题
     //cout << "calptrlen = " << calPtrLen(src) << ", elementlen" << (src->ty->data.array.len) << endl;
     int elementLen = calPtrLen(src)/(src->ty->data.array.len);
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

//这个Value是重点，如果value已经被分配了寄存器，直接返回
//如果存在内存当中，调用loadreg后直接返回
//如果这个处于未分配时，这时应该是遍历的时候访问的，分配内存和寄存器
//这个Visit的方法就是要将RawValue值存到寄存器中，至于具体如何访问无需知道
//现在可能需要做一个约定：凡是遇到全局变量或者函数参数
void Visit(const RawValueP &value) {    
    const auto& kind = value->value;
    if(hardware.IsRegister(value)) {
        if(kind.tag == RVT_FUNC_ARGS) {
            hardware.AllocRegister(value);
            hardware.LoadFromMemory(value);
        }
        return;
    }  else if(hardware.IsMemory(value)) {
        hardware.LoadFromMemory(value);
        return;
    }
    else {
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
        //cout << "alloc" << endl;
        hardware.StackAlloc(value); 
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
    case RVT_GLOBAL: {
        const auto &global = kind.data.global;
        cout << "  .globl " << value->name <<  endl;
        cout << value->name << ":" << endl;
        Visit(global,value);
        cout << endl;
        break;
    }
    case RVT_AGGREGATE: {//这个貌似没有单独出现，都是依附于alloc之类的存在的
        //cout << "aggregate handler" << endl;
        break;
    }
    case RVT_GET_PTR: {
        const auto &getptr = kind.data.getptr;
        Visit(getptr,value);
        break;
    }
    case RVT_GET_ELEMENT: {//对于这种element类型的变量，直接分配空间就行
    //貌似对于多维数组来说，基地址已经存进寄存器中了，其他的只需要调用就行
        const auto &getElement = kind.data.getelement;
        Visit(getElement,value);
        break;
    }
    default:{
        cerr << "unknown kind: " << kind.tag << endl;
        assert(false); 
    }
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
        int ParamLen = func->params.len;
        int bbsLen = func->bbs.len;
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
         }//这个地方还是很好处理的，毕竟t0在这里不怎么用
         Visit(func->params);
         Visit(func->bbs);
         cout << endl;
        }
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
    cout << "  .data" << endl;
    Visit(value->Value);
    cout << "  .text" << endl;
    Visit(value->Funcs);
}









