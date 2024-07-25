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
    else if("putfloat"==fun_name){
        cout<<"decl @putfloat(float)"<<endl;
        return true;
    }
    else if("getfloat"==fun_name){
        cout<<"decl @getfloat(): float"<<endl;
        return true;
    } 
    else if("getfarray"==fun_name){
        cout<<"decl @getfarray(*float): i32"<<endl;
        return true;
    } 
    else if("putfarray"==fun_name){
        cout<<"decl @putfarray(i32, *float)"<<endl;
        return true;
    }
    else
        return false;
}
void init_symbol()
{
    symbol_num = 0;
    ptr_idx = 0;
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
void alloc_ptr_symbol(const RawValueP &value)
{
    Symbol_List[value] = "\%ptr"+to_string(ptr_idx++);
}
string GetValueType(const RawTypeP &ty)
{
    switch(ty->tag){
        case RTT_INT32:{
            return string("i32");
        }
        case RTT_FLOAT:{
            return string("float");
            break;
        }
        case RTT_ARRAY:{
            string str = "["+GetValueType(ty->array.base)+", ";
            str += to_string(ty->array.len)+"]";
            return str;
        }
        case RTT_POINTER:{
            return GetValueType(ty->pointer.base);
        }
        default:{
            cerr << "{tag:} " << ty->tag << endl;
            assert(0);
        }
    }
}

string GetParamType(const RawTypeP &ty)
{
    switch(ty->tag){
        case RTT_INT32:{
            return string("i32");
        }
        case RTT_FLOAT:{
            return string("float");
            break;
        }
        case RTT_ARRAY:{
            string str = "["+GetValueType(ty->array.base)+", ";
            str += to_string(ty->array.len)+"]";
            return str;
        }
        case RTT_POINTER:{
            return "*"+GetValueType(ty->pointer.base);
        }
        default:{
            cerr << "{tag:} " << ty->tag << endl;
            assert(0);
        }
    }
}

void GetRetType(const RawFunctionP &func) {
auto &retType = func->ty->function.ret;
switch(retType->tag) {
    case RTT_INT32:{
        cout << "): i32" << endl;
        break;
    }
    case RTT_FLOAT:{
        cout << "): float" << endl;
        break;
    }
    case RTT_UNIT:{
        cout << ")" << endl;
        break;
    }
    default:
        assert(0);
}
}

void Visit_ZEROINIT(const RawValueP &value)
{
    Symbol_List[value] = "zeroinit";
}
void Visit_Return(const RawValueP &value)
{
    const auto& ret = value->value.ret;
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
    Symbol_List[value]=to_string(value->value.integer.value);
}
void Visit_Float(const RawValueP &value){
    Symbol_List[value]=to_string(value->value.floatNumber.value);
}
void Visit_Binary(const RawValueP &value)
{
    const RawBinary &data = value->value.binary;
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
        case RBO_FADD:
            cout<<res<<" = fadd "<<ls<<", "<<rs<<endl;
            break;
        case RBO_FSUB:
            cout<<res<<" = fsub "<<ls<<", "<<rs<<endl;
            break;
        case RBO_SUB:
            cout<<res<<" = sub "<<ls<<", "<<rs<<endl;
            break;
        case RBO_FEQ:
            cout<<res<<" = feq "<<ls<<", "<<rs<<endl;
            break;
        case RBO_EQ:
            cout<<res<<" = eq "<<ls<<", "<<rs<<endl;
            break;
        case RBO_NOT_FEQ:
            cout<<res<<" = fne "<<ls<<", "<<rs<<endl;
            break;
        case RBO_NOT_EQ:
            cout<<res<<" = ne "<<ls<<", "<<rs<<endl;
            break;
        case RBO_FMUL:
            cout<<res<<" = fmul "<<ls<<", "<<rs<<endl;
            break;
        case RBO_MUL:
            cout<<res<<" = mul "<<ls<<", "<<rs<<endl;
            break;
        case RBO_FDIV:
            cout<<res<<" = fdiv "<<ls<<", "<<rs<<endl;
            break;
        case RBO_DIV:
            cout<<res<<" = div "<<ls<<", "<<rs<<endl;
            break;
        case RBO_MOD:
            cout<<res<<" = mod "<<ls<<", "<<rs<<endl;
            break;
        case RBO_FLT: 
            cout<<res<<" = flt "<<ls<<", " <<rs<<endl;
            break;
        case RBO_LT:
            cout<<res<<" = lt "<<ls<<", "<<rs<<endl;
            break;
        case RBO_FGT: 
            cout<<res<<" = fgt"<<ls<<", " <<rs<<endl;
            break;
        case RBO_GT:
            cout<<res<<" = gt "<<ls<<", "<<rs<<endl;
            break;
        case RBO_FGE: 
            cout<<res<<" = fge"<<ls<<", " <<rs<<endl;
            break;
        case RBO_GE:
            cout<<res<<" = ge "<<ls<<", "<<rs<<endl;
            break;
        case RBO_FLE: 
            cout<<res<<" = fle"<<ls<<", " <<rs<<endl;
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
        default: {
            cerr << "unknown {op:} " << op << endl;
            assert(0);
        }
    }
}

