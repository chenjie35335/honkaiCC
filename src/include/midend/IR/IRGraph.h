#ifndef STORMY_IRGRAPH_DEF
#define STORMY_IRGRAPH_DEF
#include <vector>
#include <cstdint>
using namespace std;

/// A raw slice that can store any kind of items.
typedef struct RawSlice {
    /// Buffer of slice item
    const void ** buffer;   
    /// The length of array buffer
    uint32_t len;
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



#endif