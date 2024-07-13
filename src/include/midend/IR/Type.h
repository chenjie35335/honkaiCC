#ifndef STORMY_TYPE
#define STORMY_TYPE
#include "Utility.h"
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include "TypeKind.h"
using namespace std;
class RawType{
    public:
    /// 类别标志
    uint32_t tag;
    /// 数组数据
    ArrayType array;
    /// 指针数据
    PointerType pointer;
    /// 函数数据
    FunctionType function;
    /// 构造函数
    RawType () {

    }
};   
typedef const RawType * RawTypeP; 
#endif