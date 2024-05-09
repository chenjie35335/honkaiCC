#ifndef STORMY_FUNCTION
#define STORMY_FUNCTION
#include "Type.h"
#include "Utility.h"
#include "../SSA/DT.h"
#include <vector>
#include <unordered_map>
using namespace std;
/// @brief type
class RawFunction{
    public:
    /// type
    RawType* ty;
    /// name of function
    const char *name;
    /// parameter(not used until now)
    RawSlice params;
    /// basic blocks
    RawSlice bbs;
    /// value set
    unordered_set<RawValue *> values;
};
typedef const RawFunction * RawFunctionP;
/// @brief generate rawfunction data structure
/// @param function 
/// @param name 
/// @param type 
void generateRawFunction(RawFunction *&function, const char *name,int type);

#endif