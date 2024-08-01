#ifndef CONDITIONCCP_STORMY//这里使用条件常数传播
#define CONDITIONCCP_STORMY
#include <unordered_set>
#include "../IR/common.h"
#include "../IR/Value.h"
#include <unordered_map>
using namespace std;

class CondCCPTabl{
    public:
        unordered_map<RawValue *,int> MidIntTable;

    int LookValue(RawValue *data) { 
        if(data->value.tag == RVT_INTEGER) return data->value.integer.value;
        else if(data->value.tag == RVT_FLOAT) return data->value.floatNumber.value;
        return this->MidIntTable[data];
    }

    RawValue * generateNumber(int value) {
    RawValue *data = new RawValue();
    data->name = nullptr;
    data->value.tag = RVT_INTEGER;
    data->value.integer.value = value;
    RawType *ty = new RawType();
    ty->tag = RTT_INT32;
    data->ty = ty;
    return data;
}
};

void OptimizeSCCP(RawProgramme *&programme);
#endif