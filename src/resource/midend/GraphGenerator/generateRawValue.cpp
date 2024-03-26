#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include <cstdlib>
#include <unordered_map>
extern unordered_map<string, RawValueP> MidVarTable;

/// @brief 这个是通过查RawValue表实现创建RawValue
/// @param value
/// @param sign
void generateRawValue(RawValueP &value, string &sign)
{
    assert(MidVarTable.find(sign) != MidVarTable.end());
    value = MidVarTable.at(sign);
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
void generateRawValue(string &sign, RawValueP lhs, RawValueP rhs, uint32_t op)
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
    MidVarTable.insert(pair<string, RawValueP>(sign, value));
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
        if(MidVarTable.find(to_string(number)) != MidVarTable.end()) {
            MidVarTable[to_string(number)] = value;
        }
        else {
        MidVarTable.insert(pair<string, RawValueP>(to_string(number), value));
        }
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
void generateRawValue(string sign)
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
    MidVarTable.insert(pair<string, RawValueP>(sign, alloc));
}
/// @brief load型value
/// @param sign 
/// @param src 
void generateRawValue(string &sign, RawValueP &src)
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
    MidVarTable.insert(pair<string,RawValueP>(sign,load));
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

void createRawProgramme(RawProgramme *&Programme) {
    Programme = (RawProgramme *) malloc(sizeof(RawProgramme));
    auto &funcs = Programme->Funcs;
    funcs.kind = RSK_FUNCTION;
    funcs.len  = 0;
    funcs.buffer = (const void **) malloc(sizeof(const void *) * 100);
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

void generateRawFunction(RawFunction *&function, const string &name) {
    auto programme = getTempProgramme();
    auto &funcs = programme->Funcs;
    function = (RawFunction *) malloc(sizeof(RawFunction));
    auto &bbs = function->bbs;
    bbs.kind = RSK_BASICBLOCK;bbs.len = 0;
    bbs.buffer = (const void **) malloc(sizeof(const void *)*100);
    function->name = name.c_str();
    funcs.buffer[funcs.len++] = (const void *) function;
    setTempFunction(function);
}