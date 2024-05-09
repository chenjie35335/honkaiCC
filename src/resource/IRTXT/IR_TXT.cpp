#include <iostream>
#include <cassert>
#include "../../include/midend/IR/IRGraph.h"
#include "../../include/IRTXT/IR_TXT.h"
#include "../../include/midend/IR/ValueKind.h"
bool judgement(string fun_name)
{
    if("putch"==fun_name){
        cout<<"decl @putch(i32)"<<endl;
        return true;
    }
    else if("putint"==fun_name){
        cout<<"decl @putint(i32)"<<endl;
        return true;
    }
    else if("starttime"==fun_name){
        cout<<"decl @starttime()"<<endl;
        return true;
    }
    else if("stoptime"==fun_name){
        cout<<"decl @stoptime()"<<endl;
        return true;
    }
    else if("getch"==fun_name){
        cout<<"decl @getch(): i32"<<endl;
        return true;
    }
    else if("getint"==fun_name){
        cout<<"decl @getint(): i32"<<endl;
        return true;
    }
    else if("putarray"==fun_name){
        cout<<"decl @getarray(*i32): i32"<<endl;
        return true;
    }
    else if("getarray"==fun_name){
        cout<<"decl @putarray(i32, *i32)"<<endl;
        return true;
    }
    else
        return false;
}
void init_symbol()
{
    symbol_num = 0;
    symbol_id = 0;
    Symbol_List.clear();
    for (const auto& pair : Global_List) {
        // 将全局符号插入
        Symbol_List.insert(pair);
    }
}
void alloc_symbol(const RawValueP &value)
{
    Symbol_List[value] = '%'+to_string(symbol_num++);
}
void Visit_ZEROINIT(const RawValueP &value)
{
    Symbol_List[value] = "zeroinit";
}
void Visit_Return(const RawValueP &value)
{
    const auto& ret = value->value.data.ret;
    string ret_res = "";
    // return 语句存在返回值
    if(ret.value != nullptr) {
        Visit_Value(ret.value);
        ret_res = Symbol_List[ret.value];
    }
    else{// return 语句不存在返回值
    }   
    cout<<"  ret "<<ret_res;
    cout<<endl;
}
void Visit_Integer(const RawValueP &value)
{
    Symbol_List[value]=to_string(value->value.data.integer.value);
}
void Visit_Binary(const RawValueP &value)
{
    const RawBinary &data = value->value.data.binary;
    const auto &lhs = data.lhs;
    const auto &rhs = data.rhs;
    const auto &op  = data.op;
    Visit_Value(lhs);
    string ls = Symbol_List[lhs];
    Visit_Value(rhs);
    string rs = Symbol_List[rhs];
    alloc_symbol(value);
    string res = Symbol_List[value];
    cout<<"  ";
    switch(op) {
        case RBO_ADD:
            cout<<res<<" = add "<<ls<<", "<<rs<<endl;
            break;
        case RBO_SUB:
            cout<<res<<" = sub "<<ls<<", "<<rs<<endl;
            break;
        case RBO_EQ:
            cout<<res<<" = eq "<<ls<<", "<<rs<<endl;
            break;
        case RBO_NOT_EQ:
            cout<<res<<" = ne "<<ls<<", "<<rs<<endl;
            break;
        case RBO_MUL:
            cout<<res<<" = mul "<<ls<<", "<<rs<<endl;
            break;
        case RBO_DIV:
            cout<<res<<" = div "<<ls<<", "<<rs<<endl;
            break;
        case RBO_MOD:
            cout<<res<<" = mod "<<ls<<", "<<rs<<endl;
            break;
        case RBO_LT:
            cout<<res<<" = lt "<<ls<<", "<<rs<<endl;
            break;
        case RBO_GT:
            cout<<res<<" = gt "<<ls<<", "<<rs<<endl;
            break;
        case RBO_GE:
            cout<<res<<" = ge "<<ls<<", "<<rs<<endl;
            break;
        case RBO_LE:
            cout<<res<<" = le "<<ls<<", "<<rs<<endl;
            break;   
        case RBO_OR:
            cout<<res<<" = or "<<ls<<", "<<rs<<endl;
            break;
        case RBO_XOR:
            cout<<res<<" = xor "<<ls<<", "<<rs<<endl;
            break;
        case RBO_AND:
            cout<<res<<" = and "<<ls<<", "<<rs<<endl;
            break;
        default: assert(0);
    }
}
void Visit_Alloc(const RawValueP &value)
{
    Symbol_List[value] = "@a"+to_string(symbol_id++);
    cout<<"  "<<Symbol_List[value]<< " = alloc i32"<<endl;
}
void Visit_Load(const RawValueP &value)
{
    alloc_symbol(value);
    string res = Symbol_List[value];
    Visit_Value(value->value.data.load.src);
    string src = Symbol_List[value->value.data.load.src];
    cout<<"  "<<res<<" = load "<<src<<endl;
}
void Visit_Store(const RawValueP &value)
{
    Visit_Value(value->value.data.store.value);
    string sv = Symbol_List[value->value.data.store.value];
    Visit_Value(value->value.data.store.dest);
    string sd = Symbol_List[value->value.data.store.dest];
    cout<<"  store "<<sv<<", "<<sd<<endl;
}
void Visit_Branch(const RawValueP &value)
{
    const RawBranch branch = value->value.data.branch;
    Visit_Value(branch.cond);
    cout<<"  br "<<Symbol_List[branch.cond]<<", %"<<branch.true_bb->name<<", %"<<branch.false_bb->name<<endl;
}
void Visit_Jump(const RawValueP &value)
{
    cout<<"  jump %"<<value->value.data.jump.target->name<<endl;
}
void Visit_Call(const RawValueP &value)
{
    const RawCall call = value->value.data.call;
    if (value->ty->tag != RTT_UNIT)//函数有返回值
    {
        alloc_symbol(value);
        cout<<"  "<<Symbol_List[value]<<" = call @";
        cout<<call.callee->name<<'(';
    }
    else{//函数无返回值
        cout<<"  call @"<<call.callee->name<<'(';
    }
    //函数参数
    for(int i=0;i<call.args.len;i++)
    {
        if(i!=0)cout<<',';
        cout<<Symbol_List[RawValueP(call.args.buffer[i])];
    }
    cout<<')'<<endl;
    
}
void Visit_Func_Args(const RawValueP &value)
{
    int index = value->value.data.funcArgs.index;
    // 为参数重新分配空间
    Symbol_List[value] = "@p_"+to_string(index);
    cout<<"  "<<Symbol_List[value]<< " = alloc i32"<<endl;
    cout<<"  store "<<"\%p_"<<index<<", "<<Symbol_List[value]<<endl;
}
void Visit_Global(const RawValueP &value)
{
    Global_List[value] = '@'+string(value->name);
    RawGlobal global = value->value.data.global;
    cout<<"global "<<Global_List[value]<<" = alloc ";
    //全局变量类型
    if(global.Init->ty->tag == RTT_INT32)
        cout<<"i32, ";
        
    //全局变量初始值
    Visit_Value(global.Init);
    cout<<Symbol_List[global.Init]<<endl;
}
void GeneratorIRTxt(RawProgramme * &programme)
{
    // 访问函数
    Visit_Slice(programme->Value);
    Visit_Slice(programme->Funcs);
}
void Visit_Fun(const RawFunctionP &func)
{
        if(judgement(func->name))
            return;
        //初始化符号表
        init_symbol();
        //函数名称
        printf("fun @%s(",func->name);
        //函数参数
        for(int i=0;i<func->params.len;i++)
        {
            if(i!=0)cout<<',';
            cout<<"\%p_"<<i<<": i32";
        }
        //判断函数返回值类型
        printf("): i32");

        // 访问基本块
        printf("{");
        Visit_Slice(func->bbs);
        printf("}\n\n");
       
}
void Visit_BBS(const RawBasicBlockP &bb){
    cout <<"\n%"<<bb->name << ":" << endl;
    // 访问Value
    Visit_Slice(bb->insts);
}
void Visit_Value(const RawValueP &value) {    
    if(Symbol_List.find(value)!=Symbol_List.end())
    {
        // cout<<"value has alloc"<<endl;
        return ;
    }
    const auto& kind = value->value;
    switch(kind.tag) {
        case RVT_RETURN: {
            // cout<<"Value:{RVT_RETURN}"<<endl;
            Visit_Return(value);
            break;
        }
        case RVT_INTEGER: {
            // cout<<"Value:{RVT_INTEGER}"<<endl;
            Visit_Integer(value);
            break;
        }
        case RVT_BINARY: {
            // cout<<"Value:{RVT_BINARY}"<<endl;
            Visit_Binary(value);
            break;
        }
        case RVT_ALLOC: {
            // cout<<"Value:{RVT_ALLOC}"<<endl;
            Visit_Alloc(value);
            break;
        }
        case RVT_LOAD: {
            // cout<<"Value:{RVT_LOAD}"<<endl;
            Visit_Load(value);
            break;
        }
        case RVT_STORE: {
            // cout<<"Value:{RVT_STORE}"<<endl;
            Visit_Store(value);
            break;
        }
        case RVT_BRANCH: {
            // cout<<"Value:{RVT_BRANCH}"<<endl;
            Visit_Branch(value);
            break;
        }
        case RVT_JUMP: {
            // cout<<"Value:{RVT_JUMP}"<<endl;
            Visit_Jump(value);
            break;
        }
        case RVT_CALL:{
            // cout<<"Value:{RVT_CALL}"<<endl;
            Visit_Call(value);
            break;
        }
        case RVT_FUNC_ARGS:{
            // cout<<"Value:{RVT_FUNC_ARGS}"<<endl;
            Visit_Func_Args(value);
            break;
        }
        case RVT_GLOBAL:{
            // cout<<"Value:{RVT_GLOBAL}"<<endl;
            Visit_Global(value);
            break;
        }
        case RVT_ZEROINIT:{
            Visit_ZEROINIT(value);
        }
        default:
            // cout<<"tag:{"<<kind.tag<<'}'<<endl;
            assert(false);
    }
// }
}
void Visit_Slice(const RawSlice &slice)
{
    for(size_t i = 0; i < slice.len; i++) {
        // cout << "i = " << i << ", len = " << slice.len << endl;
        auto ptr = slice.buffer[i];
        switch(slice.kind) {
            case RSK_FUNCTION:
                // cout << "parsing function" << endl;
                Visit_Fun(reinterpret_cast<RawFunctionP>(ptr));
                break;
            case RSK_BASICBLOCK:
                // cout << "parsing BasicBlock" << endl;
                Visit_BBS(reinterpret_cast<RawBasicBlockP>(ptr));
                break;
            case RSK_BASICVALUE:
                // cout << "parsing Value" << endl;
                Visit_Value(reinterpret_cast<RawValueP>(ptr));
                break;
            default:
                assert(false);
        }
    }
}