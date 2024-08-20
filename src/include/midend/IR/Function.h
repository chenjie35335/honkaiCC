#ifndef STORMY_FUNCTION
#define STORMY_FUNCTION
#include "Type.h"
#include "Utility.h"
#include "../SSA/DT.h"
#include <vector>
#include <unordered_map>
#include <list>
using namespace std;
/// @brief type
class RawFunction{
    public:
    /// type
    RawType* ty;
    /// name of function
    const char *name;
    /// params
    vector <RawValue *> params;
    /// basicblock list
    list <RawBasicBlock *> basicblock;
    /// value set
    unordered_set<RawValue *> values;
    /// 需要用到的int型变量的数量
    int IntNumber;
    /// 需要使用的float型变量的数量
    int floatNumber;
};
typedef const RawFunction * RawFunctionP;
/// @brief generate rawfunction data structure
/// @param function 
/// @param name 
/// @param type 
void generateRawFunction(RawFunction *&function, const char *name,int type);
/// @brief insert function into programme
/// @param function 
void PushRawFunction(RawFunction *&function);

#endif
//考虑一个问题：就是我如何判断我需要分配的数量？
