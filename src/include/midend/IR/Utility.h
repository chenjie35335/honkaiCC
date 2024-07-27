#ifndef STORMY_VALUEKIND_DEF
#define STORMY_VALUEKIND_DEF
#include <cstdint>
#include <cstdlib>
// binary op
enum {
    /// Not equal to.
    RBO_NOT_EQ,
    /// float not equal to
    RBO_NOT_FEQ,
    /// Equal to.
    RBO_EQ,
    /// float equal to
    RBO_FEQ,
    /// Greater than.
    RBO_GT,
    /// float greater than
    RBO_FGT,
    /// Less than.
    RBO_LT,
    /// float less than
    RBO_FLT,
    /// Greater than or equal to
    RBO_GE,
    /// float greater than or equal to
    RBO_FGE,
    /// Less than or equal to.
    RBO_LE,
    /// float less than or equal to
    RBO_FLE,
    /// Addition.
    RBO_ADD,
    /// float addition
    RBO_FADD,
    /// Subtraction.    
    RBO_SUB,
    /// float subtraction
    RBO_FSUB,
    /// Multiplication.
    RBO_MUL,
    /// float multiplication
    RBO_FMUL,
    /// Division.
    RBO_DIV,
    /// float division
    RBO_FDIV,
    /// Modulo.
    RBO_MOD,
    /// Bitwise AND.
    RBO_AND,
    /// Bitwise OR.
    RBO_OR,
    /// Bitwise XOR.
    RBO_XOR,
    /// Shift left logical.
    RBO_SHL,
    /// Shift right logical.
    RBO_SHR,
    /// Shift right arithmetic.
    RBO_SAR,
};

enum {
    /// float multiply add
    RTO_FMADD,
    /// float multiply sub
    RTO_FMSUB,
    /// float multiply neg add
    RTO_FNMADD,
    /// float multiply neg sub
    RTO_FNMSUB
};

//the kind of value
enum {
    /// Integer constant0
    RVT_INTEGER,
    /// float constant1
    RVT_FLOAT,
    /// local alloc2
    RVT_ALLOC,
    /// load3
    RVT_LOAD,
    /// store4
    RVT_STORE,
    /// binary5
    RVT_BINARY,
    /// return6
    RVT_RETURN,
    /// branch7
    RVT_BRANCH,
    /// jump8
    RVT_JUMP,
    /// call9
    RVT_CALL,
    /// function args10
    RVT_FUNC_ARGS,
    /// basicblock args11
    RVT_BLOCK_ARGS,
    /// global12 
    RVT_GLOBAL,
    /// getptr13
    RVT_GET_PTR,
    /// getelement14
    RVT_GET_ELEMENT,
    /// aggregate15
    RVT_AGGREGATE,
    /// zeroinit16
    RVT_ZEROINIT,
    /// valuecopy17
    RVT_VALUECOPY,
    /// phi18
    RVT_PHI,
    /// value convert19
    RVT_CONVERT,
    /// triple20
    RVT_TRIPE,
};

enum {
    // 32-bits integer
    RTT_INT32,
    // float
    RTT_FLOAT,
    // void
    RTT_UNIT,
    // Array
    RTT_ARRAY,
    // pointer
    RTT_POINTER,
    // function
    RTT_FUNCTION
};



#endif