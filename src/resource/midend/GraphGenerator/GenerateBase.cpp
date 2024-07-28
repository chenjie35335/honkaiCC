#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
#define MAGIC 0xEEEEE
extern IRBuilder* irBuilder;
extern SignTable signTable;
int funcNum = 0;
void backend(RawProgramme *&IR);

void CompUnitAST::generateGraph(RawProgramme *&IR) const {
    irBuilder = new IRBuilder();
    createRawProgramme(IR);
    setTempProgramme(IR);
    multCompUnit->generateGraph();
    //backend(IR);
    delete irBuilder;
}

void MultCompUnitAST::generateGraph() const{
    GeneratePutch();
    GeneratePutInt();
    GenerateStartTime();
    GenerateStopTime();
    GenerateGetch();
    GenerateGetInt();
    GeneratePutArray();
    GenerateGetArray();
    //GeneratePutFloat();
    //GenerateGetFloat();
    //GenerateGetFArray();
    //GeneratePutFArray();
    for(auto &sinComp : sinCompUnit) {
        sinComp->generateGraph();
    }
} 

void SinCompUnitAST::generateGraph() const{
    switch(type){
        case COMP_FUNC: {
            int retType;
            funcType->generateGraph(retType);
            funcDef->generateGraph(retType);
            break;
        }
        case COMP_CON:
            constGlobal->generateGraphGlobal();
            break;
        case COMP_VAR:{
            //需要添加类型
            int tempType = funcType->calc();
            int retType;
            if(tempType == FUNCTYPE_FLOAT){
                retType = RTT_FLOAT;
                varGlobal->generateGraphGlobal(retType);
            } else {
                retType = RTT_INT32;
                varGlobal->generateGraphGlobal(retType);
            }
            break;
        }
        case COMP_ARR:{
            break;
        }
        default:
            assert(0);
      }
}
//访问函数类型
void FuncTypeAST::generateGraph(int &retType) const {
    retType = type;
}
//这里访问的是RawFunction
//这里设置funcs的位置应该提前
void FuncDefAST::generateGraph(int &retType) const{
    RawFunction* p;
    generateRawFunction(p,ident.c_str(),retType);
    signTable.identForward();
    RawBasicBlock *q;
    string FirstBB = "entry";
    generateRawBasicBlock(q,FirstBB.c_str());
    PushRawBasicBlock(q);
    setTempBasicBlock(q);
    setFinished(false);
    FuncFParams->generateGraph();
    signTable.insertFunction(ident,p);
    signTable.identBackward();
    block->generateGraph();
    if(!getFinished()) {
        RawValueP RetSrc = nullptr;
        generateRawValue(RetSrc);
    }
    signTable.clearMidVar();
    PushRawFunction(p);
}

//访问参数
void FuncFParamsAST::generateGraph() const{
    for(int i = 0; i < para.size();i++) {
        auto &par = para[i];
        par->generateGraph(i);
    }
}
//单个参数访问
void SinFuncFParamAST::generateGraph(int &index) const{
    int para = paraType->calc();
    int RetFlag;
    if(para == TYPE_FLOAT) RetFlag = RTT_FLOAT;
    else RetFlag = RTT_INT32;
    switch(type) {
        case PARA_VAR:{
                string ArgName = ident+"_"+to_string(signTable.IdentTable->level);
                //cerr << ArgName << endl;
                string TempArgName = ident;
                //cerr << TempArgName << endl;
                generateRawValueArgs(ArgName,index,RetFlag);
                generateRawValue(TempArgName,RetFlag);
                RawValueP TempArg = signTable.getVarR(TempArgName);
                RawValueP Arg = signTable.getVarR(ArgName);
                generateRawValue(Arg,TempArg);
            break;
        }  
        case PARA_ARR_SIN:{
            generateRawValueSinArr(ident,index,RetFlag);
            break;
        }
        case PARA_ARR_MUL:{
             vector<int>dimens;
            //int para = paraType->calc();
            arrayDimen->generateGraph(dimens);
            generateRawValueMulArr(ident,index,dimens,RetFlag);
            break;
        }  
    }
}

