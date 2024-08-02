#ifndef BLOCKELIMINATE_STORMY//这里使用条件常数传播
#define BLOCKELIMINATE_STORMY
#include <unordered_set>
#include "../IR/common.h"
#include "../IR/Value.h"
#include <unordered_map>
using namespace std;

void BlockEliminateRun(RawProgramme *programme);
#endif