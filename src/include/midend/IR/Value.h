#ifndef STORMY_VALUE
#define STORMY_VALUE

#include<string>
#include<vector>
#include<stack>
#include<list>
#include <cstring>
#include"Function.h"
#include"ValueKind.h"
#include"Type.h"
using namespace std;

enum {
    /// ground
        BOT,
    /// middle
        VAL,
    /// up
        TOP
};

enum {
        IDENT_VAR,
        IDENT_ARR,
        IDENT_POINTER,
        IDENT_UNIT
};

//这里应该还需要一个defvalues和usevalues
class RawValue {
    public:
    /// @brief name of value
    const char * name;
    /// @brief kind of value
    ValueKind value;
    /// @brief type of value
    RawTypeP ty;
    /// @brief basicblock which define the value
    vector<RawBasicBlock *> defbbs;
    ///
    vector<RawBasicBlock *> usebbs;
    /// value rename stack(no matter global or local)
    stack<RawValue *> tempCopy;
    /// 定值点
    list<RawValue *> defPoints;
    /// 使用点
    list<RawValue *> usePoints;
    /// copies
    list<RawValue *> copiesValues;
    /// Deleted
    bool isDeleted;
    /// half grid (关于取值直接使用hash表)
    uint32_t status;
    /// identStatus
    uint32_t identType;
    /// count of copy
    uint32_t copyCount;


    RawValue * addr;

    RawValue * offset;

    RawValue() {
        this->isDeleted = false;
        this->status = BOT;
        this->identType = IDENT_UNIT;
        this->copyCount = 0;
    }
    RawValue(RawValue*oldvalue){
        this->isDeleted = false;
        this->status = BOT;
        this->identType = IDENT_UNIT;
        this->copyCount = 0;
        
        if(!!oldvalue->name){
            size_t bufSize = strlen(oldvalue->name);
            this->name = new char[bufSize];
            strcpy((char *)this->name, oldvalue->name);
            // snprintf((char *)this->name, bufSize, "%s", oldvalue->name);
        }
        this->value = new ValueKind(oldvalue->value);
        this->ty = oldvalue->ty;
    }
};
// addr = new RawValue()
// addr->value.tag = RVT_ALLOC

// offset = new RawValue()
// addr->value.tag = RVT_INTEGER
typedef const RawValue * RawValueP;

void generateRawValueArgs(const string &ident,int index, int32_t flag);

void getMidVarValue(RawValueP &value, string &name);

void getVarValueL(RawValueP &value,string &name);

void generateRawValue(RawFunctionP callee,vector<RawValueP> paramsValue,string &sign);

void generateRawValue(string &sign, RawValueP lhs, RawValueP rhs, uint32_t op);

void generateRawValue(RawValueP src);

void generateRawValue(int32_t number);

void generateRawValue(float number);

RawValue * generateNumber(int32_t number);

RawValue * generateFloat(float number);

RawValueP generateZero();

void generateRawValue(RawValueP &src, RawValueP &dest);

void generateRawValue(vector<RawValueP>elements,string &sign);

void generateRawValue(string& name,int32_t flag); //override

void generateRawValue(string &sign, RawValueP &src);

void generateRawValue(RawValueP &cond, RawBasicBlock* &Truebb, RawBasicBlock* &Falsebb);

void generateRawValue(RawBasicBlock* &TargetBB);

void generateRawValueGlobal(const char *name,int num);

void generateRawValueGlobal(const char *name,float num);

void generateRawValueArr(string &name,vector<int> &dimen, int32_t flag); //override RTT_FLOAT

void fillZero(RawValueP &rawSrc,RawValueP &src,vector<int> &dimen);

void generateRawValueArr(int32_t number);

void generateRawValueArr(float number);

void generateRawValueArrGlobal(string &name,vector<int> &dimen,RawValue *&init, int32_t flag); //override of float

void generateZeroInit(RawValue *&zeroinit);

void generateElement(RawValueP &src,RawValueP &index,string &name);

void generatePtr(RawValueP &src,RawValueP &index,string &name);

void generateRawValueSinArr(const string &ident,int index, int flag);

void generateRawValueMulArr(const string &ident,int index,vector<int>dimens,int flag);

void MarkUse(RawValue *src,RawValue *target);

void MarkDef(RawValue *src,RawValue *target);

void ArrInit(RawValueP src,RawValueP target);

void generateConvert(RawValueP &src,string &name);

void generateRawValuePointer(string &name,RawType *ty);

#endif