//这个blockAST的generateGraph对于分支语句来说是个重点
//所以需要一个数据结构来存储当前的RawFunction下的RawSlice
//到了函数阶段以后，由于产生了函数调用，所以可能还需要添加当前RawProgram下的RawSlice
void BlockAST::generateGraph() const {
    signTable.identForward();
    MulBlockItem->generateGraph();
    signTable.deleteBackward();
}

void MulBlockItemAST::generateGraph() const {
    for(auto &sinBlockItem : SinBlockItem) {
        if(getFinished()) break;
        sinBlockItem->generateGraph();
    }
}

void SinBlockItemAST::generateGraph() const{
    switch(type) {
       case SINBLOCKITEM_DEC: 
                decl->generateGraph();break;
                break;
        case SINBLOCKITEM_STM: 
                stmt->generateGraph();break;
        default:assert(0);
      }
    //SSA
    
}

//这里感觉左值没有什么问题，但是问题最大的是右值
void StmtAST::generateGraph() const {
    RawValueP zero = generateZero();
      switch(type) {
          case STMTAST_RET:{
            //这个ret做的不是很好
              string RetValue;int type;
              SinExp->generateGraph(RetValue,type);
              RawValueP RetSrc ;
              if(type == SINEXPAST_EXP) {
              getMidVarValue(RetSrc,RetValue);
              } else RetSrc = nullptr;
              generateRawValue(RetSrc);
              PushFollowBasieBlock();
              setFinished(true);
              break;
          }
          case STMTAST_LVA: 
          {
            //此处SSA入口关键点
            ///exp ident
            //ident = exp
            //比较两者是否是同一个符号
            string ExpSign,IdentSign;
            IdentSign = this->ident;
            Exp->generateGraph(ExpSign);
            RawValueP src,dest;
            getMidVarValue(src,ExpSign);
            getVarValueL(dest,IdentSign);
            generateRawValue(src,dest);
            break;
          }
          case STMTAST_SINE: {
              string SinExpSign;int type;
              SinExp->generateGraph(SinExpSign,type); 
              break; 
          }
          case STMTAST_BLO: 
              Block->generateGraph();break;
          case STMTAST_IF:
              ifStmt->generateGraph();break;
          case STMTAST_WHILE:
              WhileHead->generateGraph();break;
          case STMTAST_INWHILE:
              if(!InWhile()) {
                cerr << "Break or Continue not in while scope" << endl;
                exit(-1);
              }
              else InWhileStmt->generateGraph();
              break;
          case STMTAST_ARR:{
            string ExpSign,IdentSign;
            IdentSign = this->ident;
            Exp->generateGraph(ExpSign);
            RawValueP src,dest;
            getMidVarValue(src,ExpSign);
            getVarValueL(dest,IdentSign);
            vector<RawValueP> dimens;
            arrPara->generateGraph(dimens);
            auto DestType = dest->identType;
            auto &DestBase = dest->ty->pointer.base;
            auto DestBaseTag = DestBase->tag;
            string ElementSign;
            if(DestType == IDENT_ARR) {
                 for(auto &dimen : dimens) {
                     generateElement(dest,dimen,ElementSign);
                     dest = signTable.getMidVar(ElementSign);
                     DestBaseTag = dest->ty->pointer.base->tag;
                 }
                 //to find the position
                 //insert to arrTable
                 RawValue * temp;
                 temp->value.integer.value = stoi(ExpSign);
                 signTable.IdentTable->ArrayTable.at(ident)->arrValue.elements[DestBaseTag] = temp;
            } else if(DestType == IDENT_POINTER) {
                auto it = dimens.begin();
                generatePtr(dest, *it, ElementSign);
                dest = signTable.getMidVar(ElementSign);
                for(advance(it,1);it != dimens.end();it++) {
                    generateElement(dest, *it, ElementSign);
                    dest = signTable.getMidVar(ElementSign);
                }
                DestBaseTag = dest->ty->pointer.base->tag;
            } else {
                cerr << "unknown type:" << DestType << endl;
                assert(0);
            }
            generateRawValue(src,dest);
            break;
          }
          default: assert(0);
      }
}
//类型转换应该在什么时候出现？
//先不考虑数组的问题，先局部变量
//float a = 1;这里就直接转换就行，编译器直接建立1.0的浮点数节点然后直接store进去就完了
//float a = b;如果这里b是int型，这里发现，需要将b load出来，然后这时需要转换，再store进去
//对于a+b这种也是需要先转换再处理