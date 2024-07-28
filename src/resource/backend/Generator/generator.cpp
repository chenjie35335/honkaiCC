#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include<bits/stdc++.h>
using namespace std;
static int LC_Number[100];
static int current_LC = 0;
vector<RawBasicBlockP> buf;
extern HardwareManager hardware;
//extern RegisterManager registerManager;
//visit const LC
map<RawValueP,int> malc;
int FF[1000];
void Visit(const RawValueP &value,int id) ;

void Visit(int number,int id)
{
    LC_Number[current_LC] = number;
    current_LC++;
}
vector<string> xinc;
vector<int> yinc;
void Print_LC()
{
    for(int i = 0; i < current_LC ;i++){
        cout<<".LC"<<i;
        cout<<": "<<endl;
        cout<<"   .word  "<<LC_Number[i]<<endl;
    }
}

int convert(float number)
{
    //联合体的神奇功能
    union {
        float f;
        int i;
    } u;
    u.f = number;
    return u.f;
};

HardwareManager hardware;
//处理load运算，由于我们在类型那里处理的调整，这里可能需要多加一个分类讨论
void Visit(const RawLoad &data, const RawValueP &value,int id) {
    const auto &src = data.src;
    if(src->value.tag == RVT_GLOBAL) {
        hardware.AllocRegister(value,id);
        string TargetReg = hardware.GetRegister(value,id);
        cout << "  la  " << TargetReg << ", " << src->name << endl;
        cout << "  lw  " << TargetReg << ", " << 0 << '(' << TargetReg << ')' << endl;
    } else if(hardware.IsMemory(src)){
        hardware.AllocRegister(value,id);
        string TargetReg = hardware.GetRegister(value,id);
        int srcAddress = hardware.getTargetOffset(src); //这里有点好，直接跳过了visit过程
        if(srcAddress > 2047) {
            cout << "  li   " << "t0, " << srcAddress << endl;
            cout << "  add  " << "t0, sp, t0" << endl;
            cout << "  lw  " <<  TargetReg << ", " << 0 << "(t0)" << endl;
        } else {
                cout << "  lw   " << TargetReg << ", " << srcAddress << "(sp)" << endl;
            }
        // if(hardware.registerManager.sadd[value]){
        //         cout << "  add " << TargetReg<< ", sp ," << TargetReg << endl;
        //     }
    }
    else if(src->value.tag == RVT_GET_ELEMENT || src->value.tag == RVT_GET_PTR){
//        hardware.addLockRegister(src);
        hardware.AllocRegister(value,id);
        string TargetReg = hardware.GetRegister(value,id);
        string ElementReg = hardware.GetRegister(src,id);
        cout << "  lw  " << TargetReg << ", " << 0 << '(' << ElementReg << ')' << endl;
        // hardware.LeaseLockRegister(src);
    } else{
        cout<<src->value.tag<<"!!!"<<endl;
        cout<<src->value.integer.value<<endl;
        exit(0);
    }
}
int F[1000];
//处理aggregate类型//首先是先计算地址然后store？
void Visit(const RawAggregate &aggregate,string src,string dest,int &index,int id) {
    cout<<"!!!!!!"<<endl;
    auto &elements = aggregate.elements;
    for(auto element : elements) {
        auto elementTag = element->value.tag;
        if(elementTag == RVT_AGGREGATE) {
            Visit(element->value.aggregate,src,dest,index,id);
        } else if(elementTag == RVT_INTEGER) {
            auto value = element->value.integer.value;
            //int offset = index *4;
            cout<<"Agg"<<endl;
            cout << "  li  " << src << ", " << value << endl;
            cout << "  sw  " << src << ", " << 0 << '(' << dest << ')' << endl;
            cout << "  addi " << dest <<  ", " << dest << ", " << 4 << endl;
            index++;
        } //add float
        else if(elementTag == RVT_FLOAT) {
            auto value = element->value.floatNumber.value;
            //int offset = index *4;
            //貌似要用到HI-LO寄存器
            int str = convert(value);
            Visit(str,id);
            cout<<"  lui "<< src <<", "<< "%hi(.LC"<< current_LC << ")" <<endl;
            cout<<"  flw  "<< "f" << src <<", %lo(.LC"<< current_LC << ")" <<endl;  //浮点寄存器
            cout<<"  fsw  "<< "f" << src << ", " << "-" << index*4 << "(" << dest << ")" <<endl;
            index++;
        }
        else {
            assert(0);
        }
    }
    //cout << endl;
}

