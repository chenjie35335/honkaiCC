#include "../../../include/midend/IR/IRGraph.h"
#include <string>
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <algorithm>
int32_t convert(float number)
{
    union {
        float f;
        int i;
    } u;
    u.f = number;
    return u.i;
}

bool IsNeed(RawValue *value) {
    value->dictIt++;
    if(value->dictIt == value->dict.end()) return false;
    else return true;
}

HardwareManager hardware;
//处理load运算，由于我们在类型那里处理的调整，这里可能需要多加一个分类讨论
void Visit(const RawLoad &data, const RawValueP &value) {
    
    const auto &src = data.src;
    auto srcPointerTy = src->ty->pointer.base;
    if(src->value.tag == RVT_GLOBAL) {
        int ValueReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
        const char *ValueRegister = hardware.GetRegister(ValueReg,value->ty->tag);
        if(value->dict.empty()) hardware.FreeRegister(ValueReg,value->ty->tag);
        else hardware.AlterNext(ValueReg,value->ty->tag,*value->dictIt);
        if(srcPointerTy->tag == RTT_FLOAT)
            cout << "  flw  " << ValueRegister << ", " << src->name << ", t0" <<  endl;
        else {
            cout << "  lw  " << ValueRegister << ", " << src->name << endl;
        }
    } else if(src->value.tag == RVT_ALLOC){
        int ValueReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
        const char *ValueRegister = hardware.GetRegister(ValueReg,value->ty->tag);
        if(value->dict.empty()) hardware.FreeRegister(ValueReg,value->ty->tag);
        else hardware.AlterNext(ValueReg,value->ty->tag,*value->dictIt);
        int srcAddress = hardware.getTargetOffset(src); //这里有点好，直接跳过了visit过程
        if(srcAddress > 2047) {
            cout << "  li   " << "t0, " << srcAddress << endl;
            cout << "  add  " << "t0, sp, t0" << endl;
            if(srcPointerTy->tag == RTT_INT32)
                cout << "  lw  " <<  ValueRegister << ", " << 0 << "(t0)" << endl;
            else if(srcPointerTy->tag == RTT_FLOAT) 
                cout << "  flw  " <<  ValueRegister << ", " << 0 << "(t0)" << endl;
            else 
                cout << "  ld  " <<  ValueRegister << ", " << 0 << "(t0)" << endl; 
        } else {
            if(srcPointerTy->tag == RTT_INT32)
                cout << "  lw   " << ValueRegister << ", " << srcAddress << "(sp)" << endl;
            else if(srcPointerTy->tag == RTT_FLOAT) 
                cout << "  flw  " <<  ValueRegister << ", " << srcAddress << "(sp)" << endl;
            else 
                cout << "  ld  " <<  ValueRegister << ", " << srcAddress << "(sp)" << endl; 
            }
        } else if(src->value.tag == RVT_GET_ELEMENT || src->value.tag == RVT_GET_PTR){
            int SrcReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)src],src->ty->tag);
            if(!IsNeed((RawValue *)src)) hardware.FreeRegister(SrcReg,src->ty->tag);
            else {
                hardware.AlterNext(SrcReg,src->ty->tag,*src->dictIt);
            }
            int ValueReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
            const char *ValueRegister = hardware.GetRegister(ValueReg,value->ty->tag);
            const char *ElementReg = hardware.GetRegister(ValueReg,src->ty->tag);
            if(value->dict.empty()) hardware.FreeRegister(SrcReg,value->ty->tag);
            else hardware.AlterNext(ValueReg,value->ty->tag,*value->dictIt);
            auto ValueTag = value->ty->tag;
            if(ValueTag == RTT_FLOAT)
                cout << "  flw  " << ValueReg << ", " << 0 << '(' << ElementReg << ')' << endl;
            else  
                cout << "  lw  " << ValueReg << ", " << 0 << '(' << ElementReg << ')' << endl;
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
        int srcReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)src], src->ty->tag);
        //hardware.AllocRegister(dest);
        const char *SrcReg = hardware.GetRegister(srcReg,src->ty->tag);
        if(!IsNeed((RawValue *)src)) hardware.FreeRegister(srcReg,src->ty->tag);
        else {
            hardware.AlterNext(srcReg,src->ty->tag,*src->dictIt);
        }
        //const char * DestReg = hardware.GetRegister(dest);
        if(src->ty->tag == RTT_FLOAT) 
            cout << "  fsw  " << SrcReg << ", " << dest->name << ", t0"<< endl;
        else
            cout << "  sw  " << SrcReg << ", " << dest->name << ", t0" << endl;
    } else if(dest->value.tag == RVT_ALLOC){
        int srcReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)src], src->ty->tag);
        const char *SrcReg = hardware.GetRegister(srcReg,src->ty->tag);
        int srcAddress = hardware.getTargetOffset(dest);
        if(!IsNeed((RawValue *)src)) hardware.FreeRegister(srcReg,src->ty->tag);
        else {
            hardware.AlterNext(srcReg,src->ty->tag,*src->dictIt);
        }
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
        int srcReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)src], src->ty->tag);
        //hardware.AllocRegister(dest);
        const char *SrcReg = hardware.GetRegister(srcReg,src->ty->tag);
        if(!IsNeed((RawValue *)src)) hardware.FreeRegister(srcReg,src->ty->tag);
        else {
            hardware.AlterNext(srcReg,src->ty->tag,*src->dictIt);
        }
        int destReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)dest], dest->ty->tag);
        //hardware.AllocRegister(dest);
        const char *DestReg = hardware.GetRegister(destReg,dest->ty->tag);
        if(!IsNeed((RawValue *)dest)) hardware.FreeRegister(destReg,dest->ty->tag);
        else {
            hardware.AlterNext(destReg,dest->ty->tag,*dest->dictIt);
        }
        // Visit(src);
        // const char *SrcReg = hardware.GetRegister(src);
        // const char *ElementReg = hardware.GetRegister(dest);
        auto srcTag = src->ty->tag;
        if(srcTag == RTT_FLOAT)
            cout << "  fsw  " << SrcReg << ", " << 0 << '(' << DestReg << ')' << endl;
        else  // int
            cout << "  sw  " << SrcReg << ", " << 0 << '(' << DestReg << ')' << endl;
    } else assert(0);
}


