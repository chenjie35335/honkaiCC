#ifndef STORMY_LCSE
#define STORMY_LCSE
#include <unordered_set>
#include "../IR/common.h"
#include "../IR/Value.h"
#include <unordered_map>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <string>
using namespace std;

class LCSE_Builder{
    public:
        unordered_set<RawValue *> Expressions;

    LCSE_Builder(){}
};

void OptimizeLCSE(RawProgramme *programme);
#endif