#ifndef STORMY_TYPE
#define STORMY_TYPE
#include "Utility.h"
#include <stdlib.h>
#include <stdint.h>
class RawType{
    public:
    uint32_t tag;
    union {
        struct {
            const  RawType *base;
            size_t len;
        }array;
        struct {
            const  RawType *base;
        }pointer;
        struct {
            RawSlice params;//存储参量的类型
            const  RawType *ret;//存储返回值的类型
        }function;
    }data;
};   
typedef const RawType * RawTypeP; 
#endif