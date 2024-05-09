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
    auto &params = putint->ty->data.function.params;
    RawType *IntType = new RawType();
    IntType->tag = RTT_INT32;
    params.buffer[params.len++] = (const void *)IntType;
    signTable.insertFunction(name,putint);
}

void GeneratePutch() {
    RawFunction *putch;
    string name = "putch";
    generateRawFunction(putch,name.c_str(),FUNCTYPE_VOID);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = putch->ty->data.function.params;
    RawType *IntType = new RawType();
    IntType->tag = RTT_INT32;
    params.buffer[params.len++] = (const void *)IntType;
    signTable.insertFunction(name,putch);
}

void GeneratePutArray() {
    RawFunction *putarray;
    string name = "putarray";
    generateRawFunction(putarray,name.c_str(),FUNCTYPE_VOID);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = putarray->ty->data.function.params;
    RawType *IntType = new RawType();
    IntType->tag = RTT_INT32;
    params.buffer[params.len++] = (const void *)IntType;
    RawType *PointerType = new RawType();
    PointerType->tag = RTT_POINTER;
    RawType *PointeeType = new RawType();
    PointeeType->tag = RTT_INT32;
    PointerType->data.pointer.base = PointeeType;
    params.buffer[params.len++] = (const void *)PointerType;
    signTable.insertFunction(name,putarray);
}

void GenerateGetArray() {
    RawFunction *getarray;
    string name = "getarray";
    generateRawFunction(getarray,name.c_str(),FUNCTYPE_INT);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    auto &params = getarray->ty->data.function.params;
    RawType *PointerType = new RawType();
    PointerType->tag = RTT_POINTER;
    RawType *PointeeType = new RawType();
    PointeeType->tag = RTT_INT32;
    PointerType->data.pointer.base = PointeeType;
    params.buffer[params.len++] = (const void *)PointerType;
    signTable.insertFunction(name,getarray);
}