//处理二进制运算
void Visit(const RawBinary &data,const RawValueP &value) {
    const auto &lhs = data.lhs;
    const auto &rhs = data.rhs;
    const auto &op  = data.op;
    const char *LhsRegister;
    const char *RhsRegister;
    const char *ValueRegister;
    int LhsReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)lhs],lhs->ty->tag);
    int RhsReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)rhs],rhs->ty->tag);
    if(!IsNeed((RawValue *)lhs)) hardware.FreeRegister(LhsReg,lhs->ty->tag);
    else {
        hardware.AlterNext(LhsReg,lhs->ty->tag,*lhs->dictIt);
    }
    if(!IsNeed((RawValue *)rhs)) hardware.FreeRegister(RhsReg,rhs->ty->tag);
    else {
        hardware.AlterNext(RhsReg,rhs->ty->tag,*rhs->dictIt);
    }
    int ValueReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);

    //这里需要根据类型判断他是在哪个寄存器里面

    //lhs
    LhsRegister = hardware.GetRegister(LhsReg,lhs->ty->tag);
    //rhs
    RhsRegister = hardware.GetRegister(RhsReg,rhs->ty->tag);
    //value
    ValueRegister = hardware.GetRegister(ValueReg,value->ty->tag);

    if(value->dict.empty()) hardware.FreeRegister(ValueReg,value->ty->tag);
    else hardware.AlterNext(ValueReg,value->ty->tag,*value->dictIt);
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
    auto cond = (RawValue *)data.cond;
    int CondReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)cond],cond->ty->tag);
    if(!IsNeed((RawValue *)cond)) hardware.FreeRegister(CondReg,cond->ty->tag);
    else {
        hardware.AlterNext(CondReg,cond->ty->tag,*cond->dictIt);
    }
    const char * CondRegister = hardware.GetRegister(CondReg,cond->ty->tag);
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
        if(i < 8) {
            int PtrReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)ptr], ptr->ty->tag);
            const char *reg = hardware.GetRegister(PtrReg,ptr->ty->tag);
            if(!IsNeed((RawValue *)ptr)) hardware.FreeRegister(PtrReg,ptr->ty->tag);
            else {
                hardware.AlterNext(PtrReg,ptr->ty->tag,*ptr->dictIt);
            }
            //hardware.spill(10+i,ptr->ty->tag);
            if(ptr->ty->tag == RTT_FLOAT)
                cout << "  fmv.s  " << RegisterManager::fregs[10+i] << ", " << reg << endl;
            else 
                cout << "  mv  " << RegisterManager::regs[10+i] << ", " << reg << endl;
        } else {
            int PtrReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)ptr], ptr->ty->tag);
            const char *reg = hardware.GetRegister(PtrReg,ptr->ty->tag);
            if(!IsNeed((RawValue *)ptr)) hardware.FreeRegister(PtrReg,ptr->ty->tag);
            else {
                hardware.AlterNext(PtrReg,ptr->ty->tag,*ptr->dictIt);
            }
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
         hardware.spill(RegisterManager::callerSave[i],RTT_INT32);
     }
     for(int i = 0;i < 12;i++) {
         hardware.spill(RegisterManager::callerFSave[i],RTT_FLOAT);
     }
    cout<<"  call "<<data.callee->name<<endl;
    if(value->ty->tag != RTT_UNIT) {
    int RetReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
    //hardware.StackAlloc(value);
    const char *retReg = hardware.GetRegister(RetReg,value->ty->tag);
    if(value->ty->tag == RTT_FLOAT)
        cout << "  fmv.s  " << retReg << ", fa0" << endl;
    else
        cout << "  mv  " << retReg << ", a0" << endl;
    }
}
//这里不需要分配寄存器，直接默认在a的几个寄存器中，读出来后直接分配栈空间
void Visit(const RawFuncArgs &data,const RawValueP &value) {
    int index = data.index;
    hardware.StackAlloc(value);
    if(index < 8) 
        hardware.AssignRegister(hardware.ValueToIndex[(RawValue *)value],10+index,value->ty->tag);//这里直接分配a寄存器
    else {
        // cout << "funcargs" << endl;
        int ValueReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *) value],value->ty->tag);
        const char *reg = hardware.GetRegister(ValueReg,value->ty->tag);
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
         auto &src = data.src;
     auto &index = data.index;
     const char *srcAddrReg;
    int SrcAddrReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)src],src->ty->tag);
    srcAddrReg = hardware.GetRegister(SrcAddrReg,src->ty->tag);
    int indexReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)index],index->ty->tag);
    const char * IndexReg = hardware.GetRegister(indexReg,index->ty->tag);
     //这个地方应该乘的是单个元素的长度，这里先解决的是一维数组的问题
     //cout << "calptrlen = " << calPtrLen(src) << ", elementlen" << (src->ty->data.array.len) << endl;
     int elementLen = calBaseLen(src)/(src->ty->pointer.base->array.len);
     int ValueReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
     if(value->dict.empty()) hardware.FreeRegister(ValueReg,value->ty->tag);
     else hardware.AlterNext(ValueReg,value->ty->tag,*value->dictIt);
     if(!IsNeed((RawValue *)src)) hardware.FreeRegister(SrcAddrReg,src->ty->tag);
     else {
        hardware.AlterNext(SrcAddrReg,src->ty->tag,*src->dictIt);
    }
    if(!IsNeed((RawValue *)index)) hardware.FreeRegister(indexReg,index->ty->tag);
    else {
        hardware.AlterNext(indexReg,index->ty->tag,*index->dictIt);
    }
     const char *ptrReg = hardware.GetRegister(ValueReg,value->ty->tag);
    if(elementLen == 4) {
        cout << "  slli " << ptrReg << ", " << IndexReg << ", " << 2 << endl;
    } else{
        cout << "  li  " << ptrReg << ", " << elementLen << endl;
        cout << "  mul " << ptrReg << ", " << IndexReg << ", " << ptrReg << endl;
    }
    cout << "  add  " << ptrReg << ", " << srcAddrReg << ", " << ptrReg << endl;
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
    int SrcAddrReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)src],src->ty->tag);
    srcAddrReg = hardware.GetRegister(SrcAddrReg,src->ty->tag);
    int indexReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)index],index->ty->tag);
    const char * IndexReg = hardware.GetRegister(indexReg,index->ty->tag);
     //这个地方应该乘的是单个元素的长度，这里先解决的是一维数组的问题
     //cout << "calptrlen = " << calPtrLen(src) << ", elementlen" << (src->ty->data.array.len) << endl;
     int elementLen = calBaseLen(src)/(src->ty->pointer.base->array.len);
     int ValueReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
     if(value->dict.empty()) hardware.FreeRegister(ValueReg,value->ty->tag);
     else hardware.AlterNext(ValueReg,value->ty->tag,*value->dictIt);
     if(!IsNeed((RawValue *)src)) hardware.FreeRegister(SrcAddrReg,src->ty->tag);
     else {
        hardware.AlterNext(SrcAddrReg,src->ty->tag,*src->dictIt);
    }
    if(!IsNeed((RawValue *)index)) hardware.FreeRegister(indexReg,index->ty->tag);
    else {
        hardware.AlterNext(indexReg,index->ty->tag,*index->dictIt);
    }
     const char *ptrReg = hardware.GetRegister(ValueReg,value->ty->tag);
     if(elementLen == 4) {
        cout << "  slli " << ptrReg << ", " << IndexReg << ", " << 2 << endl;
    } else {
        cout << "  li  " << ptrReg << ", " << elementLen << endl;
        cout << "  mul " << ptrReg << ", " << IndexReg << ", " << ptrReg << endl;
    }
    cout << "  add  " << ptrReg << ", " << srcAddrReg << ", " << ptrReg << endl;
    cout << endl;
}

