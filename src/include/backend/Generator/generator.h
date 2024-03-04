#ifndef STORMY_GENERATOR_T
#define STORMY_GENERATOR_T
#include <string>
#include "../../midend/IR/IRGraph.h"
/// @brief Visit RawSlice Node
/// @param slice 
/// @param sign 
void Visit(const  RawSlice& slice,string &sign);
/// @brief Visit RawValue Node
/// @param value 
/// @param sign 
void Visit(const RawValue* &value,string &sign);
/// @brief Visit RawBasicBlock Node
/// @param value 
/// @param sign 
void Visit(const RawBasicBlock* &value,string &sign);
/// @brief Visit RawInteger
/// @param integer 
/// @param sign 
void Visit(const RawInteger &integer,string &sign);
/// @brief Visit RawReturn
/// @param ret 
/// @param sign 
void Visit(const RawReturn &ret,string &sign);
/// @brief Visit RawStore
/// @param store 
/// @param sign 
void Visit(const RawStore &store, string &sign);
/// @brief Visit RawLoad
/// @param load 
/// @param sign 
void Visit(const RawLoad &load,string &sign);
/// @brief Visit Binary
/// @param binary 
/// @param sign 
void Visit(const RawBinary &binary, string &sign);
#endif