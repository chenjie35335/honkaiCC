#ifndef STORMY_GENERATOR_T
#define STORMY_GENERATOR_T
#include <string>
#include "../../midend/IR/IRGraph.h"
/// @brief Visit RawValue
/// @param value 
void Visit(const RawValueP &value);
/// @brief Visit RawBasicBlock
/// @param bb 
void Visit(const RawBasicBlockP &bb);
/// @brief Visit Function
/// @param func 
void Visit(const RawFunctionP &func);
/// @brief Visit Programme and Generate ASM
/// @param value 
void generateASM(RawProgramme *& value);
///

#endif