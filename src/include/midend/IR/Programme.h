#ifndef STORMY_PROGRAMME 
#define STORMY_PROGRAMME
#include "Utility.h"
#include "Value.h"
#include "Function.h"
#include <list>
using namespace std;
/// @brief programme
class RawProgramme{
    public:
    /// list of global values
    list<RawValue *> values;
    /// list of functions
    list<RawFunction *> funcs;
};
/// @brief create RawProgramme structure
/// @param Programme 
void createRawProgramme(RawProgramme *&Programme);

#endif