#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
extern unordered_map <string,RawValueP> MidVarTable;

/// @brief 这个是通过查RawValue表实现创建RawValue
/// @param value 
/// @param sign 
void generateRawValue(RawValueP &value, string &sign){
    assert(MidVarTable.find(sign) != MidVarTable.end());
    value = MidVarTable.at(sign);
}

/// @brief 创建return型value
/// @param value 
/// @param src 
void generateRawValue(RawValue *&value, RawValueP src) {
    value = (RawValue *) malloc(sizeof(RawValue));
    value->name      = nullptr;
    value->value.tag = RVT_RETURN;
    value->value.data.ret.value = src;
}

void generateRawValue(RawValue *&value, RawValueP lhs, RawValueP rhs, uint32_t op){
    value = (RawValue *) malloc(sizeof(RawValue));
    value->name = nullptr;
    value->value.tag = RVT_BINARY;
    value->value.data.binary.lhs = lhs;
    value->value.data.binary.rhs = rhs;
    value->value.data.binary.op = op;
}

void generateRawValue(RawValue *&value, int32_t number,RawSlice &IR){
    if(MidVarTable.find(to_string(number)) != MidVarTable.end()) {
        value = (RawValue *)MidVarTable.at(to_string(number));
    }
    else {
        value = (RawValue *) malloc(sizeof(RawValue));
        value->name  = nullptr;
        value->value.tag = RVT_INTEGER;
        value->value.data.integer.value = number;
        IR.buffer[IR.len++] = (const void *) value;
        MidVarTable.insert(pair<string,RawValueP>(to_string(number),value));
    }
}

void generateRawValue(RawValueP &src, RawValueP &dest,RawSlice &IR){
    RawValue *store = (RawValue *) malloc(sizeof(RawValue));
    store->name = nullptr;
    store->value.tag = RVT_STORE;
    store->value.data.store.value = src;
    store->value.data.store.dest = dest;
    IR.buffer[IR.len++] = (const void *) store;
}
