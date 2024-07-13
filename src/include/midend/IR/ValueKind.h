#ifndef STORMY_VALUEKIND
#define STORMY_VALUEKIND
#include <stdint.h>
#include <stdlib.h>
#include <vector>
#include "common.h"
using namespace std;

class RawInteger {
    public:
    int32_t value;
    RawInteger(){}
};

//float
class RawFloat {
    public:
    float value;
    RawFloat() {}
};

/// load
class RawLoad{
    public:
    RawValueP src;
    RawLoad(){}
};
///
class RawGlobal{
    public:
    RawValueP Init;
    RawGlobal() {}
};

/// store
class RawStore{
    public:
    RawValueP value;
    RawValueP dest;
    RawStore() {}
};

/// binary
class RawBinary{
    public:
    /// kind of op
    uint32_t op;
    RawValueP lhs;
    RawValueP rhs;
    RawBinary() {}
};

/// return 
class RawReturn{
    public:
    RawValueP value;
    RawReturn() {}
} ;
/// call
class RawCall{
    public:
    /// @brief the called function
    RawFunctionP callee;
    /// @brief params
    vector<RawValue *> args;

    RawCall() {}
};
/// @brief args
class RawFuncArgs{
    public:
    /// index of args in the function
    size_t index;

    RawFuncArgs() {}
};
/// @brief jump
class RawJump{
    public:
    /// @brief the target basicblock
    RawBasicBlockP target;

    RawJump() {}
};

class RawGetPtr{
    public: 
    RawValueP src;
    RawValueP index;

    RawGetPtr() {}
};

class RawGetElement{
    public: 
    RawValueP src;
    RawValueP index;

    RawGetElement() {}
};

class RawAggregate{
    public: 
    vector<RawValue *> elements;

    RawAggregate() {} 
};

class RawPhi {
    public:
    RawValueP target;
    vector<pair<RawBasicBlock *,RawValue *>> phi;

    RawPhi() {}
};

class RawBranch{//其他剩余两个参数貌似当前用不到
public:
    /// condition
    RawValueP cond;
    /// Target if condition is true
    RawBasicBlockP true_bb;
    /// Target if condition is false
    RawBasicBlockP false_bb;

    RawBranch() {}
};

class RawValueCop {
    public:
        /// @brief target of this copy
        RawValueP target;

        RawValueCop() {}
};

class RawConvert{
    public:
        RawValueP src;

        RawConvert() {}
};


class RawBlockArgs{
public:
    /// @brief the target of phi function
    RawValueP target;
    /// @brief the index of args
    size_t index;

    RawBlockArgs() {}
};

class ValueKind {
    public:
        uint32_t tag;
        RawInteger integer;
        RawFloat floatNumber;
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
        RawValueCop valueCop;
        RawPhi phi;
        RawConvert Convert;
        // 其他数据类型

    ValueKind() {
        
    }
    ValueKind(ValueKind *oldvalue) {
        this->tag=oldvalue->tag;
    }
};
#endif