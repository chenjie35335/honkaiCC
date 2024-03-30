#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
extern SignTable signTable;
/// @brief 这个是通过查符号表获取的RawValue(包括中间变量和常数)
/// @param value
/// @param sign
void getMidVarValue(RawValueP &value, string &name)
{
    value = (RawValueP) signTable.getMidVar(name);
}
/// @brief 查符号表获取变量左值
/// @param value 
/// @param name 
void getVarValueL(RawValueP &value,string &name) 
{
    value = (RawValueP) signTable.getVarL(name);
}
/// @brief 查符号表获取变量右值
/// @param value 
/// @param name 
void getVarValueR(RawValueP &value,string &name)
{
    value = (RawValueP) signTable.getVarR(name);
}
/// @brief 创建return型value
/// @param value
/// @param src
void generateRawValue(RawValueP src)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue * value = (RawValue *)malloc(sizeof(RawValue));
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_UNIT;
    value->name = nullptr;
    value->value.tag = RVT_RETURN;
    value->value.data.ret.value = src;
    value->ty = (RawTypeP) ty;
    insts.buffer[insts.len++] = (const void *)value;
}
/// @brief binary型value
/// @param sign 
/// @param lhs 
/// @param rhs 
/// @param op 
void generateRawValue(string &name, RawValueP lhs, RawValueP rhs, uint32_t op)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue * value = (RawValue *)malloc(sizeof(RawValue));
    value->name = nullptr;
    value->value.tag = RVT_BINARY;
    value->value.data.binary.lhs = lhs;
    value->value.data.binary.rhs = rhs;
    value->value.data.binary.op = op;
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_INT32;
    value->ty = (RawTypeP) ty;
    insts.buffer[insts.len++] = (const void *)value;
    signTable.insertMidVar(name,value);
}
/// @brief number型value
/// @param number 
void generateRawValue(int32_t number)
{
        auto bb = getTempBasicBlock();
        auto &insts = bb->insts;
        RawValue * value = (RawValue *)malloc(sizeof(RawValue));
        value->name = nullptr;
        value->value.tag = RVT_INTEGER;
        value->value.data.integer.value = number;
        RawType *ty = (RawType *) malloc(sizeof(RawType));
        ty->tag = RTT_INT32;
        value->ty = ty;
        insts.buffer[insts.len++] = (const void *)value;
        signTable.insertNumber(number,value);
}
/// @brief store型value
/// @param src 
/// @param dest 
void generateRawValue(RawValueP &src, RawValueP &dest)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *store = (RawValue *)malloc(sizeof(RawValue));
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_UNIT;
    store->ty = (RawTypeP)ty;
    store->name = nullptr;
    store->value.tag = RVT_STORE;
    store->value.data.store.value = src;
    store->value.data.store.dest = dest;
    insts.buffer[insts.len++] = (const void *)store;
}
/// @brief alloc型value
/// @param sign 
void generateRawValue(string& name)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *alloc = (RawValue *)malloc(sizeof(RawValue));
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_POINTER;
    RawType *pointerTy = (RawType *) malloc(sizeof(RawType));
    pointerTy->tag = RTT_INT32;
    ty->data.pointer.base = pointerTy;
    alloc->ty = (RawTypeP)ty;
    alloc->value.tag = RVT_ALLOC;
    insts.buffer[insts.len++] = (const void *)alloc;
    signTable.insertVar(name,alloc);
}
/// @brief load型value
/// @param sign 
/// @param src 
void generateRawValue(string &name, RawValueP &src)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue * load = (RawValue *)malloc(sizeof(RawValue));
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_INT32;
    load->ty = (RawTypeP) ty;
    load->name = nullptr;
    load->value.tag = RVT_LOAD;
    load->value.data.load.src = src;
    insts.buffer[insts.len++] = (const void *)load;
    signTable.insertMidVar(name,load);
}
/// @brief branch型value
/// @param cond 
/// @param Truebb 
/// @param Falsebb 
void generateRawValue(RawValueP &cond, RawBasicBlock* &Truebb, RawBasicBlock* &Falsebb){
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *br = (RawValue *) malloc(sizeof(RawValue));
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_UNIT;
    br->ty = (RawTypeP) ty;
    br->name = nullptr;
    br->value.tag = RVT_BRANCH;
    br->value.data.branch.cond = cond;
    br->value.data.branch.true_bb = (RawBasicBlockP)Truebb;
    br->value.data.branch.false_bb = (RawBasicBlockP)Falsebb;
    insts.buffer[insts.len++] = br;
}
/// @brief jump型value
/// @param TargetBB 
void generateRawValue(RawBasicBlock* &TargetBB){
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *jump = (RawValue *) malloc(sizeof(RawValue));
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_UNIT;
    jump->ty = (RawTypeP) ty;
    jump->name = nullptr;
    jump->value.tag = RVT_JUMP;
    jump->value.data.jump.target = (RawBasicBlockP)TargetBB;
    insts.buffer[insts.len++] = jump;
}
/// @brief call型value
void generateRawValue(RawFunctionP callee,vector<RawValueP> paramsValue,string &sign){
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *call = (RawValue *) malloc(sizeof(RawValue));
    call->name = nullptr;
    call->value.tag = RVT_CALL;
    call->value.data.call.callee = callee;
    auto &params = call->value.data.call.args;
    params.buffer = (const void **) malloc(sizeof(const void *)*100);
    params.kind = RSK_BASICVALUE;
    params.len = 0;
    auto &calleeParams = callee->ty->data.function.params;
    if(calleeParams.len != paramsValue.size()) {
        cerr << "wrong variables, has " <<  paramsValue.size() << ", expect " << calleeParams.len << endl;
        assert(0);
    }
    for(int i = 0; i < paramsValue.size();i++) {
        auto funcParamType = reinterpret_cast<RawTypeP>(callee->ty->data.function.params.buffer[i]); 
        auto expectType = funcParamType->tag;
        auto actualType = paramsValue[i]->ty->tag;
        if(expectType == actualType) {
            params.buffer[params.len++] = (const void *) paramsValue[i];
            continue;
        }
        else {//如果是浮点数的话，这里需要类型转换

        }
    } 
    auto retType = callee->ty->data.function.ret->tag;
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = retType;
    call->ty = ty;
    switch(retType) {
        case RTT_INT32:
        {
            alloc_now++;
            sign =  "%" + to_string(alloc_now);
            signTable.insertMidVar(sign,call);
            break;
        }
        case RTT_UNIT:  
            break;
        default:
            assert(0);
    } 
    insts.buffer[insts.len++] = (const void *) call;
}//call的类型和function的返回值相同

