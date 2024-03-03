#ifndef STORMY_GENERATOR_T
#define STORMY_GENERATOR_T
#include <string>
#include "../../midend/IR/IRGraph.h"
/// @brief Visit RawSlice Node
/// @param slice 
/// @param sign 
void Visit(const  RawSlice& slice,string &sign);

void Visit(const RawValue* &value,string &sign);
#endif