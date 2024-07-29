#ifndef MEM2REG_STORMY//这里使用条件常数传播
#define MEM2REG_STORMY
#include <unordered_set>
#include "../IR/common.h"
#include "../IR/Value.h"
#include <unordered_map>
using namespace std;

void OptimizeMem2Reg(RawProgramme *&programme);
#endif
