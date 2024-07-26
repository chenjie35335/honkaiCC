#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern SignTable signTable;

//常量的定义的初始化不需要管，只有变量的初始化要压入表

void DeclAST::generateGraph() const
{
    //btype;
    switch (type)
    {
    case DECLAST_CON:
        ConstDecl->generateGraph();
        break;
    case DECLAST_VAR:{
        int32_t BTYPE = btype->getType();
        // cout << "BTYPE:" << BTYPE << endl;
        VarDecl->generateGraph(BTYPE);
        break;
    }
    default:
        assert(0);
    }
}

void VarDeclAST::generateGraph(int &retType) const
{
    MulVarDef->generateGraph(retType);
}

void MulVarDefAST::generateGraph(int &retType) const
{
    for (auto &sinValDef : SinValDef){
        sinValDef->generateGraph(retType);
    }
}

void VarDeclAST::generateGraphGlobal(int &retType) const{
    MulVarDef->generateGraphGlobal(retType);
}

void MulVarDefAST::generateGraphGlobal(int &retType) const
{
    for (auto &sinValDef : SinValDef){
        sinValDef->generateGraphGlobal(retType);
    }
}


void SinVarDefAST::generateGraphGlobal(int &retType) const {
    string DestSign = ident;
    signTable.varMultDef(ident);
    switch(type) {
    case SINVARDEFAST_UIN: {
        generateRawValueGlobal(DestSign.c_str(),0);
        break;
    }
    case SINVARDEFAST_INI: {
        //这里确实进行计算，但是和之前一样，如果不是常量就未定义
        auto value = InitVal->Calculate();
         if(retType == RTT_FLOAT) {
            if(value->type == ExpResult::INT) 
                generateRawValueGlobal(DestSign.c_str(),(float)value->IntResult);
            else
                generateRawValueGlobal(DestSign.c_str(),value->FloatResult);
        } else {
            if(value->type == ExpResult::INT) 
                generateRawValueGlobal(DestSign.c_str(),value->IntResult);
            else
                generateRawValueGlobal(DestSign.c_str(),(int)value->FloatResult);
        }
        break;
    }
    case SINVARDEFAST_INI_ARR: {
        //IDENT ArrayDimen '=' ConstArrayInit
        string SrcSign;
        vector<int> dimens;
        dimen->generateGraph(dimens);
        constInit->generateGraphGlobal(SrcSign, retType);
        RawValueP rawSrc;
        getMidVarValue(rawSrc,SrcSign);
        RawValueP src;
        fillZero(rawSrc,src,dimens);
        RawValue *init = (RawValue *) src;
        //cerr << "retType: " << retType << endl;
        generateRawValueArrGlobal(DestSign,dimens,init, retType);
        break;
    }
    case SINVARDEFAST_UNI_ARR: {
        vector<int> dimens;
        dimen->generateGraph(dimens);
        RawValue *zeroinit;
        generateZeroInit(zeroinit);
        generateRawValueArrGlobal(DestSign,dimens,zeroinit,retType);
        break;
    }
    default:
    assert(0);
}
}//这里我会考虑直接计算出来，存在某处
// 对于分配的变量来说，在遍历时候需要使用@ident_dep,但是identTable内需要用ident
void SinVarDefAST::generateGraph(int &retType) const
{
    string DestSign = ident;
    //检查下ident到底是什么类型
    signTable.varMultDef(ident);
    switch(type) {
        case SINVARDEFAST_UIN: {
            generateRawValue(DestSign,retType);
            break;
        }
        case SINVARDEFAST_INI: {
            string SrcSign;
            RawValueP dest,src;
            generateRawValue(DestSign,retType);
            InitVal->generateGraph(SrcSign);
            getMidVarValue(src,SrcSign);
            getVarValueL(dest,DestSign);
            auto SrcTag = src->ty->tag;
            auto DestPtrTag = dest->ty->pointer.base->tag;
            if(SrcTag != DestPtrTag) generateConvert(src,SrcSign);
            src = signTable.getMidVar(SrcSign);
            generateRawValue(src,dest);
            break;
        }
        case SINVARDEFAST_INI_ARR: {
            string SrcSign;
            vector<int> dimens;
            dimen->generateGraph(dimens);
            generateRawValueArr(DestSign,dimens,retType);
            constInit->generateGraph(SrcSign,retType);
            RawValueP dest,rawSrc;
            getMidVarValue(rawSrc,SrcSign);
            RawValueP src;
            fillZero(rawSrc,src,dimens);
            getVarValueL(dest,DestSign);
            //generateRawValue(src,dest);
            ArrInit(src,dest);
            break;
        }//首先常量数组不需要额外弄一个alloc节点，感觉只需要给一个
        case SINVARDEFAST_UNI_ARR: {
            vector<int> dimens;
            dimen->generateGraph(dimens);
            generateRawValueArr(DestSign,dimens,retType);
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
   int retType = Btype->getType();
   MulConstDef->generateGraph(retType);
}

void SinConstDefAST::generateGraph(int &retType) const{
    // cout << ident;
    signTable.constMulDef(ident);
    switch(type) {
        case SINCONST_VAR: {
            auto value = constExp->Calculate();
        if(retType == RTT_FLOAT){
            if(value->type == ExpResult::INT) {
                // cout << "value: " << value->IntResult << endl;
                signTable.insertFconst(ident,value->IntResult);
            }
            else{
                // cout << "value: " << value->FloatResult << endl;
                signTable.insertFconst(ident,value->FloatResult);
            }
        } else {
            if(value->type == ExpResult::INT) 
                signTable.insertConst(ident,value->IntResult);
            else
                signTable.insertFconst(ident,value->FloatResult);
        }
            break;
        }
        case SINCONST_ARRAY: {//这里有个非常恶心的地方是该如何存的问题
        //我的感觉是这里还是需要按照之前的方式存储，但是读的时候就需要另外处理了
        string SrcSign,DestSign;
        DestSign = ident;
        vector<int> dimens;
        arrayDimen->generateGraph(dimens);
        generateRawValueArr(DestSign,dimens,retType);
        constArrayInit->generateGraphGlobal(SrcSign,retType);
        RawValueP dest,rawSrc;
        getMidVarValue(rawSrc,SrcSign);
        RawValueP src;
        fillZero(rawSrc,src,dimens);
        getVarValueL(dest,DestSign);
        //generateRawValue(src,dest);
        ArrInit(src,dest);
        break;
        }//这里const和其他的是有一定的区别的,这个存起来实在太恶心了，还是不判断了，直接也当成变量对待吧
}   
}

void ConstDeclAST::generateGraphGlobal() const {
    int retType = Btype->getType();
    MulConstDef->generateGraphGlobal(retType);
}
 
void MulConstDefAST::generateGraphGlobal(int &retType) const {
    for(auto &sinConstDef : SinConstDef) {
        sinConstDef->generateGraphGlobal(retType);
    }
}

void SinConstDefAST::generateGraphGlobal(int &retType) const {
    // cout << ident;
    // cout << "const def global" << endl;
    string DestSign = ident;
    signTable.constMulDef(ident);
    switch(type) {
        case SINCONST_VAR: { 
        auto value = constExp->Calculate();
        if(retType == RTT_FLOAT){
            if(value->type == ExpResult::INT) {
                // cout << " value: " << value->IntResult << endl;
                signTable.insertFconst(ident,value->IntResult);
            }
            else{
                // cout << " Fvalue: " << value->FloatResult << endl;
                signTable.insertFconst(ident,value->FloatResult);
            }
        } else {
            if(value->type == ExpResult::INT) 
                signTable.insertConst(ident,value->IntResult);
            else
                signTable.insertConst(ident,value->FloatResult);
        }
            break;
        }
        case SINCONST_ARRAY: {//这里有个非常恶心的地方是该如何存的问题
        //我的感觉是这里还是需要按照之前的方式存储，但是读的时候就需要另外处理了
            string SrcSign;
            vector<int> dimens;
            arrayDimen->generateGraph(dimens);
            constArrayInit->generateGraphGlobal(SrcSign, retType);
            RawValueP dest,rawSrc;
            getMidVarValue(rawSrc,SrcSign);
            RawValueP src;
            fillZero(rawSrc,src,dimens);
            RawValue *init = (RawValue *) src;
            generateRawValueArrGlobal(DestSign,dimens,init,retType);
            break;
        }//这里const和其他的是有一定的区别的
    }   
}

