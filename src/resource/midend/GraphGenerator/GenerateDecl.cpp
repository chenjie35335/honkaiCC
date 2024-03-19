#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern unordered_map<string, RawValueP> MidVarTable;

void DeclAST::generateGraph(RawSlice &IR) const
{
    switch (type)
    {
    case DECLAST_CON:
        ConstDecl->Dump();
        break;
    case DECLAST_VAR:
        VarDecl->generateGraph(IR);
        break;
    default:
        assert(0);
    }
}

void VarDeclAST::generateGraph(RawSlice &IR) const
{
    MulVarDef->generateGraph(IR);
}

void MulVarDefAST::generateGraph(RawSlice &IR) const
{
    for (auto &sinValDef : SinValDef)
    {
        sinValDef->generateGraph(IR);
    }
}
// 对于分配的变量来说，在遍历时候需要使用@ident_dep,但是identTable内需要用ident
void SinVarDefAST::generateGraph(RawSlice &IR) const
{
    int value = 0;
    int dep = IdentTable->level;
    IdentTable->VarContrdef(ident);
    IdentTable->VarMultDef(ident);
    string sign1;
    switch (type)
    {
    case SINVARDEFAST_UIN:
    {
        RawValue *alloc = (RawValue *)malloc(sizeof(RawValue));
        alloc->value.tag = RVT_ALLOC;
        IR.buffer[IR.len++] = (const void *)alloc;
        MidVarTable.insert(pair<string, RawValueP>("@" + ident + "_" + to_string(dep), alloc));
        break;
    }
    case SINVARDEFAST_INI:
    {
        RawValue *alloc = (RawValue *)malloc(sizeof(RawValue));
        alloc->value.tag = RVT_ALLOC;
        IR.buffer[IR.len++] = (const void *)alloc;
        MidVarTable.insert(pair<string, RawValueP>("@" + ident + "_" + to_string(dep), alloc));
        InitVal->generateGraph(IR,sign1);
        RawValueP src;
        generateRawValue(src,sign1);
        RawValue *store = (RawValue *) malloc(sizeof(RawValue));
        store->name = nullptr;
        store->value.tag = RVT_STORE;
        store->value.data.store.value = src;
        store->value.data.store.dest = alloc;
        IR.buffer[IR.len++] = (const void *) store;
        break;
        //store类型的变量值没有返回值，不会被使用，所以不用存中间变量表
    }
    default:
        assert(0);
    }
    IdentTable->addVariable(ident, value);
}

void InitValAST::generateGraph(RawSlice &IR,string &sign) const{
    //cout << "enter InitValAST" << endl;
    Exp->generateGraph(IR,sign);
}