/// @brief args型value
void generateRawValueArgs(const string &ident,int index){
    auto function = getTempFunction();
    auto &params = function->params;
    RawValue *value = (RawValue *) malloc(sizeof(RawValue));
    value->value.tag = RVT_FUNC_ARGS;
    value->value.data.funcArgs.index = index;
    value->name = nullptr;
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_INT32;
    value->ty = ty;
    params.buffer[params.len++] = (const void *)value;
    auto &paramsTy = function->ty->data.function.params;
    paramsTy.buffer[paramsTy.len++] = (const void *) ty;
    signTable.insertVar(ident,value);
}

void createRawProgramme(RawProgramme *&Programme) {
    Programme = (RawProgramme *) malloc(sizeof(RawProgramme));
    auto &funcs = Programme->Funcs;
    funcs.kind = RSK_FUNCTION;
    funcs.len  = 0;
    funcs.buffer = (const void **) malloc(sizeof(const void *) * 100);
    auto &value = Programme->Value;
    value.kind = RSK_BASICVALUE;
    value.len = 0;
    value.buffer = (const void **) malloc(sizeof(const void **)*100);
}


void generateRawBasicBlock(RawBasicBlock *&bb, const string &name){
    bb = (RawBasicBlock *) malloc(sizeof(RawBasicBlock));
    bb->name = name;
    auto &insts = bb->insts;
    insts.kind = RSK_BASICVALUE;
    insts.len = 0;
    insts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
}
//将basicBlock挂在Function下
void PushRawBasicBlock(RawBasicBlock *&bb) {
    auto function = getTempFunction();
    auto &bbs = function->bbs;
    bbs.buffer[bbs.len++] = (const void *)bb;
}
//初始化时不进行对于类型的操作，等到后面一起修改
void generateRawFunction(RawFunction *&function, const string &name,int type) {
    auto programme = getTempProgramme();
    auto &funcs = programme->Funcs;
    function = (RawFunction *) malloc(sizeof(RawFunction));
    auto &bbs = function->bbs;
    bbs.kind = RSK_BASICBLOCK;
    bbs.len = 0;
    bbs.buffer = (const void **) malloc(sizeof(const void *)*100);
    auto &params = function->params;
    params.kind = RSK_BASICVALUE;
    params.len = 0;
    params.buffer = (const void **) malloc(sizeof(const void *)*100);
    function->name = name.c_str();
    RawType *ty = (RawType *) malloc(sizeof(RawType));
    ty->tag = RTT_FUNCTION;
    RawType *retTy = (RawType *) malloc(sizeof(RawType));
    switch(type) {
        case FUNCTYPE_INT:
            retTy->tag = RTT_INT32; break;
        case FUNCTYPE_VOID:
            retTy->tag = RTT_UNIT;break;
        default:
            assert(0);
    }
    ty->data.function.ret = retTy;
    auto &ParamTy = ty->data.function.params;
    ParamTy.kind = RSK_TYPE;
    ParamTy.buffer = (const void **) malloc(sizeof(const void *)*100);
    ParamTy.len = 0;
    function->ty = ty;
    funcs.buffer[funcs.len++] = (const void *) function;
    setTempFunction(function);
}