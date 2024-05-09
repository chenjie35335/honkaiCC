#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
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
    case DECLAST_ARR:
        //VarDecl->generateGraph();
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

void VarDeclAST::generateGraphGlobal() const{
    MulVarDef->generateGraphGlobal();
}

void MulVarDefAST::generateGraphGlobal() const
{
    for (auto &sinValDef : SinValDef){
        sinValDef->generateGraphGlobal();
    }
}

void SinVarDefAST::generateGraphGlobal() const {
    string DestSign = ident;
    signTable.varMultDef(ident);
    switch(type) {
    case SINVARDEFAST_UIN: {
        generateRawValueGlobal(DestSign.c_str(),0);
        break;
    }
    case SINVARDEFAST_INI: {//这里确实进行计算，但是和之前一样，如果不是常量就未定义
        int value = InitVal->calc();
        generateRawValueGlobal(DestSign.c_str(),value);
        break;
    }
    case SINVARDEFAST_INI_ARR: {
        string SrcSign;
        vector<int> dimens;
        dimen->generateGraph(dimens);
        constInit->generateGraph(SrcSign);
        RawValueP rawSrc;
        getMidVarValue(rawSrc,SrcSign);
        RawValueP src;
        fillZero(rawSrc,src,dimens);
        RawValue *init = (RawValue *) src;
        generateRawValueArrGlobal(DestSign.c_str(),dimens,init);
        break;
    }
    case SINVARDEFAST_UNI_ARR: {
        vector<int> dimens;
        dimen->generateGraph(dimens);
        RawValue *zeroinit;
        generateZeroInit(zeroinit);
        generateRawValueArrGlobal(DestSign.c_str(),dimens,zeroinit);
        break;
    }
    default:
    assert(0);
}
}//这里我会考虑直接计算出来，存在某处
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
        case SINVARDEFAST_INI_ARR: {
            string SrcSign;
            vector<int> dimens;
            dimen->generateGraph(dimens);
            generateRawValueArr(DestSign,dimens);
            constInit->generateGraph(SrcSign);
            RawValueP dest,rawSrc;
            getMidVarValue(rawSrc,SrcSign);
            RawValueP src;
            fillZero(rawSrc,src,dimens);
            getVarValueL(dest,DestSign);
            generateRawValue(src,dest);
            break;
        }//首先常量数组不需要额外弄一个alloc节点，感觉只需要给一个
        case SINVARDEFAST_UNI_ARR: {
            vector<int> dimens;
            dimen->generateGraph(dimens);
            generateRawValueArr(DestSign,dimens);
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
    switch(type) {
        case SINCONST_VAR: {
            int value = constExp->calc();
            signTable.insertConst(ident,value);//这个地方即使不做全局貌似也没事
            break;
        }
        case SINCONST_ARRAY: {//这里有个非常恶心的地方是该如何存的问题
        //我的感觉是这里还是需要按照之前的方式存储，但是读的时候就需要另外处理了
        string SrcSign,DestSign;
        DestSign = ident;
        vector<int> dimens;
        arrayDimen->generateGraph(dimens);
        generateRawValueArr(DestSign,dimens);
        constArrayInit->generateGraph(SrcSign);
        RawValueP dest,rawSrc;
        getMidVarValue(rawSrc,SrcSign);
        RawValueP src;
        fillZero(rawSrc,src,dimens);
        getVarValueL(dest,DestSign);
        generateRawValue(src,dest);
        break;
        }//这里const和其他的是有一定的区别的,这个存起来实在太恶心了，还是不判断了，直接也当成变量对待吧
}   
}

void ConstDeclAST::generateGraphGlobal() const {
    MulConstDef->generateGraphGlobal();
}
 
void MulConstDefAST::generateGraphGlobal() const {
    for(auto &sinConstDef : SinConstDef) {
        sinConstDef->generateGraphGlobal();
    }
}

void SinConstDefAST::generateGraphGlobal() const {
    string DestSign = ident;
    signTable.constMulDef(ident);
    switch(type) {
        case SINCONST_VAR: {
            int value = constExp->calc();
            signTable.insertConst(ident,value);//这个地方即使不做全局貌似也没事
            break;
        }
        case SINCONST_ARRAY: {//这里有个非常恶心的地方是该如何存的问题
        //我的感觉是这里还是需要按照之前的方式存储，但是读的时候就需要另外处理了
            string SrcSign;
            vector<int> dimens;
            arrayDimen->generateGraph(dimens);
            constArrayInit->generateGraph(SrcSign);
            RawValueP dest,rawSrc;
            getMidVarValue(rawSrc,SrcSign);
            RawValueP src;
            fillZero(rawSrc,src,dimens);
            RawValue *init = (RawValue *) src;
            generateRawValueArrGlobal(DestSign.c_str(),dimens,init);
            break;
        }//这里const和其他的是有一定的区别的
    }   
}

