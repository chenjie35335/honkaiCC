#ifndef STORMY_FUNCTION
#define STORMY_FUNCTION
#include "Type.h"
#include "Utility.h"
#include "../SSA/DT.h"
#include <vector>
#include <unordered_map>
#include <list>
class AsmInst;
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
    /// handle the stacklen
    vector<AsmInst *> handleStack;
    /// handle the save registers
    list<AsmInst *> saveRegister;
    /// load the save register
    list<AsmInst *> loadRegister;
    /// handle the stacklen
    list<AsmInst *> exitStack;

    RawFunction() {}
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