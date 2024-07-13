#ifndef ExitSSA_STORMY
#define ExitSSA_STORMY
#include <unordered_set>
#include "../IR/common.h"
void exitSSA(const RawBasicBlockP &bb);

void exitSSA(const RawFunctionP &func);

void exitSSA(RawValueP &value);

/// @brief 退出SSA形式
/// @param programme 
void exitSSA(RawProgramme *&programme);
#endif