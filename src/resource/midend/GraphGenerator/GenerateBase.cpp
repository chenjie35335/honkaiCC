#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern IRBuilder* irBuilder;
extern SignTable signTable;
int funcNum = 0;

void CompUnitAST::generateGraph(RawProgramme *&IR) const {
    irBuilder = new IRBuilder();
    createRawProgramme(IR);
    setTempProgramme(IR);
    multCompUnit->generateGraph();
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
            varGlobal->generateGraphGlobal();
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
    switch(type) {
        case PARA_VAR:
            generateRawValueArgs(ident,index);
            break;
        case PARA_ARR_SIN:
            generateRawValueSinArr(ident,index);
            break;
        case PARA_ARR_MUL:
            vector<int>dimens;
            arrayDimen->generateGraph(dimens);
            generateRawValueMulArr(ident,index,dimens);
            break;
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
}

void StmtAST::generateGraph() const {
      switch(type) {
          case STMTAST_RET:{//这个ret做的不是很好
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
//则返回的类型就是
            string ElementSign;
            for(auto &dimen : dimens) {
                generateElement(dest,dimen,ElementSign);
                //cout << "elementName = " << ElementSign << endl;
                dest = signTable.getMidVar(ElementSign);
            }
            //cout << "create dest tag :" << dest->value.tag << endl;
            generateRawValue(src,dest);
            break;
          }
          default: assert(0);
      }
}
//这里有最后一个小问题就是常数的问题，这个暂且搁置
//这里我干一个比较傻逼的事情是：不判断类型

