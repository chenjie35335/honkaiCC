#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern SignTable signTable;
void DeclAST::generateGraph() const
{
    switch (type)
    {
    case DECLAST_CON:
        ConstDecl->generateGraph();
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
    for (auto &sinValDef : SinValDef){
        sinValDef->generateGraph();
    }
}
// 对于分配的变量来说，在遍历时候需要使用@ident_dep,但是identTable内需要用ident
void SinVarDefAST::generateGraph() const
{
    string DestSign = ident;
    signTable.varMultDef(ident);
    switch(type) {
        case SINVARDEFAST_UIN: {
            generateRawValue(DestSign);
            break;
        }
        case SINVARDEFAST_INI: {
            string SrcSign;
            generateRawValue(DestSign);
            RawValueP dest,src;
            InitVal->generateGraph(SrcSign);
            getMidVarValue(src,SrcSign);
            getVarValueL(dest,DestSign);
            generateRawValue(src,dest);
            break;
        }
        default:
        assert(0);
    }
}

void InitValAST::generateGraph(string &sign) const{
    Exp->generateGraph(sign);
}

void ConstDeclAST::generateGraph() const {
   MulConstDef->generateGraph();
}

void SinConstDefAST::generateGraph() const{
  signTable.constMulDef(ident);
  int value = ConstExp->calc();
  signTable.insertConst(ident,value);
}

