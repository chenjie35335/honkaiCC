#ifndef CONSTCOMBINED_STORMY
#define CONSTCOMBINED_STORMY
#include <unordered_set>
#include <unordered_map>
#include "../IR/Value.h"
using namespace std;
/*
    对于load来说，该如何处理？
    首先，load
*/
class Combine{
    public:
        /// @brief 整数值
        unordered_map<RawValue *,int> IntValue;
        /// @brief 浮点数值
        unordered_map<RawValue *,double> DoubleValue;
        /// 添加值
        void AddIntValue(RawValue *data,int value) { this->IntValue.insert(pair<RawValue *,int>(data,value));}
        /// 删除值
        void DeleteIntValue(RawValue *data) { this->IntValue.erase(data);}
        /// 查询值
        int LookValue(RawValue *data) { return this->IntValue[data];}
        /// 生成Ingeter型value
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
        bool find(RawValue *data) { return this->IntValue.find(data) != this->IntValue.end();}
};

void ConstCombine(RawProgramme *&programme);

#endif









