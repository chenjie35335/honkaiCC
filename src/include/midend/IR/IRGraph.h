#ifndef STORMY_IRGRAPH_DEF
#define STORMY_IRGRAPH_DEF
#include <vector>
#include <cstdint>
#include <string>
using namespace std;

typedef struct ValueKind ValueKind;

typedef struct RawValue RawValue;

typedef struct RawFunction RawFunction;

typedef const RawFunction * RawFunctionP;

typedef struct RawBasicBlock RawBasicBlock;

typedef const RawBasicBlock * RawBasicBlockP;

typedef const RawValue *RawValueP;
/// A raw slice that can store any kind of items.
typedef struct {
    /// Buffer of slice item
    const void ** buffer;   
    /// The length of array buffer
    uint32_t len;
    /// kind of RawSlice
    uint32_t kind;
}RawSlice;

/// programme
typedef struct RawProgramme{
    /// global values
    RawSlice Value;
    /// global functions
    RawSlice Funcs;
}RawProgramme;
/// function
struct RawFunction{
    /// name of function
    const char *name;
    /// parameter(not used until now)
    RawSlice params;
    /// basic blocks
    RawSlice bbs;
};
/// basic block
struct RawBasicBlock{
/// name of bb
    string name;
/// parameter(not used until now)
    RawSlice params;
/// instructions
    RawSlice insts;
};
//这里还是将regsiter和memory分开，不放在一起
/// integer
typedef struct {
    int32_t value;
}RawInteger;
/// load
typedef struct {
    RawValueP src;
} RawLoad;

/// store
typedef struct{
    RawValueP value;
    RawValueP dest;
} RawStore;

/// binary
typedef struct{
    /// kind of op
    uint32_t op;
    RawValueP lhs;
    RawValueP rhs;
} RawBinary;

/// return 
typedef struct{
    RawValueP value;
} RawReturn;

typedef struct{
    RawBasicBlockP target;
} RawJump;

typedef struct{//其他剩余两个参数貌似当前用不到
    /// condition
    RawValueP cond;
    /// Target if condition is true
    RawBasicBlockP true_bb;
    /// Target if condition is false
    RawBasicBlockP false_bb;
} RawBranch;

struct ValueKind {
    uint32_t tag;
    union {
        RawInteger integer;
        RawLoad load;
        RawStore store;
        RawBinary binary;
        RawReturn ret;
        RawBranch branch;
        RawJump jump;
        // 其他数据类型
    } data;
};

struct RawValue {
    const char * name;
    ValueKind value;
};
/// @brief 创建RawValue对象（其实这里由于是面向过程的，结果比较差）
/// @param value 
/// @param sign 
void generateRawValue(RawValueP &value, string &sign);

void generateRawValue(string &sign, RawValueP lhs, RawValueP rhs, uint32_t op);

void generateRawValue(RawValueP src);

void generateRawValue(int32_t number);

void generateRawValue(RawValueP &src, RawValueP &dest);

void generateRawValue(string sign);

void generateRawValue(string &sign, RawValueP &src);

void generateRawValue(RawValueP &cond, RawBasicBlock* &Truebb, RawBasicBlock* &Falsebb);

void generateRawValue(RawBasicBlock* &TargetBB);
#endif

