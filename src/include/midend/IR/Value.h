#ifndef STORMY_VALUE
#define STORMY_VALUE

#include<string>
#include<vector>
#include<stack>
#include"Function.h"
#include"ValueKind.h"
#include"Type.h"
using namespace std;


class RawValue {
    public:
    /// @brief name of value
    const char * name;
    /// @brief kind of value
    ValueKind value;
    /// @brief type of value
    RawTypeP ty;
    /// @brief basicblock which define the value
    unordered_set<RawBasicBlock *> defbbs;
    /// value rename stack(no matter global or local)
    RawValue * tempCopy;
};

typedef const RawValue * RawValueP;

void generateRawValueArgs(const string &ident,int index);

void getMidVarValue(RawValueP &value, string &name);

void getVarValueL(RawValueP &value,string &name);

void generateRawValue(RawFunctionP callee,vector<RawValueP> paramsValue,string &sign);

void generateRawValue(string &sign, RawValueP lhs, RawValueP rhs, uint32_t op);

void generateRawValue(RawValueP src);

void generateRawValue(int32_t number);

void generateRawValue(RawValueP &src, RawValueP &dest);

void generateRawValue(vector<RawValueP>elements,string &sign);

void generateRawValue(string& name);

void generateRawValue(string &sign, RawValueP &src);

void generateRawValue(RawValueP &cond, RawBasicBlock* &Truebb, RawBasicBlock* &Falsebb);

void generateRawValue(RawBasicBlock* &TargetBB);

void generateRawValueGlobal(const char *name,int num);

void generateRawValueArr(string &name,vector<int> &dimen);

void fillZero(RawValueP &rawSrc,RawValueP &src,vector<int> &dimen);

void generateRawValueArr(int32_t number);

void generateRawValueArrGlobal(const char *name,vector<int> &dimen,RawValue *&init);

void generateZeroInit(RawValue *&zeroinit);

void generateElement(RawValueP &src,RawValueP &index,string &name);

void generateRawValueSinArr(const string &ident,int index);

void generateRawValueMulArr(const string &ident,int index,vector<int>dimens);

#endif