//全局处理aggregate类型
void Visit(const RawAggregate &aggregate,int id) {
    auto &elements = aggregate.elements;
    for(auto element : elements) {
        auto elementTag = element->value.tag;
        if(elementTag == RVT_AGGREGATE) {
            Visit(element->value.aggregate,id);
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
void Visit(const RawStore &data, const RawValueP &value,int id) {//store这个地方的加锁问题一直很大
    //cout << endl;
    const auto &src = data.value;
    const auto &dest= data.dest;
    // cout<<(value->ty->tag)<<endl;
    if(dest->value.tag==RVT_ALLOC&&!malc[dest]){
        hardware.StackAlloc(dest);
    }
    
    if(src->value.tag==RVT_FUNC_ARGS&&src->value.funcArgs.index>=8){
        int idd=src->value.funcArgs.index;
            if(FF[idd]!=-1){
                int stackLen = hardware.getStackSize();
            int offsets=stackLen+(idd-8)*4;
            string regg=hardware.GetRegister(src,id);
            cout << "  lw  " << regg << ", " << offsets << "(sp)" << endl;
                hardware.StackAlloc(src,offsets);
            FF[idd]=-1;
            }
    }

    
    if(src->value.tag != RVT_AGGREGATE && dest->ty->tag != RTT_ARRAY) {
    if(dest->value.tag == RVT_GLOBAL) {
        Visit(src,id);
//        hardware.addLockRegister(src);
        hardware.AllocRegister(dest,id);
        // hardware.LeaseLockRegister(src);
        string SrcReg = hardware.GetRegister(src,id);
        string  DestReg = hardware.GetRegister(dest,id);
        string regg=hardware.GetRegister(src,id);
        cout << "  la  " << DestReg << ", " << dest->name << endl;
        cout << "  sw  " << SrcReg << ", " << 0 << '(' << DestReg << ')' << endl;
        //首先全局变量会被当成寄存器使用吗？
    } else if(hardware.IsMemory(dest)){
        Visit(src,id);

        string SrcReg = hardware.GetRegister(src,id);
        int srcAddress = hardware.getTargetOffset(dest);

        if(srcAddress > 2047) {
            cout << "  li   " << "t0, " << srcAddress << endl;
            cout << "  add  " << "t0, sp, t0" << endl;
            cout << "  sw  " <<  SrcReg << ", " << 0 << "(t0)" << endl;
        } 
        else
        cout << "  sw  " << SrcReg << ", " << srcAddress << "(sp)" << endl;
    } else if(dest->value.tag == RVT_GET_ELEMENT || dest->value.tag == RVT_GET_PTR) {
 //      hardware.addLockRegister(dest);
        Visit(src,id);
        hardware.LeaseLockRegister(dest);
        string SrcReg = hardware.GetRegister(src,id);
        string ElementReg = hardware.GetRegister(dest,id);
        cout << "  sw  " << SrcReg << ", " << 0 << '(' << ElementReg << ')' << endl;
    } else assert(0);
    } else if(src->value.tag == RVT_AGGREGATE && dest->ty->tag == RTT_ARRAY){//这里貌似还是没有什么好办法
//         hardware.AllocRegister(src,id);
//  //       hardware.addLockRegister(src);
//         hardware.AllocRegister(dest,id);
//         // hardware.LeaseLockRegister(src);
//         string SrcReg = hardware.GetRegister(src,id);
//         string DestReg = hardware.GetRegister(dest,id);//可以考虑把这个DestReg分配给中间变量
//         int srcAddress = hardware.getTargetOffset(dest);
//         if(srcAddress >= 2048) {
//              cout << "  li   " << "t0, " << srcAddress << endl;
//             cout << "  add  " << "t0, sp, t0" << endl;
//             cout << "  sw  " <<  SrcReg << ", " << 0 << "(t0)" << endl;
//         } else { 
//             cout << "  addi  " << DestReg << ", " << "sp" << ", " << srcAddress << endl;
//         }
//         int index = 0;
//         Visit(src->value.aggregate,SrcReg,DestReg,index,id);

        Visit(src,id);
        hardware.LeaseLockRegister(dest);
        string SrcReg = hardware.GetRegister(src,id);
        string ElementReg = hardware.GetRegister(dest,id);
        auto srcTag = src->ty->tag;
            cout << "  sw  " << SrcReg << ", " << 0 << '(' << ElementReg << ')' << endl;
    } else {
        cerr << "src tag:" << src->value.tag << ", dest tag: " << dest->value.tag << endl;
        cerr << "an aggregate value can't be assigned to a non-aggregate value" << endl;
        assert(0);
    }
}

//处理二进制运算
//添加浮点寄存器，貌似是最简单的
void Visit(const RawBinary &data,const RawValueP &value,int id) {
    const auto &lhs = data.lhs;
    const auto &rhs = data.rhs;
    const auto &op  = data.op;
    Visit(lhs,id);
//    hardware.addLockRegister(lhs);
    Visit(rhs,id);
 //   hardware.addLockRegister(rhs);
    hardware.AllocRegister(value,id);
    // hardware.LeaseLockRegister(lhs);hardware.LeaseLockRegister(rhs);
    string LhsRegister = hardware.GetRegister(lhs,id);
    string RhsRegister = hardware.GetRegister(rhs,id);
    string ValueRegister = hardware.GetRegister(value,id);
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
void Visit(const RawBranch &data, const RawValueP &value,int id){
    Visit(data.cond,id);
    string CondRegister = hardware.GetRegister(data.cond,id);
    string TrueBB = data.true_bb->name;
    string FalseBB = data.false_bb->name;
    cout << "  bnez  " << CondRegister << ", " << TrueBB << endl;
    cout << "  j  " << FalseBB << endl;
}

//处理jump运算
void Visit(const RawJump &data, const RawValueP &value,int id){
    string TargetBB = data.target->name;
    cout << "  j  " << TargetBB << endl;
}
//处理RawCall对象，a0和a1寄存器直接不使用
//这里有个严重问题
void Visit(const RawCall &data,const RawValueP &value,int id) {
    auto &params = data.args;
    for(int i = 0; i <params.size(); i++) {
        auto &ptr = (params[i]);
        Visit(params[i],id);
        
        if(i < 8) {
        
        string reg = hardware.GetRegister(params[i],id);
        hardware.StoreReg(10+i);
        auto z=hardware.registerManager.vp[id][params[i]];
        int zz=26-hardware.registerManager.vis[id][z]+5;
        int L=hardware.registerManager.LX.size();
        if(L){
        for(int pos=0;pos<L;pos++){
            auto [r,l]=hardware.registerManager.LX[pos];
            // cout<<r<<" "<<zz<<"!!!"<<endl;
        if(r==zz){
            hardware.StoreReg(zz);

            if(l > 2047) {
                        cout << "  li   " << "t0, " << l << endl;
                        cout << "  add  " << "t0, sp, t0" << endl;
                        cout << "  lw  " <<  hardware.registerManager.regs[zz] << ", " << 0 << "(t0)" << endl; 
                    } else
            cout << "  lw   " << hardware.registerManager.regs[zz] << ", " << l << "(sp)" << endl;
        hardware.registerManager.registerLook.insert(pair<RawValueP, int>(params[i],zz));
            hardware.registerManager.LX[pos]={-1,-1};
            }
        }
    }
            const char* paramReg = RegisterManager::regs[10+i];
            cout << "  mv  " << paramReg << ", " << reg <<endl;
            hardware.registerManager.registerLook.erase(value);
            hardware.registerManager.registerLook.insert(pair<RawValueP, int>(params[i], 10+i));
            
            for(int pos=0;pos<L;pos++){
            auto [r,l]=hardware.registerManager.LX[pos];
            auto val = hardware.registerManager.LY[pos];
            // cout<<r<<" "<<zz<<"!!!"<<endl;
        if(r==zz){
            if(l > 2047) {
                        cout << "  li   " << "t0, " << l << endl;
                        cout << "  add  " << "t0, sp, t0" << endl;
                        cout << "  lw  " <<  hardware.registerManager.regs[zz] << ", " << 0 << "(t0)" << endl; 
                    } else
                     cout << "  lw   " << hardware.registerManager.regs[zz] << ", " << l << "(sp)" << endl;
        hardware.registerManager.registerLook.insert(pair<RawValueP, int>(params[i],zz));
            hardware.registerManager.LX[pos]={-1,-1};
            }
        }

        } else {
            string reg = hardware.GetRegister(params[i],id);
            int offset = (i-8)*4;
            FF[i]=offset;

            if(offset > 2047) {
                cout << "  li  t0, " << offset << endl;
                cout << "  add  t0, t0, sp" << endl;
                cout << "  sw  " << reg << ", " << 0 << "(t0)" << endl;
            } else{
                cout << "  sw  " << reg << ", " << offset << "(sp)" << endl;
            }
            hardware.registerManager.registerLook.erase(params[i]);
            hardware.StackAlloc(params[i]);
         }
    }


    queue<int> Q;
     for(int i = 0;i < 7;i++) {
        // cout<<"!!!"<<endl;
         hardware.StoreReg(RegisterManager::callerSave[i]);
     }
     
    cout<<"  call "<<data.callee->name<<endl;

    

    if((hardware.registerManager.LX).size()){

        for(int i = 0;i < 7;i++) {
        for(int pos=0;pos<hardware.registerManager.LX.size();pos++){
            auto [r,l]=hardware.registerManager.LX[pos];
            auto val=hardware.registerManager.LY[pos];
        if(r!=RegisterManager::callerSave[i]) continue;
        hardware.registerManager.LX[pos]={-1,-1};

        if(l > 2047) {
                        cout << "  li   " << "t0, " << l << endl;
                        cout << "  add  " << "t0, sp, t0" << endl;
                        cout << "  lw  " <<  hardware.registerManager.regs[RegisterManager::callerSave[i]] << ", " << 0 << "(t0)" << endl; 
                    } else
                    cout << "  lw   " << hardware.registerManager.regs[RegisterManager::callerSave[i]] << ", " << l << "(sp)" << endl;
        hardware.registerManager.registerLook.insert(pair<RawValueP, int>(val,RegisterManager::callerSave[i]));
        }
     }
    }
    
     
    if(value->ty->tag == RTT_INT32){
        hardware.AssignRegister(value,10);
    }
    string st=hardware.GetRegister(value,id);
    if(st!="a0"){
        cout << "  mv  " << st << ", " << "a0" << endl;
            hardware.registerManager.registerLook.erase(value);
            int Y=hardware.registerManager.vp[id][value];
            int X=26-hardware.registerManager.vis[id][Y]+5;
            hardware.registerManager.registerLook.insert(pair<RawValueP, int>(value,X));
    }
    if((hardware.registerManager.LX).size())
   for(int i=10;i<min((int)(10+params.size()),18);i++){
    for(int pos=0;pos<hardware.registerManager.LX.size();pos++){
        auto [r,l]=hardware.registerManager.LX[pos];
        auto val=hardware.registerManager.LY[pos];
        if(r!=i) continue;
        
        if(l > 2047) {
                        cout << "  li   " << "t0, " << l << endl;
                        cout << "  add  " << "t0, sp, t0" << endl;
                        cout << "  lw  " << hardware.registerManager.regs[i] << ", " << 0 << "(t0)" << endl; 
                    } 
        else
        cout << "  lw   " << hardware.registerManager.regs[i] << ", " << l << "(sp)" << endl;
        hardware.registerManager.registerLook.insert(pair<RawValueP, int>(val,i));
    }
           
        }
        hardware.registerManager.LX.clear();
        hardware.registerManager.LY.clear();
}
//这里不需要分配寄存器，直接默认在a的几个寄存器中，读出来后直接分配栈空间
void Visit(const RawFuncArgs &data,const RawValueP &value,int id) {
    int index = data.index;
    int stackLen = hardware.getStackSize();
    int addr = hardware.StackAlloc(value->addr);
    // if(index < 8) {
    //     int regAddr = 10+index;//10是a0号寄存器
    //     string reg = RegisterManager::regs[regAddr];
    //     cout << "  sw  " << reg << ", " << addr << "(sp)" << endl;//如果访问这个RawFuncArgs的话，单纯是分配内存空间
    //     hardware.registerManager.registerLook.insert(pair<RawValueP, int>(value, regAddr));
    //     //但是存在问题，这里会把这个内存空间
    // } else {
    //     int srcAddr = stackLen+(index-8)*4;
    //     hardware.GetRegister(value,id);
    //     cout << "  lw  " << "t0"<< "," << srcAddr << "(sp)" << endl;
    //     cout << "  sw  " << "t0"<< "," << addr << "(sp)" << endl;
    // }
}

//这里需要写一个浮点IEEE754转换函数


//处理Global Alloc变量
void Visit(const RawGlobal &data,const RawValueP &value,int id) {
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
            int str = convert(Init);
            cout << "  .word " << str << endl; 
        }
    }
    
     else if(tag == RVT_ZEROINIT) {
       int len =  calBaseLen(value);
       cout << "  .zero " << len << endl;
    } else if(tag == RVT_AGGREGATE) {
        Visit(data.Init->value.aggregate,id);
    } else assert(0);
}

void Visit(const RawGetPtr &data, const RawValueP &value,int id) {
    //cout << "parse getptr" << endl;
    auto src = data.src;
    auto index = data.index;
    string srcAddrReg;
    Visit(src,id);
    srcAddrReg = hardware.GetRegister(src,id);
//    hardware.addLockRegister(src);
    Visit(index,id);
 //   hardware.addLockRegister(index);
    string IndexReg = hardware.GetRegister(index,id);
    int elementLen = calBaseLen(src);
    hardware.AllocRegister(value,id);
    // hardware.LeaseLockRegister(src);
    // hardware.LeaseLockRegister(index);
    string ptrReg = hardware.GetRegister(value,id);
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
void Visit(const RawGetElement &data,const RawValueP &value,int id) {
     //cout << "Visit GetElement" << endl;
     auto &src = data.src;
     auto &index = data.index;
     string srcAddrReg;
     if(src->value.tag == RVT_GLOBAL) {
        hardware.AllocRegister(src,id);
        srcAddrReg = hardware.GetRegister(src,id);
        cout << "  la  " << srcAddrReg << ", " << src->name << endl;
     } else if(hardware.IsMemory(src)){//这里包含了参数值的问题
        hardware.AllocRegister(src,id);
        srcAddrReg = hardware.GetRegister(src,id);
        int srcAddr = hardware.getTargetOffset(src);
        // cout<<"ELe"<<endl;
        cout << "  li  " << srcAddrReg << ", " << srcAddr << endl; 
        cout << "  add " << srcAddrReg << ", sp, " << srcAddrReg << endl;  
     } else {
        srcAddrReg = hardware.GetRegister(src,id);
     }
     Visit(index,id);
     string IndexReg = hardware.GetRegister(index,id);
     //这个地方应该乘的是单个元素的长度，这里先解决的是一维数组的问题
     //cout << "calptrlen = " << calPtrLen(src) << ", elementlen" << (src->ty->data.array.len) << endl;
     //int elementLen = calPtrLen(src);??
     int elementLen = calBaseLen(src)/(src->ty->pointer.base->array.len);

     hardware.AllocRegister(value,id);
     string ptrReg = hardware.GetRegister(value,id);
     if(elementLen == 4) {
        cout << "  slli " << IndexReg << ", " << IndexReg << ", " << 2 << endl;
    } else {
        cout << "  li  " << "t0" << ", " << elementLen << endl;
        cout << "  mul " << IndexReg << ", " << IndexReg << ", " << "t0" << endl;
    }
     cout << "  add  " << ptrReg << ", " << srcAddrReg << ", " << IndexReg << endl;

}

//这个Value是重点，如果value已经被分配了寄存器，直接返回
//如果存在内存当中，调用loadreg后直接返回
//如果这个处于未分配时，这时应该是遍历的时候访问的，分配内存和寄存器
//这个Visit的方法就是要将RawValue值存到寄存器中，至于具体如何访问无需知道
//现在可能需要做一个约定：凡是遇到全局变量或者函数参数
map<RawValueP,int> hx;
void Visit(const RawValueP &value,int id) {    
    const auto& kind = value->value;
    if(hx[value]) return;
    hx[value]=1;
    // if(hardware.IsRegister(value)) {
    //      if(kind.tag == RVT_FUNC_ARGS) {
    //          hardware.AllocRegister(value,id);
    //          hardware.LoadFromMemory(value,id);
    //      }
    //     return;
    // }  
    //else 
    // if(hardware.IsMemory(value)) {
    //      hardware.LoadFromMemory(value,id);
    //     return;
    // }
    // else {
    switch(kind.tag) {
    case RVT_RETURN: {
        const auto& ret = kind.ret.value; 
        if(ret != nullptr) {
        Visit(ret,id);
        string RetRegister = hardware.GetRegister(ret,id);
        if(RetRegister!="a0") {
        cout << "  mv   a0, "<< RetRegister << endl;
            hardware.registerManager.registerLook.erase(value);
            hardware.registerManager.registerLook.insert(pair<RawValueP, int>(value, 10));
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
        // if(integer == 0) {
        //     hardware.registerManager.vis[id][hardware.registerManager.vp[id][value]]=31;
        //     hardware.AssignRegister(value,0);
        // } else {
            hardware.AllocRegister(value,id);
            string reg = hardware.GetRegister(value,id);
            cout << "  li   "  <<  reg  << ", "  << integer << endl;
            int XX=hardware.registerManager.vp[id][value];
            hardware.registerManager.registerLook.erase(value);
            hardware.registerManager.registerLook.insert(pair<RawValueP, int>(value, 26-hardware.registerManager.vis[id][XX]+5));
    //    }
        //cout << endl;
        break;
    }
    case RVT_FLOAT:{
        const auto& floatNumber = kind.floatNumber.value;
        if(floatNumber >= 0 && floatNumber <= 0) { //判断浮点数为0，看起来很蠢
            hardware.AssignRegister(value,0);
        } else {
            hardware.AllocFRegister(value);
            string reg = hardware.GetRegister(value,id);
            //浮点数的load操作很复杂，这里先不处理
            cout << "  li   "  <<  reg  << ", "  << floatNumber << endl;
        }
        break;
    }
    case RVT_BINARY: {
        const auto &binary = kind.binary;
        Visit(binary,value,id);
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
        Visit(load,value,id);
        break;
    }
    case RVT_STORE: {
        const auto &store = kind.store;
        Visit(store,value,id);
        break;
    }
    case RVT_BRANCH: {
        const auto &branch = kind.branch;
        Visit(branch,value,id);
        break;
    }
    case RVT_JUMP: {
        const auto &jump = kind.jump;
        Visit(jump,value,id);
        break;
    }
    case RVT_CALL: {
        const auto &call = kind.call;
        Visit(call,value,id);
        break;
    }
    case RVT_FUNC_ARGS:{
        const auto &ARGS=kind.funcArgs;
        Visit(ARGS,value,id);
        break;
    }
    case RVT_GLOBAL: {
        const auto &global = kind.global;
        cout << "  .globl " << value->name <<  endl;
        cout << value->name << ":" << endl;
        Visit(global,value,id);
        cout << endl;
        break;
    }
    case RVT_AGGREGATE: {//这个貌似没有单独出现，都是依附于alloc之类的存在的
        //cout << "aggregate handler" << endl;
        break;
    }
    case RVT_GET_PTR: {
        const auto &getptr = kind.getptr;
        Visit(getptr,value,id);
        break;
    }
    case RVT_GET_ELEMENT: {//对于这种element类型的变量，直接分配空间就行
    //貌似对于多维数组来说，基地址已经存进寄存器中了，其他的只需要调用就行
        const auto &getElement = kind.getelement;
        Visit(getElement,value,id);
        break;
    }
    default:{
        cerr << "unknown kind: " << kind.tag << endl;
        assert(false); 
    }
    }
//}
}
// Visit RawBlock
void Visit(const RawBasicBlockP &bb,int id){
     if(strcmp(bb->name,"entry")){
     cout << endl;
     cout << bb->name << ":" << endl;
     }
     auto &insts = bb->inst;
     for(auto inst : insts)
     Visit(inst,id);
} 
int NOW=0;
// Visit RawFunction
void Visit(const RawFunctionP &func,int id)
{
        auto &bbs = func->basicblock;
        auto &params = func->params;
        
        int bbsLen = bbs.size();
        //cerr << "bbsLen " << bbsLen << endl;
        if(bbsLen != 0) {
         NOW=hardware.init(func);
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
        //  for(int i=0;i<xinc.size();i++){
        //     cout<<"  lw "<<xinc[i]<<", "<<yinc[i]<<endl;
        //  }
        //  xinc.clear();yinc.clear();
        
        

        
        // for(auto param : params)
        //  Visit(param,id);



        for(auto bb : bbs)
         Visit(bb,id);
         cout << endl;
        }
        
}

void Visits(const RawBinary &data,const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP bb);
void Visits(const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP bb);
extern SignTable signTable;
void addload(int x,list<RawValue*>::const_iterator it,RawBasicBlock* bb){

            // RawValue *value = new RawValue();
            // value->name = nullptr;
            // value->value.tag = RVT_INTEGER;
            // value->value.integer.value = x;
            // RawType *ty = new RawType();
            // ty->tag = RTT_INT32;
            // value->ty = ty;

            // RawValueP nb;
            // auto p=--it;
            // auto &insts = bb->inst;
            // RawValue * load = new RawValue();
            // RawType *tyy = new RawType();
            // tyy->tag = RTT_INT32;
            // load->ty = (RawTypeP) tyy;
            // load->name = nullptr;
            // load->value.tag = RVT_LOAD;
            // load->value.load.src = value;
            // MarkUse((RawValue *)x,load);
            // bb->inst.insert(p,load);
            // RawValue *SrcValue = (RawValue*) x;
            // bb->uses.insert(SrcValue);
            // string nm=value->name;
            // signTable.insertMidVar(nm,load);
            // MarkDef(load,load);

            // //打标记记这条value后需要add sp即可，不会对生命周期造成影响。maybe?
            // hardware.registerManager.sadd[load]=1;
}

void Visits(const RawLoad &data, const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP bb) {
    const auto &src = data.src;
    if(src->value.tag == RVT_GLOBAL) {
    } else if(hardware.IsMemory(src)){
        int srcAddress = hardware.getTargetOffset(src);
    //     if(srcAddress>=2048){
    //         addload(srcAddress,it,(RawBasicBlock*)bb);
    //         //            cout << "  li  " << DestReg << ", " << srcAddress << endl;  加load
    //     }
    // } else if(src->value.tag == RVT_GET_ELEMENT || src->value.tag == RVT_GET_PTR){
    //     //先不处理
    //     // hardware.addLockRegister(src);
    //     // hardware.AllocRegister(value);
    //     // string TargetReg = hardware.GetRegister(value);
    //     // string ElementReg = hardware.GetRegister(src);
    //     // cout << "  lw  " << TargetReg << ", " << 0 << '(' << ElementReg << ')' << endl;
    //     // hardware.LeaseLockRegister(src);
     } else return;
}

void Visits(const RawStore &data, const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP bb) {//store
    //cout << endl;
    const auto &src = data.value;
    const auto &dest= data.dest;
    if(src->value.tag != RVT_AGGREGATE && dest->ty->tag != RTT_ARRAY) {
    if(dest->value.tag == RVT_GLOBAL) {
        Visits(src,it,bb);
        //首先全局变量会被当成寄存器使用吗？
    } else if(hardware.IsMemory(dest)){

        Visits(src,it,bb);
    } else if(dest->value.tag == RVT_GET_ELEMENT || dest->value.tag == RVT_GET_PTR) {
        Visits(src,it,bb);
    } else return;
    } else if(src->value.tag == RVT_AGGREGATE && dest->ty->tag == RTT_ARRAY){//这里貌似还是没有什么好办法
        // hardware.LeaseLockRegister(src);
        // string SrcReg = hardware.GetRegister(src);
        // string DestReg = hardware.GetRegister(dest);//可以考虑把这个DestReg分配给中间变量
        int srcAddress = hardware.getTargetOffset(dest);
//         if(srcAddress >= 2048) {
//              addload(srcAddress,it,(RawBasicBlock*)bb);
// //            cout << "  li  " << DestReg << ", " << srcAddress << endl;  加load
//         }
//         int index = 0;
// //        Visit(src->value.aggregate,SrcReg,DestReg,index); 这是？
     }
// else {
//         assert(0);
//     }
}

void Visits(const RawCall &data,const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP bb) {
    auto &params = data.args;
    for(int i=0;i<min(8,(int)params.size());i++){
        // auto &insts = bb->inst;
        //     RawValue *alloc = new RawValue();
        //     RawType *ty = new RawType();
        //     ty->tag = RTT_POINTER;
        //     RawType *pointerTy = new RawType();
        //     pointerTy->tag = RTT_INT32;
        //     ty->pointer.base = pointerTy;
        //     alloc->ty = (RawTypeP)ty;
        //     alloc->value.tag = RVT_ALLOC;

        //     auto &instss = bb->inst;
        //     RawValue *store = new RawValue();
        //     RawType *tyy = new RawType();
        //     tyy->tag = RTT_UNIT;
        //     store->ty = (RawTypeP)tyy;
        //     store->name = nullptr;
        //     store->value.tag = RVT_STORE;
        //     store->value.store.value = value;
        //     store->value.store.dest = alloc;
        //     RawValue *DestValue = (RawValue*)alloc;
        //     RawBasicBlock* bbb=(RawBasicBlock*)bb;
        //     bbb->inst.insert(it,store);
        //     store->addr=alloc;
    }
    auto itt=it;

}

void Visits(const RawGetPtr &data, const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP bb) {
    //cout << "parse getptr" << endl;
    auto src = data.src;
    auto index = data.index;
    string srcAddrReg;
    Visits(src,it,bb);
    Visits(index,it,bb);
    // RawBasicBlock* bbb=(RawBasicBlock*)bb;
    //  RawValue* srcc=(RawValue*)src;
    //  RawValue* indexx=(RawValue*)index;
    //  bbb->inst.insert(it,srcc);
    //  bbb->inst.insert(it,indexx);
}

//先不处理
void Visits(const RawGetElement &data,const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP bb) {
     auto &src = data.src;
     auto &index = data.index;
     Visits(src,it,bb);
     Visits(index,it,bb);
    //  RawBasicBlock* bbb=(RawBasicBlock*)bb;
    //  RawValue* srcc=(RawValue*)src;
    //  RawValue* indexx=(RawValue*)index;
    //  bbb->inst.insert(it,srcc);
    //  bbb->inst.insert(it,indexx);
}
map<RawValueP,int> vv;
void Visits(const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP b) {    
    const auto& kind = value->value;
        RawBasicBlock* bb=(RawBasicBlock*)b;
        if(kind.tag == RVT_FUNC_ARGS) {

            return;
        }
        

    switch(kind.tag) {
    case RVT_RETURN: {
        const auto& ret = kind.ret.value; 
        if(ret != nullptr) {
        Visits(ret,it,bb);
        }
        int StackSize = hardware.getStackSize();
//         if(StackSize > 2047) {   //1   问题在t0会不会冲突？
// //        addload(StackSize,it,(RawBasicBlock*)bb);
//         // cout << "  li t0, " << StackSize << endl;
//         // cout << "  add sp, sp, t0" << endl;
//         }
        break;
    }
    case RVT_INTEGER: {
        const auto& integer = kind.integer.value;
        if(integer == 0) {
            // hardware.AssignRegister(value,0);
        } else {
            // hardware.AllocRegister(value);
            // string reg = hardware.GetRegister(value);
            // cout << "  li   "  <<  reg  << ", "  << integer << endl;
        }
        //cout << endl;
        break;
    }
    case RVT_FLOAT:{
        const auto& floatNumber = kind.floatNumber.value;
        if(floatNumber >= 0 && floatNumber <= 0) { //判断浮点数为0，看起来很蠢
            // hardware.AssignRegister(value,0);
        } else {
            // hardware.AllocFRegister(value);
            // string reg = hardware.GetRegister(value);
            // //浮点数的load操作很复杂，这里先不处理
            // cout << "  li   "  <<  reg  << ", "  << floatNumber << endl;
        }
        break;
    }
    case RVT_BINARY: {
        const auto &binary = kind.binary;
        Visits(binary,value,it,bb);
        //cout << endl;
        break;
    }
    case RVT_ALLOC: {
        //cout << "alloc" << endl;
        // hardware.StackAlloc(value); 
        break;
    }
    case RVT_LOAD: {
        const auto &load = kind.load;
        Visits(load,value,it,bb);
        break;
    }
    case RVT_STORE: {
        const auto &store = kind.store;
        Visits(store,value,it,bb);
        break;
    }
    case RVT_BRANCH: {
        // const auto &branch = kind.branch;
        // Visits(branch,value,it);
        break;
    }
    case RVT_JUMP: {
        // const auto &jump = kind.jump;
        // Visits(jump,value,it);
        break;
    }
    case RVT_CALL: {//1
        const auto &call = kind.call;
        Visits(call,value,it,bb);
        break;
    }
    case RVT_FUNC_ARGS:{
        // const auto &args = kind.funcArgs;
        // Visits(args,it,(RawBasicBlock*)bb);
        break;
    }
    case RVT_GLOBAL: {
        // const auto &global = kind.global;
        // // cout << "  .globl " << value->name <<  endl;
        // // cout << value->name << ":" << endl;
        // Visits(global,value,it);
        // // cout << endl;
        break;
    }
    case RVT_AGGREGATE: {//这个貌似没有单独出现，都是依附于alloc之类的存在的
        //cout << "aggregate handler" << endl;
        break;
    }
    case RVT_GET_PTR: {
        const auto &getptr = kind.getptr;
        Visits(getptr,value,it,bb);
        break;
    }
    case RVT_GET_ELEMENT: {//对于这种element类型的变量，直接分配空间就行
    //貌似对于多维数组来说，基地址已经存进寄存器中了，其他的只需要调用就行
        const auto &getElement = kind.getelement;
        Visits(getElement,value,it,bb);
        break;
    }
    default:{
        // cerr << "unknown kind: " << kind.tag << endl;
        assert(false); 
    }
    }
}

//处理二进制运算
//添加浮点寄存器，貌似是最简单的
void Visits(const RawBinary &data,const RawValueP &value,list<RawValue*>::const_iterator it,RawBasicBlockP bb) {
    const auto &lhs = data.lhs;
    const auto &rhs = data.rhs;
    const auto &op  = data.op;
    Visits(lhs,it,bb);
//    hardware.addLockRegister(lhs);
    Visits(rhs,it,bb);
 //   hardware.addLockRegister(rhs);
}

void Visits(const RawBasicBlockP &bb){
    buf.push_back(bb);
     auto &insts = bb->inst;
     for(auto it=insts.begin();it!=insts.end();it++){
        Visits(*it,it,bb);
     }
     
} 
int T=10000;
void Visits(const RawFunctionP &func,int id)
{
        
        auto &bbs = func->basicblock;
        auto &params = func->params;
        int bbsLen = bbs.size();
        //cerr << "bbsLen " << bbsLen << endl;

        // int StackSize = hardware.getStackSize();
        //  if( StackSize <= 2048) {
        //     cout << "  addi sp, sp, " << -StackSize << endl;
        //  } else {
        //     cout << "  li t0, " << -StackSize << endl;
        //     cout << "  add sp, sp, t0" << endl;
        //  }
        
        if(bbsLen != 0) {
    //      for(int i = 0; i < params.size(); i++) {
    //     if(i < 8) {
    //         hardware.registerManager.vp[params[i]]=++T;
    //         hardware.registerManager.rvp[T]=params[i];
    //         hardware.registerManager.vis[id][hardware.registerManager.vp[params[i]]]=5+i;
    //     }
    // }
        auto e1=*(bbs.begin());
        auto j=e1->inst.begin();
        int ee=params.size();
        for(auto bb : bbs)
         Visits(bb);
        }
        
}
void generateASM(RawProgramme *& programme) {
    cout << "  .data" << endl;
    auto &values = programme->values;
    auto &funcs = programme->funcs;
    map<RawFunction*,int> mp;
    for(int i=0;i<1000;i++) FF[i]=-1;
    int cnt=0;
    if(funcs.end()!=funcs.begin())
    for(auto it=--funcs.end();;it--){
        auto func=*it;
        hardware.registerManager.registerLook.clear();
        buf.clear();
        mp[func]=++cnt;
        Visits(func,cnt);
        vector<RawValue*> &cuf=(func->params);
        while(1){
           // cout<<func->name<<endl;
            int x=hardware.struct_graph(buf,cnt,cuf);
            if(x) break;
        }
    if(it==funcs.begin()) break;
    }
    
    for(auto value : values)
    Visit(value,0);
    cout << "  .text" << endl;
        if(funcs.end()!=funcs.begin())
    for(auto it=--funcs.end();;it--){
        auto func=*it;
    Visit(func,mp[func]);
    if(it==funcs.begin()) break;
    }
    Print_LC();
}