void Visit_Convert(const RawValueP &value) {
    alloc_symbol(value);
    string res = Symbol_List[value];
    Visit_Value(value->value.Convert.src);
    string src = Symbol_List[value->value.Convert.src];
    cout<<"  "<<res<<" = Convert "<<src<<endl;
}

void Visit_Alloc(const RawValueP &value)
{
    if(SSAmode){
        var_id[value]=0;//变量首次定义
    }
    
    Symbol_List[value] = "@"+string(value->name);
    cout<<"  "<<Symbol_List[value]<< " = alloc "<<GetValueType(value->ty)<<endl;
}
void Visit_Load(const RawValueP &value)
{
    alloc_symbol(value);
    string res = Symbol_List[value];
    Visit_Value(value->value.load.src);
    string src = Symbol_List[value->value.load.src];
    cout<<"  "<<res<<" = load "<<src<<endl;
}
void Visit_Store(const RawValueP &value)
{
    Visit_Value(value->value.store.value);
    string sv = Symbol_List[value->value.store.value];
    RawValueP dest = value->value.store.dest;
    if(SSAmode)//多次赋值
    {
        RawValueP target = dest->value.valueCop.target;
        var_id[target]++;
        Symbol_List[dest] = "@"+ string(target->name) +'_'+to_string(var_id[target]); 
    }
    else{
        Visit_Value(value->value.store.dest);
    }
    string sd = Symbol_List[value->value.store.dest];
    cout<<"  store "<<sv<<", "<<sd<<endl;
}
void Visit_Branch(const RawValueP &value)
{
    const RawBranch branch = value->value.branch;
    Visit_Value(branch.cond);
    cout<<"  br "<<Symbol_List[branch.cond]<<", %"<<branch.true_bb->name<<", %"<<branch.false_bb->name<<endl;
}
void Visit_Jump(const RawValueP &value)
{
    cout<<"  jump %"<<value->value.jump.target->name<<endl;
}
void Visit_Call(const RawValueP &value)
{
    const RawCall call = value->value.call;
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
    for(auto arg : call.args)
        Visit_Value(arg);
    for(auto arg =  call.args.begin(); arg != call.args.end();arg++)
    {
        if(arg!=call.args.begin())
            cout<<',';
        cout<<Symbol_List[*arg];
    }
    cout<<')'<<endl;
    
}
void Visit_Func_Args(const RawValueP &value)
{
    int index = value->value.funcArgs.index;
    // 为参数重新分配空间
    Symbol_List[value] = "@p_"+to_string(index);
    //cout<<"  "<<Symbol_List[value]<< " = alloc "<<GetValueType(value->ty)<<endl;
    //cout<<"  store "<<"\%p_"<<index<<", "<<Symbol_List[value]<<endl;
}
void Visit_Global(const RawValueP &value)
{
    Global_List[value] = '@'+string(value->name);
    RawGlobal global = value->value.global;

    cout<<"global "<<Global_List[value]<<" = alloc "<<GetValueType(value->ty)<<", ";
        
    //全局变量初始值
    Visit_Value(global.Init);
    cout<<Symbol_List[global.Init]<<endl;
}
void visit_get_element(const RawValueP &value)
{
    alloc_ptr_symbol(value);
    RawValueP src = value->value.getelement.src;
    RawValueP index = value->value.getelement.index;
    Visit_Value(src);
    Visit_Value(index);
    cout<<"  "<<Symbol_List[value]<<" = getelemptr "<<Symbol_List[src]<<", "<<Symbol_List[index]<<endl;
}
void visit_aggregate(const RawValueP &value)
{
    string content = "{";
    vector<RawValue *> elements = value->value.aggregate.elements;
    for(auto elem:elements)
    {
        uint32_t tag = elem->value.tag;
        if(tag == RVT_INTEGER){content+=to_string(elem->value.integer.value);}
        else if(tag == RVT_FLOAT){content+=to_string(elem->value.floatNumber.value);}
        else {Visit_Value(elem);content+=Symbol_List[elem];}
        if(elem != elements.back())
            content+=", ";
    }
    content+='}';
    Symbol_List[value] = content;
}
void visit_get_ptr(const RawValueP &value)
{
    RawValueP src = value->value.getptr.src;
    Visit_Value(src);
    RawValueP index = value->value.getptr.index;
    Visit_Value(index);
    alloc_ptr_symbol(value);
    cout<<"  "<<Symbol_List[value]<<" = getptr "<<Symbol_List[src]<<", "<<Symbol_List[index]<<endl;
}
void Visit_PHI(const RawValueP &value) {
    if(!SSAmode){

        cout<<"在非SSA模式的IR中使用PHI函数"<<endl;
        return;
    }
    RawValueP target =value->value.phi.target;
    var_id[target]++;
    Symbol_List[value] = "@"+ string(target->name) +'_'+to_string(var_id[target]);
    for(auto pvalue:value->value.phi.phi)
    {
        Visit_Value(pvalue.second);
    }
    
    cout<<"  "<<Symbol_List[value]<<" = phi {";
     for(auto pvalue:value->value.phi.phi){
        cout<<'('<<pvalue.first->name<<','<<Symbol_List[pvalue.second]<<')';
        if(pvalue != value->value.phi.phi.back())
            cout<<", ";
     }
    cout<<'}'<<endl;
}

