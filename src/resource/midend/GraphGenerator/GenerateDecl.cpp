#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern unordered_map<string, RawValueP> MidVarTable;

void DeclAST::generateGraph() const
{
    switch (type)
    {
    case DECLAST_CON:
        ConstDecl->Dump();
        break;
    case DECLAST_VAR:
        VarDecl->generateGraph();
        break;
    default:
        assert(0);
    }
}

void VarDeclAST::generateGraph() const
{
    MulVarDef->generateGraph();
}

void MulVarDefAST::generateGraph() const
{
    for (auto &sinValDef : SinValDef)
    {
        sinValDef->generateGraph();
    }
}
// 对于分配的变量来说，在遍历时候需要使用@ident_dep,但是identTable内需要用ident
void SinVarDefAST::generateGraph() const
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
        generateRawValue(MidIdent);
        break;
    }
    case SINVARDEFAST_INI:
    {
        string MidIdent =  "@" + ident + "_" + to_string(dep);
        //cout << MidIdent << endl; 
        generateRawValue(MidIdent);
        InitVal->generateGraph(sign1);
        RawValueP dest,src;
        generateRawValue(src,sign1);
        generateRawValue(dest,MidIdent);
        generateRawValue(src,dest);
        break;
        //store类型的变量值没有返回值，不会被使用，所以不用存中间变量表
    }
    default:
        assert(0);
    }
    IdentTable->addVariable(ident, value);
}

void InitValAST::generateGraph(string &sign) const{
    Exp->generateGraph(sign);
}