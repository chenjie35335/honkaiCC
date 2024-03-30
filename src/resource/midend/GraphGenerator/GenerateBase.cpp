#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
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
            constGlobal->generateGraph();
            break;
        case COMP_VAR:{
            //varGlobal->Dump(DECL_GLOB);
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
    generateRawFunction(p,ident,retType);
    signTable.identForward();
    RawBasicBlock *q;
    string FirstBB = "entry";
    generateRawBasicBlock(q,FirstBB);
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
    generateRawValueArgs(ident,index);
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
          case STMTAST_RET:{
              string RetValue;
              SinExp->generateGraph(RetValue);
              RawValueP RetSrc;
              getMidVarValue(RetSrc,RetValue);
              generateRawValue(RetSrc);
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
          case STMTAST_SINE: 
              SinExp->generateGraph(); break; 
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
          default: assert(0);
      }
}
//这里有最后一个小问题就是常数的问题，这个暂且搁置

