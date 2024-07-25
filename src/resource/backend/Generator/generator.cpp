#include "../../../include/midend/IR/IRGraph.h"
#include <string>
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include <cassert>
#include <cstring>
#include <iostream>
#define EXP  0.00000001
static int LC_Number[100];
static int current_LC = 0;
//visit const LC
void Visit(int number)
{
    LC_Number[current_LC] = number;
    current_LC++;
}

void Print_LC()
{
    for(int i = 0; i < current_LC ;i++){
        cout<<".LC:"<<i;
        cout<<": "<<endl;
        cout<<"   .word  "<<LC_Number[i]<<endl;
    }
}

int32_t convert(float number)
{
    //联合体的神奇功能
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
    if(src->value.tag == RVT_GLOBAL) {
        hardware.AllocRegister(value);
        const char *TargetReg = hardware.GetRegister(value);
        cout << "  la  " << TargetReg << ", " << src->name << endl;
        if(value->value.tag == RVT_FLOAT){
            cout << "  flw  " << TargetReg << ", " << 0 << '(' << TargetReg << ')' << endl;
        } else {
            cout << "  lw  " << TargetReg << ", " << 0 << '(' << TargetReg << ')' << endl;
        }
        
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
    for(auto element : elements) {
        auto elementTag = element->value.tag;
        if(elementTag == RVT_AGGREGATE) {
            Visit(element->value.aggregate,src,dest,index);
        } 
        else if(elementTag == RVT_INTEGER) {
           auto value = element->value.integer.value;
           //int offset = index *4;
           cout << "  li  " << src << ", " << value << endl;
           cout << "  sw  " << src << ", " << 0 << '(' << dest << ')' << endl;
           cout << "  addi " << dest <<  ", " << dest << ", " << 4 << endl;
           index++;
        } //add float
        else if(elementTag == RVT_FLOAT) {
           auto value = element->value.floatNumber.value;
           //int offset = index *4;
           //貌似要用到HI-LO寄存器
           int32_t str = convert(value);
           Visit(str);
           cout<<"  lui "<< src <<", "<< "%hi(.LC"<< current_LC << ")" <<endl;
           cout<<"  flw  "<< "f" << src <<", %lo(.LC"<< current_LC << ")" <<endl;  //浮点寄存器
           cout<<"  fsw  "<< "f" << src << ", " << "-" << index*4 << "(" << dest << ")" <<endl;
           index++;
        } else {
            Visit(element);
            cout<< "element"<<endl;
            const char *elemReg = hardware.GetRegister(element);
            cout << "  mv  " << src << ", " << elemReg << endl;
            cout << "  sw  " << src << ", " << 0 << '(' << dest << ')' << endl;
            cout << "  addi " << dest <<  ", " << dest << ", " << 4 << endl;
        }
    }
    //cout << endl;
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
        } //add float 
        else if(elementTag == RVT_FLOAT){
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
        hardware.addLockRegister(dest);
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
        Visit(src->value.aggregate,SrcReg,DestReg,index);
        hardware.LeaseLockRegister(dest);
        hardware.LeaseLockRegister(src);
    } else {
        cerr << "src tag:" << src->value.tag << ", dest tag: " << dest->value.tag << endl;
        cerr << "an aggregate value can't be assigned to a non-aggregate value" << endl;
        assert(0);
    }
}