void Visit_VALUECOPY(const RawValueP &value) {
    if(!SSAmode){

        cout<<"在非SSA模式的IR中使用ValueCopy"<<endl;
        return;
    }

    RawValueP  target = value->value.valueCop.target;
    Symbol_List[value] = "@"+string(target->name)+'_'+to_string(var_id[target]);
    //cout << Symbol_List[value] << " = " << "alloc i32" << endl;
    // cout <<"regard as copy"<< endl;
}

void Visit_BBS(const RawBasicBlockP &bb){
    auto &insts = bb->inst;
    auto &phis = bb->phi;
    cout <<"\n%"<<bb->name << ":" << endl;
    if(SSAmode) {
    // 访问Value
    for(auto phi : phis) 
    {
        Visit_Value(phi);
    }
    }
    for(auto inst : insts) 
    {
        // cout<<"valueType:{"<<inst->value.tag<<"}"<<endl;
        Visit_Value(inst);
        //定值点
        // cout<<inst->value.tag<<'[';
        // for(auto defpoint:inst->defPoints)
        // {
        //     cout<<Symbol_List[defpoint]<<'|';
        // }
        // cout<<inst->defPoints.size();
        // cout<<']'<<endl;
    }
    // for(auto inst : insts) 
    // {
    //     //使用点
    //     cout<<inst->value.tag<<'[';
    //     for(auto defpoint:inst->usePoints)
    //     {
    //         Visit_Value(defpoint);
    //         cout<<'|';
    //     }
    //     cout<<']'<<inst->usePoints.size()<<endl;
    // }

    //生成定义变量
    // cout<<bb->name<<"基本块定义的变量:[";
    // for(auto val_def:bb->defs){
    //     cout<<Symbol_List[val_def]<<'|';
    // }
    // cout<<']'<<endl;
    //生成使用变量
    // cout<<bb->name<<"基本块使用的变量:[";
    // for(auto val_ues:bb->uses){
    //     cout<<Symbol_List[val_ues]<<'|';
    // }
    // cout<<']'<<endl;
}
void Visit_Fun(const RawFunctionP &func)
{
        auto &bbs = func->basicblock;
        auto &params = func->params;
        if(judgement(func->name))
            return;
        //初始化符号表
        init_symbol();
        //函数名称
        printf("fun @%s(",func->name);
        //函数参数
        for(int i=0;i<params.size();i++)
        {
            if(i!=0)cout<<',';
            cout<<"@p_"<<i<<": "<<GetParamType(params[i]->ty);
        }
        //判断函数返回值类型
        // cout<<"): i32";
        GetRetType(func);
        // cout<<"tag:"<<func->ty->tag<<endl;

        // 访问基本块
        printf("{");
        for(auto &bb : bbs)
            Visit_BBS(bb);
        printf("}\n\n");
       
}

void Visit_Value(const RawValueP &value) {    
    if(Symbol_List.find(value)!=Symbol_List.end())
    {
        // cout<<"value has alloc"<<endl;
        return ;
    }
    const auto& kind = value->value;
    // cout<<"valueType:{"<<kind.tag<<"}"<<endl;
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
        case RVT_FLOAT:{
            //cout<<"Value:{RVT_FLOAT}"<<endl;
            Visit_Float(value);
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
            Visit_ZEROINIT(value);break;
        }
        case RVT_GET_PTR:{
            // cout<<"Value:{RVT_GET_PTR}"<<endl;
            visit_get_ptr(value);
            break;
        }
        case RVT_GET_ELEMENT:{
            // cout<<"Value:{RVT_GET_ELEMENT}"<<endl;
            visit_get_element(value);
            break;
        }
        case RVT_AGGREGATE:
        {
            // cout<<"Value:{RVT_AGGREGATE}"<<endl;
            visit_aggregate(value);
            break;
        }
        case RVT_VALUECOPY: {
            Visit_VALUECOPY(value);break;
        }
        case RVT_PHI: {
            // cout<<"Value:{RVT_PHI}"<<endl;
            Visit_PHI(value);break;
        }
        case RVT_CONVERT: {
            Visit_Convert(value);break;
        }
        default:
            cerr<<"tag:{"<<kind.tag<<'}'<<endl;
            assert(false);
    }

}

void GeneratorIRTxt(RawProgramme * &programme,bool isSSAmode)
{
    // 访问函数
    SSAmode = isSSAmode;//确定IR的输出形式
    auto &values = programme->values;
    auto &funcs = programme->funcs;
    for(auto &value : values) Visit_Value(value);
    for(auto &func : funcs) Visit_Fun(func);
}