void Visit(const RawTriple &data,const RawValueP &value) 
{
    const auto &hs1 = data.hs1;
    const auto &hs2 = data.hs2;
    const auto &hs3 = data.hs3;
    const auto &op  = data.op;
    int HS1Reg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)hs1],hs1->ty->tag);
    int HS2Reg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)hs2],hs2->ty->tag);
    int HS3Reg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)hs3],hs3->ty->tag);
    if(!IsNeed((RawValue *)hs1)) hardware.FreeRegister(HS1Reg,hs1->ty->tag);
    else {
        hardware.AlterNext(HS1Reg,hs1->ty->tag,*hs1->dictIt);
    }
    if(!IsNeed((RawValue *)hs2)) hardware.FreeRegister(HS2Reg,hs2->ty->tag);
    else {
        hardware.AlterNext(HS2Reg,hs2->ty->tag,*hs2->dictIt);
    }
    if(!IsNeed((RawValue *)hs3)) hardware.FreeRegister(HS3Reg,hs3->ty->tag);
    else {
        hardware.AlterNext(HS3Reg,hs3->ty->tag,*hs3->dictIt);
    }
    int ValueReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
    //这里需要根据类型判断他是在哪个寄存器里面
    const char *hs1Register;
    const char *hs2Register;
    const char *hs3Register;
    const char *ValueRegister;
    //hs1
    hs1Register = hardware.GetRegister(HS1Reg,hs1->ty->tag);
    //hs2
    hs2Register = hardware.GetRegister(HS2Reg,hs2->ty->tag);
    //hs3
    hs3Register = hardware.GetRegister(HS3Reg,hs3->ty->tag);
    //value
    ValueRegister = hardware.GetRegister(ValueReg,value->ty->tag);
    if(value->dict.empty()) hardware.FreeRegister(ValueReg,value->ty->tag);
    else hardware.AlterNext(ValueReg,value->ty->tag,*value->dictIt);
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
    auto src = (RawValue *)data.src;
    auto SrcType = data.src->ty->tag;
    if(SrcType == RTT_INT32){
        int SrcReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)src],src->ty->tag);
        const char *srcReg = hardware.GetRegister(SrcReg,src->ty->tag);
        if(!IsNeed((RawValue *)src)) hardware.FreeRegister(SrcReg,src->ty->tag);
        else {
            hardware.AlterNext(SrcReg,src->ty->tag,*src->dictIt);
        }
        int tReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
        const char *TReg = hardware.GetRegister(tReg,value->ty->tag);
        if(value->dict.empty()) hardware.FreeRegister(tReg,value->ty->tag);
        else hardware.AlterNext(tReg,value->ty->tag,*value->dictIt);
        cout<<"  fcvt.s.w " << TReg << ", " << srcReg << ", " << "rtz" << endl;
    } else if(SrcType == RTT_FLOAT) {
        int SrcReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)src],src->ty->tag);
        const char *srcReg = hardware.GetRegister(SrcReg,src->ty->tag);
        if(!IsNeed((RawValue *)src)) hardware.FreeRegister(SrcReg,src->ty->tag);
        else {
            hardware.AlterNext(SrcReg,src->ty->tag,*src->dictIt);
        }
        int tReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],value->ty->tag);
        const char *TReg = hardware.GetRegister(tReg,value->ty->tag);
        if(value->dict.empty()) hardware.FreeRegister(tReg,value->ty->tag);
        else hardware.AlterNext(tReg,value->ty->tag,*value->dictIt);
        cout<< "  fcvt.w.s " << TReg << ", "<< srcReg << ", " << "rtz" << endl;
    }
}
//对于dict如果我们发现dict是end,说明之后不会被使用了，如果不是的话就继续往前
//这个Value是重点，如果value已经被分配了寄存器，直接返回
//如果存在内存当中，调用loadreg后直接返回
//如果这个处于未分配时，这时应该是遍历的时候访问的，分配内存和寄存器
//这个Visit的方法就是要将RawValue值存到寄存器中，至于具体如何访问无需知道
//现在可能需要做一个约定：凡是遇到全局变量或者函数参数
void Visit(const RawValueP &value) {    
    const auto& kind = value->value;
    switch(kind.tag) {
    case RVT_RETURN: {
        const auto& ret = kind.ret.value; 
        if(ret != nullptr) {
        int RetReg = hardware.Ensure(hardware.ValueToIndex[(RawValue *)ret],ret->ty->tag);
        const char *RetRegister = RegisterManager::fregs[RetReg];
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
        // for(int i = 0; i < 12;i++) {
        //     hardware.LoadFRegister(RegisterManager::calleeFSave[i]);
        // }
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
            hardware.AssignRegister(hardware.ValueToIndex[(RawValue *)value],0,RTT_INT32);
        } else {//
            int AllocReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],RTT_INT32);
            const char *reg = hardware.GetRegister(AllocReg,RTT_INT32);
            cout << "  li   "  <<  reg  << ", "  << integer << endl;
            if(value->dict.empty()) hardware.FreeRegister(AllocReg,RTT_INT32);
            else hardware.AlterNext(AllocReg,RTT_INT32,*value->dictIt);
        }
        cout << endl;
        break;
    }
    case RVT_FLOAT:{
        const auto& floatNumber = kind.floatNumber.value;
        int AllocReg = hardware.AllocRegister(hardware.ValueToIndex[(RawValue *)value],RTT_FLOAT);
        const char *reg = hardware.GetRegister(AllocReg,RTT_FLOAT);
        int32_t str = convert(floatNumber);
        cout << "  li  " << "t0 " << ", " << str << endl;
        cout << "  fmv.w.x " <<  reg << ", " << "t0" << endl;
        if(value->dict.empty()) hardware.FreeRegister(AllocReg,RTT_FLOAT);
        else hardware.AlterNext(AllocReg,RTT_FLOAT,*value->dictIt);
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


void MarkDict(RawBasicBlock *bb) {
    auto &insts = bb->inst;
    for(auto it = insts.rbegin(); it != insts.rend(); ++it){
        auto inst = *it;
        auto BBindex = hardware.ValueToIndex[inst];
        auto tag = inst->value.tag;
        switch (tag)
        {
        case RVT_RETURN://src的use
        {
            auto src = (RawValue *)inst->value.ret.value;
            if(src) {
                src->dict.push_front(BBindex);
            }
            break;
        }
        case RVT_BINARY://lhs,rhs的use ，inst的def
        {
            auto lhs = (RawValue *)inst->value.binary.lhs;
            auto rhs = (RawValue *) inst->value.binary.rhs;
            lhs->dict.push_front(BBindex);
            rhs->dict.push_front(BBindex);
            break;
        }
        case RVT_STORE://src的use,dest的def
        {
            auto src = (RawValue *) inst->value.store.value;
            auto dest = (RawValue *) inst->value.store.dest;
            src->dict.push_front(BBindex);
            if(dest->value.tag == RVT_GET_ELEMENT || dest->value.tag == RVT_GET_PTR) {
                dest->dict.push_front(BBindex);
            }
            break;
        }
        case RVT_BRANCH://cond的use
        {
            auto cond = (RawValue *)inst->value.branch.cond;
            cond->dict.push_front(BBindex);
            break;
        }
        case RVT_CALL://call的def,param的use
        {
            auto &params = inst->value.call.args;
            for(auto param : params) {
                param->dict.push_front(BBindex);
            }
            break;
        }
        case RVT_GET_PTR://这个唯一有用的就是index的use,其他没用
        //貌似src也有点用了,都是
        {
            auto index = (RawValue *) inst->value.getptr.index;
            auto src = (RawValue *)inst->value.getptr.src;
            index->dict.push_front(BBindex);
            src->dict.push_front(BBindex);
            break;
        }
        case RVT_GET_ELEMENT:
        {
            auto index = (RawValue *) inst->value.getelement.index;
            auto src = (RawValue *)inst->value.getelement.src;
            index->dict.push_front(BBindex);
            src->dict.push_front(BBindex);
            break;
        }
        case RVT_CONVERT://目前没用上,先暂时不考虑
        {
            auto src = (RawValue *)inst->value.Convert.src;
            src->dict.push_front(BBindex);
            break;
        }
        default:
           break;
        }
    }
    for(auto inst : insts) {
        inst->dictIt = inst->dict.begin();
    }
}

// Visit RawBlock
void Visit(const RawBasicBlockP &bb){
     if(strcmp(bb->name,"entry")){
     cout << endl;
     cout << bb->name << ":" << endl;
     }
     auto &insts = bb->inst;
     hardware.registerManager.init();
     int index = 1;
     for(auto inst : insts) {
        hardware.IndexToValue[index] = inst;
        hardware.ValueToIndex[inst] = index;
        index++;
     }
     MarkDict((RawBasicBlock *)bb);
     for(auto inst : insts)
        Visit(inst);
}

void CalLiveOut(RawFunction *function) {
    for(auto bb:function->basicblock){//初始化为空
        bb->liveIn.clear();
        bb->liveOut.clear();
        bb->TLiveIn.clear();
        bb->TLiveOut.clear();
    }
    while(1){
        //遍历每个基本块
        for(auto bb:function->basicblock){
            // _in[n]=in[n]; _out[n]=out[n];
            bb->TLiveIn=bb->liveIn;
            bb->TLiveOut=bb->liveOut;
            //out[n]-def[n]
            unordered_set<RawValue*> difference_set,union_set;
            set_difference(bb->liveOut.begin(), bb->liveOut.end(), bb->defs.begin(), bb->defs.end(),std::inserter(difference_set, difference_set.end()));
            //in[n]=use[n]U(out[n]-def[n])
            set_union(difference_set.begin(), difference_set.end(), bb->uses.begin(), bb->uses.end(),std::inserter(union_set, union_set.end()));
            bb->liveIn = union_set;
            //out[n] = U_in[s] 所有后继的入口活跃集合的并集
            union_set.clear();
            for(auto fbb:bb->fbbs){
                set_union(union_set.begin(), union_set.end(), fbb->liveIn.begin(), fbb->liveIn.end(),std::inserter(union_set, union_set.end()));
                //union_set.merge(fbb->liveIn);
            }
            bb->liveOut = union_set;
        }
        int count = 0;
        for(auto bb:function->basicblock){
            if(bb->liveOut==bb->TLiveOut){
                if(bb->liveIn==bb->TLiveIn){
                    count++;
                }
            }
        }
        if(count==function->basicblock.size())
            return;
    }
}

// Visit RawFunction
void Visit(const RawFunctionP &func)
{
        auto &bbs = func->basicblock;
        auto &params = func->params;
        int bbsLen = bbs.size();
        //cerr << "bbsLen " << bbsLen << endl;
        if(bbsLen != 0) {
         CalLiveOut((RawFunction *)func);
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
        //  for(int i = 0; i < 12;i++) {
        //     hardware.SaveFRegister(RegisterManager::calleeFSave[i]);
        //  }
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
}
//就是我们发现一个问题：我们的这个integer节点就是单次使用的，因此可能没有必要说两个不同
//对于binary来说不需要，但是对于getelemptr需要，但是需要也没必要说要spill,因为他的生命周期就一个








