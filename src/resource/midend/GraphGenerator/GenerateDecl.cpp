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
        string MidIdent =  "@" + ident + "_" + to_string(dep); 
        generateRawValue(MidIdent,IR);
        break;
    }
    case SINVARDEFAST_INI:
    {
        string MidIdent =  "@" + ident + "_" + to_string(dep); 
        generateRawValue(MidIdent,IR);
        InitVal->generateGraph(IR,sign1);
        RawValueP dest,src;
        generateRawValue(src,sign1);
        generateRawValue(dest,MidIdent);
        generateRawValue(src,dest,IR);
        break;
        //store类型的变量值没有返回值，不会被使用，所以不用存中间变量表
    }
    default:
        assert(0);
    }
    IdentTable->addVariable(ident, value);
}

void InitValAST::generateGraph(RawSlice &IR,string &sign) const{
    Exp->generateGraph(IR,sign);
}