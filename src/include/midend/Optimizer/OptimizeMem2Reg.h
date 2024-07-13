#ifndef STORMY_MEM2REG
#define STORMY_MEM2REG

#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "../IR/Programme.h"
#include "../IR/BasicBlock.h"

class mem2regBuilder {
    public:
        unordered_map <RawValue* , RawValue *> IncomingVals;

    void insert(RawValue * mem,RawValue *reg) { IncomingVals.insert(pair<RawValue *,RawValue *>(mem,reg));}
    RawValue * lookup(RawValue *mem);
};

void mem2regTop(RawProgramme *programme);

#endif
