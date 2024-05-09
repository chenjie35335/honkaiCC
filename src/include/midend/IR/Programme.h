#ifndef STORMY_PROGRAMME 
#define STORMY_PROGRAMME
#include "Utility.h"
/// @brief programme
class RawProgramme{
    public:
    /// global values
    RawSlice Value;
    /// global functions
    RawSlice Funcs;
};
/// @brief create RawProgramme structure
/// @param Programme 
void createRawProgramme(RawProgramme *&Programme);
#endif