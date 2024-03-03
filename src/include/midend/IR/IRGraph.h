#ifndef STORMY_IRGRAPH_DEF
#define STORMY_IRGRAPH_DEF
#include <vector>
#include <cstdint>
using namespace std;
typedef struct ValueKind ValueKind;
typedef struct RawValue RawValue;
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
typedef struct RawFunction{
    /// name of function
    const char *name;
    /// parameter(not used until now)
    RawSlice params;
    /// basic blocks
    RawSlice bbs;
}RawFunction;
/// basic block
typedef struct RawBasicBlock{
/// name of bb
    const char *name;
/// parameter(not used until now)
    RawSlice params;
/// instructions
    RawSlice insts;
}RawBasicBlock;
//这里还是将regsiter和memory分开，不放在一起
/// integer
typedef struct {
    int32_t value;
}RawInteger;
/// load
typedef struct {
    RawValue* src;
} RawLoad;

/// store
typedef struct{
    RawValue* value;
    RawValue* dest;
} RawStore;

/// binary
typedef struct{
    /// kind of op
    uint32_t op;
    RawValue* lhs;
    RawValue* rhs;
} RawBinary;

typedef struct{
    RawValue* value;
} RawReturn;

struct ValueKind {
    uint32_t tag;
    union {
        RawInteger integer;
        RawLoad load;
        RawStore store;
        RawBinary binary;
        RawReturn ret;
        // 其他数据类型
    } data;
};

struct RawValue {
    const char* name;
    ValueKind* value;
};

#endif

