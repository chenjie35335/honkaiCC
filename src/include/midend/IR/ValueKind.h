#ifndef STORMY_VALUEKIND
#define STORMY_VALUEKIND
#include <stdint.h>
#include <stdlib.h>
#include "common.h"

class RawInteger {
    public:
    int32_t value;
};
/// load
class RawLoad{
    public:
    RawValueP src;
};
///
class RawGlobal{
    public:
    RawValueP Init;
};

/// store
class RawStore{
    public:
    RawValueP value;
    RawValueP dest;
};

/// binary
class RawBinary{
    public:
    /// kind of op
    uint32_t op;
    RawValueP lhs;
    RawValueP rhs;
};

/// return 
class RawReturn{
    public:
    RawValueP value;
} ;
/// call
class RawCall{
    public:
    /// @brief the called function
    RawFunctionP callee;
    /// @brief params
    RawSlice args;
};
/// @brief args
class RawFuncArgs{
    public:
    /// index of args in the function
    size_t index;
};
/// @brief jump
class RawJump{
    public:
    /// @brief the target basicblock
    RawBasicBlockP target;
    /// @brief the args of basicblock
    RawSlice args;
};

class RawGetPtr{
    public: 
    RawValueP src;
    RawValueP index;
};

class RawGetElement{
    public: 
    RawValueP src;
    RawValueP index;
};

class RawAggregate{
    public: 
    RawSlice elements; 
};

class RawBranch{//其他剩余两个参数貌似当前用不到
public:
    /// condition
    RawValueP cond;
    /// Target if condition is true
    RawBasicBlockP true_bb;
    /// Target if condition is false
    RawBasicBlockP false_bb;
    /// the args of true_bb
    RawSlice true_args;
    /// the args of false_bb
    RawSlice false_args;
};

class RawValueCop {
    public:
        /// @brief target of this copy
        RawValueP target;
};

class RawBlockArgs{
public:
    /// @brief the target of phi function
    RawValueP target;
    /// @brief the index of args
    size_t index;
};

class ValueKind {
    public:
    uint32_t tag;
    union {
        RawInteger integer;
        RawLoad load;
        RawStore store;
        RawBinary binary;
        RawReturn ret;
        RawBranch branch;
        RawJump jump;
        RawCall call;
        RawFuncArgs funcArgs;
        RawGlobal global;
        RawGetPtr getptr;
        RawGetElement getelement;
        RawAggregate aggregate;
        RawBlockArgs blockArgs;
        RawValueCop valueCop;
        // 其他数据类型
    } data;
};
#endif