//处理二进制运算
//添加浮点寄存器，貌似是最简单的
void Visit(const RawBinary &data,const RawValueP &value) {
    const auto &lhs = data.lhs;
    const auto &rhs = data.rhs;
    const auto &op  = data.op;
    Visit(lhs);
    if(lhs->value.tag == RVT_FLOAT)
    hardware.addLockFRegister(lhs);
    else hardware.addLockRegister(lhs);

    Visit(rhs);
    if(rhs->value.tag == RVT_FLOAT)
    hardware.addLockFRegister(rhs);
    else hardware.addLockRegister(rhs);

    if(value->value.tag == RVT_FLOAT)
    hardware.AllocFRegister(value);
    else hardware.AllocRegister(value);

    //release
    int flag1 = rhs->value.tag == RVT_FLOAT;
    int flag2 = lhs->value.tag == RVT_FLOAT;
    if(!flag1)
    hardware.LeaseLockRegister(lhs);
    else hardware.LeaseLockFRegister(lhs);
    if(!flag2)
    hardware.LeaseLockRegister(rhs);
    else 
    hardware.LeaseLockFRegister(rhs);
    //这里需要根据类型判断他是在哪个寄存器里面
    int lhsType = lhs->value.tag;
    int rhsType = rhs->value.tag;
    int valueType = value->value.tag;
    const char *LhsRegister;
    const char *RhsRegister;
    const char *ValueRegister;
    //lhs
    if(lhsType == RVT_FLOAT) {
        LhsRegister = hardware.GetFRegister(lhs);
    } else{
        LhsRegister = hardware.GetRegister(lhs);
    }
    //rhs
    if(rhsType == RVT_FLOAT){
        RhsRegister = hardware.GetFRegister(rhs);
    } else {
        RhsRegister = hardware.GetRegister(rhs);
    }
    //value
    if(valueType == RVT_FLOAT){
        ValueRegister = hardware.GetFRegister(value);
    } else {
        ValueRegister = hardware.GetRegister(value);
    }
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
        //float
        //浮点寄存器的选择策略需要改
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
        //s表示单精度，我们只用实现float
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
            cout << "  fneq.s  " <<ValueRegister<<", "<< LhsRegister << ", " << RhsRegister <<endl;
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
            hardware.StoreReg(10+i);
            const char* paramReg = RegisterManager::regs[10+i];
            cout << "  mv  " << paramReg << ", " << reg << endl;
        } else {
            const char *reg = hardware.GetRegister(ptr);
            int offset = (i-8)*8;
            cout << "  sd  " << reg << ", " << offset << "(sp)" << endl;
        }
    }
     for(int i = 0;i < 7;i++) {
         hardware.StoreReg(RegisterManager::callerSave[i]);
     }
    cout<<"  call "<<data.callee->name<<endl;
    if(value->ty->tag == RTT_INT32){
        hardware.AllocRegister(value);
        hardware.StackAlloc(value);
        cout << "callee "<< endl;
        const char *retReg = hardware.GetRegister(value);
        cout << "  mv  " << retReg << ", a0" << endl;
    } else if ( value->ty->tag == RTT_FLOAT){
        //应该分配给浮点寄存器
        hardware.AllocFRegister(value);
        hardware.StackAlloc(value);
        const char *retReg = hardware.GetFRegister(value);
        cout << "  mv  " << retReg << ", a0" << endl;
        hardware.AssignFRegister(value,10);
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
        cout << "  sd  " << reg << ", " << addr << "(sp)" << endl;
        //如果访问这个RawFuncArgs的话，单纯是分配内存空间
        //但是存在问题，这里会把这个内存空间
    } else {
        int srcAddr = stackLen+(index-8)*8;
        cout << "  ld  " << "t0"<< "," << srcAddr << "(sp)" << endl;
        cout << "  sd  " << "t0"<< "," << addr << "(sp)" << endl;
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
    } 
    //add float
    else if(tag == RVT_FLOAT){
        float Init = data.Init->value.floatNumber.value;
        if(Init >= 0.0 && Init <= 0.0) {
            cout << "  .zero 4" << endl;
        } else {
            int32_t str = convert(Init);
            cout << "  .word " << str << endl; 
        }
    }
    
     else if(tag == RVT_ZEROINIT) {
       int len =  calPtrLen(value);
       cout << "  .zero " << len << endl;
    } else if(tag == RVT_AGGREGATE) {
        Visit(data.Init->value.aggregate);
    } else assert(0);
}

