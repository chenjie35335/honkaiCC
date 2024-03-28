#ifndef STORMY_VALUEKIND_DEF
#define STORMY_VALUEKIND_DEF
// binary op
enum {
    /// Not equal to.
    RBO_NOT_EQ,
    /// Equal to.
    RBO_EQ,
/// Greater than.
    RBO_GT,
/// Less than.
    RBO_LT,
/// Greater than or equal to
    RBO_GE,
/// Less than or equal to.
    RBO_LE,
/// Addition.
    RBO_ADD,
/// Subtraction.    
    RBO_SUB,
/// Multiplication.
    RBO_MUL,
/// Division.
    RBO_DIV,
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
}BinaryOp;

//the kind of value
enum {
    /// Integer constant
    RVT_INTEGER,
    /// local alloc
    RVT_ALLOC,
    /// load
    RVT_LOAD,
    /// store
    RVT_STORE,
    /// binary
    RVT_BINARY,
    /// return 
    RVT_RETURN,
    /// branch
    RVT_BRANCH,
    /// jump
    RVT_JUMP,
    /// call
    RVT_CALL,
    /// function args
    RVT_FUNC_ARGS,
}RawValueTag;

// kind of RawSlice
enum {
    /// function
    RSK_FUNCTION,
    /// basic block
    RSK_BASICBLOCK,
    /// Value
    RSK_BASICVALUE,
    /// Type
    RSK_TYPE
}RawSliceKind;

enum {
    // 32-bits integer
    RTT_INT32,
    // void
    RTT_UNIT,
    // Array
    RTT_ARRAY,
    // pointer
    RTT_POINTER,
    // function
    RTT_FUNCTION
}RawTypeTag;

#endif