#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <cstring>
#include <unordered_map>
extern SignTable signTable;

void GenerateGetInt() {
    RawFunction *getint;
    string name = "getint";
    generateRawFunction(getint,name.c_str(),FUNCTYPE_INT);
    signTable.insertFunction(name,getint);
}

void GenerateGetch() {
    RawFunction *getch;
    string name = "getch";
    generateRawFunction(getch,name.c_str(),FUNCTYPE_INT);
    signTable.insertFunction(name,getch);
}

void GenerateStartTime() {
    RawFunction *starttime;
    string name = "starttime";
    generateRawFunction(starttime,name.c_str(),FUNCTYPE_VOID);
    signTable.insertFunction(name,starttime);
}

void GenerateStopTime() {
    RawFunction *stoptime;
    string name = "stoptime";
    generateRawFunction(stoptime,name.c_str(),FUNCTYPE_VOID);
    signTable.insertFunction(name,stoptime);
}
//对于这种带参数的需要如何处理？由于没有函数体，所以params那里不需要初始化，但是type那里需要
void GeneratePutInt() {
    RawFunction *putint;
    string name = "putint";
    generateRawFunction(putint,name.c_str(),FUNCTYPE_VOID);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = putint->ty->function.params;
    RawType *IntType = new RawType();
    IntType->tag = RTT_INT32;
    params.push_back(IntType) ;
    signTable.insertFunction(name,putint);
}

void GeneratePutch() {
    RawFunction *putch;
    string name = "putch";
    generateRawFunction(putch,name.c_str(),FUNCTYPE_VOID);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = putch->ty->function.params;
    RawType *IntType = new RawType();
    IntType->tag = RTT_INT32;
    params.push_back(IntType);
    signTable.insertFunction(name,putch);
}

void GeneratePutArray() {
    RawFunction *putarray;
    string name = "putarray";
    generateRawFunction(putarray,name.c_str(),FUNCTYPE_VOID);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = putarray->ty->function.params;
    RawType *IntType = new RawType();
    IntType->tag = RTT_INT32;
    params.push_back(IntType);
    RawType *PointerType = new RawType();
    PointerType->tag = RTT_POINTER;
    RawType *PointeeType = new RawType();
    PointeeType->tag = RTT_INT32;
    PointerType->pointer.base = PointeeType;
    params.push_back(PointerType);
    signTable.insertFunction(name,putarray);
}

void GenerateGetArray() {
    RawFunction *getarray;
    string name = "getarray";
    generateRawFunction(getarray,name.c_str(),FUNCTYPE_INT);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = getarray->ty->function.params;
    RawType *PointerType = new RawType();
    PointerType->tag = RTT_POINTER;
    RawType *PointeeType = new RawType();
    PointeeType->tag = RTT_INT32;
    PointerType->pointer.base = PointeeType;
    params.push_back(PointerType);
    signTable.insertFunction(name,getarray);
}

void GeneratePutFloat(){
    RawFunction *putfloat;
    string name = "putfloat";
    generateRawFunction(putfloat,name.c_str(),FUNCTYPE_VOID);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = putfloat->ty->function.params;
    RawType *FloatType = new RawType();
    FloatType->tag = RTT_FLOAT;
    params.push_back(FloatType) ;
    signTable.insertFunction(name,putfloat);
}

void GenerateGetFloat(){
    RawFunction *getfloat;
    string name = "getfloat";
    generateRawFunction(getfloat,name.c_str(),FUNCTYPE_FLOAT);
    signTable.insertFunction(name,getfloat);
}

void GenerateGetFArray(){
    RawFunction *getfarray;
    string name = "getfarray";
    generateRawFunction(getfarray,name.c_str(),FUNCTYPE_FLOAT);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = getfarray->ty->function.params;
    RawType *PointerType = new RawType();
    PointerType->tag = RTT_POINTER;
    RawType *PointeeType = new RawType();
    PointeeType->tag = RTT_FLOAT;
    PointerType->pointer.base = PointeeType;
    params.push_back(PointerType);
    signTable.insertFunction(name,getfarray);
}

void GeneratePutFArray(){
    RawFunction *putfarray;
    string name = "putfarray";
    generateRawFunction(putfarray,name.c_str(),FUNCTYPE_VOID);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = putfarray->ty->function.params;
    RawType *FloatType = new RawType();
    FloatType->tag = RTT_FLOAT;
    params.push_back(FloatType);
    RawType *PointerType = new RawType();
    PointerType->tag = RTT_POINTER;
    RawType *PointeeType = new RawType();
    PointeeType->tag = RTT_FLOAT;
    PointerType->pointer.base = PointeeType;
    params.push_back(PointerType);
    signTable.insertFunction(name,putfarray);
}

