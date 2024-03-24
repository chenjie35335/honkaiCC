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
    value->name = nullptr;
    value->value.tag = RVT_RETURN;
    value->value.data.ret.value = src;
    insts.buffer[insts.len++] = (const void *)value;
}

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
    insts.buffer[insts.len++] = (const void *)value;
    MidVarTable.insert(pair<string, RawValueP>(sign, value));
}

void generateRawValue(int32_t number)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
        RawValue * value = (RawValue *)malloc(sizeof(RawValue));
        value->name = nullptr;
        value->value.tag = RVT_INTEGER;
        value->value.data.integer.value = number;
        insts.buffer[insts.len++] = (const void *)value;
        if(MidVarTable.find(to_string(number)) != MidVarTable.end()) {
            MidVarTable[to_string(number)] = value;
        }
        else {
        MidVarTable.insert(pair<string, RawValueP>(to_string(number), value));
        }
}

void generateRawValue(RawValueP &src, RawValueP &dest)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *store = (RawValue *)malloc(sizeof(RawValue));
    store->name = nullptr;
    store->value.tag = RVT_STORE;
    store->value.data.store.value = src;
    store->value.data.store.dest = dest;
    insts.buffer[insts.len++] = (const void *)store;
}

void generateRawValue(string sign)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *alloc = (RawValue *)malloc(sizeof(RawValue));
    alloc->value.tag = RVT_ALLOC;
    insts.buffer[insts.len++] = (const void *)alloc;
    MidVarTable.insert(pair<string, RawValueP>(sign, alloc));
}

void generateRawValue(string &sign, RawValueP &src)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue * load = (RawValue *)malloc(sizeof(RawValue));
    load->name = nullptr;
    load->value.tag = RVT_LOAD;
    load->value.data.load.src = src;
    insts.buffer[insts.len++] = (const void *)load;
    MidVarTable.insert(pair<string,RawValueP>(sign,load));
}

void generateRawValue(RawValueP &cond, RawBasicBlock* &Truebb, RawBasicBlock* &Falsebb){
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *br = (RawValue *) malloc(sizeof(RawValue));
    br->name = nullptr;
    br->value.tag = RVT_BRANCH;
    br->value.data.branch.cond = cond;
    br->value.data.branch.true_bb = (RawBasicBlockP)Truebb;
    br->value.data.branch.false_bb = (RawBasicBlockP)Falsebb;
    insts.buffer[insts.len++] = br;
}
 
void generateRawValue(RawBasicBlock* &TargetBB){
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *jump = (RawValue *) malloc(sizeof(RawValue));
    jump->name = nullptr;
    jump->value.tag = RVT_JUMP;
    jump->value.data.jump.target = (RawBasicBlockP)TargetBB;
    insts.buffer[insts.len++] = jump;
}