void Visit(const RawGetPtr &data, const RawValueP &value) {
    cout << "parse getptr" << endl;
    auto src = data.src;
    auto index = data.index;
    const char *srcAddrReg;
    Visit(src);
    srcAddrReg = hardware.GetRegister(src);
    hardware.addLockRegister(src);
    Visit(index);
    hardware.addLockRegister(index);
    const char *IndexReg = hardware.GetRegister(index);
    int elementLen = calPtrLen(src);
    if(value->ty->tag == RVT_FLOAT){
        hardware.AllocFRegister(value);
    }else {
        hardware.AllocRegister(value);
    }
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
     cout << "Visit GetElement" << endl;
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
     } else if(src->value.tag == RVT_FLOAT){
        srcAddrReg = hardware.GetFRegister(src);
     } else{ //rvt_int32
        srcAddrReg = hardware.GetRegister(src);
     }
     hardware.addLockRegister(src);
     Visit(index);
     hardware.addLockRegister(index);
     const char *IndexReg = hardware.GetRegister(index);
     //这个地方应该乘的是单个元素的长度，这里先解决的是一维数组的问题
     //cout << "calptrlen = " << calPtrLen(src) << ", elementlen" << (src->ty->data.array.len) << endl;
     int elementLen = calPtrLen(src)/(src->ty->pointer.base->array.len);
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


/// visit convert
void Visit(const RawConvert &data, const RawValueP &value)
{
    //最近舍入模式“四舍五入” rne
    //向0舍入 rtz
    // fcvt.w.s  word to single
    // fcvt.s.w  single to word
    // convert dest, src, mode
    cout << "visit convert" << endl;
    auto SrcType = data.src->ty->tag;
    if(SrcType == RTT_INT32){
        cout<< "  convert begin for ITF" << endl;
        const char *srcReg;
        //value->value.Convert.src;
        //data.src->value.Convert.src
        srcReg = hardware.GetRegister(data.src->value.Convert.src);
        RawValueP newF = new RawValue();
        hardware.AllocFRegister(newF);
        const char *TReg = hardware.GetFRegister(newF);
        cout<<"  fcvt.s.w" << TReg << ", " << srcReg << ", " << "rtz" << endl;
    } else if(SrcType == RTT_FLOAT) {
        cout<< "  convert begin for FTI" << endl;
        const char*srcReg;
        srcReg = hardware.GetFRegister(data.src->value.Convert.src);
        RawValueP newI = new RawValue();
        hardware.AllocRegister(newI);
        const char *TReg = hardware.GetRegister(newI);
        cout<< "  fcvt.w.s" << TReg << ", "<< srcReg << ", " << "rtz" << endl;
    }
    
}


//这个Value是重点，如果value已经被分配了寄存器，直接返回
//如果存在内存当中，调用loadreg后直接返回
//如果这个处于未分配时，这时应该是遍历的时候访问的，分配内存和寄存器
//这个Visit的方法就是要将RawValue值存到寄存器中，至于具体如何访问无需知道
//现在可能需要做一个约定：凡是遇到全局变量或者函数参数
void Visit(const RawValueP &value) {  
    //cout << "visit total " << endl;  
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
        const auto& ret = kind.ret.value; 
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
        const auto& integer = kind.integer.value;
        if(integer == 0) {
            hardware.AssignRegister(value,0);
        } else {
            //cout << "wrong begin"<<endl;
            hardware.AllocRegister(value);
            const char *reg = hardware.GetRegister(value);
            cout << "  li   "  <<  reg  << ", "  << integer << endl;
        }
        //cout << endl;
        break;
    }
    case RVT_FLOAT:{
        const auto& floatNumber = kind.floatNumber.value;
        if(floatNumber >= -EXP && floatNumber <= EXP) { //判断浮点数为0，看起来很蠢
            hardware.AssignFRegister(value,0);
        } else {
            hardware.AllocFRegister(value);
            const char *reg = hardware.GetFRegister(value);
            //浮点数的load操作很复杂，这里先不处理
            int32_t str = convert(floatNumber);
            Visit(str);
            cout << "  lui  "  <<  reg  << ", "  << "LC" << LC_Number[current_LC] << endl;
        }
        break;
    }
    case RVT_BINARY: {
        const auto &binary = kind.binary;
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
        //cout << "aggregate handler" << endl;
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
        const auto &getConvert = kind.Convert;
        cout<<"visit begin" <<endl;
        Visit(getConvert,value);
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
     auto &insts = bb->inst;
     for(auto inst : insts)
     Visit(inst);
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
         }//这个地方还是很好处理的，毕竟t0在这里不怎么用
        for(auto param : params)
         Visit(param);
        for(auto bb : bbs)
         Visit(bb);
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
    Print_LC();
}