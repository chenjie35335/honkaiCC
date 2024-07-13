#ifndef TYPEKIND_STORMY
#define TYPEKIND_STORMY
#include <cstdlib>
#include <vector>
using namespace std;
class RawType;

class ArrayType{
    public:
        const  RawType *base;
        size_t len;
};

class PointerType{
    public:
        const  RawType *base;   
};

class FunctionType{
    public:
        vector <RawType *> params;//存储参量的类型
        const  RawType *ret;//存储返回值的类型
};

#endif