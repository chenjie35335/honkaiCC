#ifndef FUTUROW_ASTPRINT
#define FUTUROW_ASTPRINT
#include "../midend/AST/BaseAST.h"
#include "../midend/AST/BlockAST.h"
#include "../midend/AST/DeclAST.h"
#include "../midend/AST/ExpAST.h"
#include "../midend/AST/FuncAST.h"
#include "../midend/AST/LValAST.h"
#include "../midend/AST/OpAST.h"
#include "../midend/AST/StmtAST.h"
#include "../midend/AST/ArrayAST.h"
static int idx = 0;
static unordered_map<void *,int> node_idx;
static bool astPrintGraphMode = false;
void Generator_ast(unique_ptr<BaseAST> &ast,int mode);
void printNodeEdge(void * a,void *b,string lb);
void printCompUnitAST(CompUnitAST *CompUnitAST);
void printMultCompUnitAST(MultCompUnitAST *multCompUnit);
void printSinCompUnitAST(SinCompUnitAST *sinCompUnit);
void printFuncDefAST(FuncDefAST * funcDef,FuncTypeAST * funcTypeAST);
void printFuncFParamsAST(FuncFParamsAST * funcFParams);
void printSinFuncFParamAST(SinFuncFParamAST * sinFuncFParamAST);
void printArrayDimenAST(ArrayDimenAST* arrayDimenAST);
void printSinArrayDimenAST(SinArrayDimenAST* sinArrayDimenAST);
void printConstArrayInitAST(ConstArrayInitAST* constArrayInitAST);
void printMultiArrayElementAST(MultiArrayElementAST*multiArrayElementAST);
void printSinArrayElementAST(SinArrayElementAST*sinArrayElementAST);

void printBlockAST(BlockAST* blockAST);
void printMulBlockItemAST(MulBlockItemAST * mulBlockItemAST);
void printSinBlockItemAST(SinBlockItemAST* sinBlockItemAST);
void printDeclAST(DeclAST * declAST);

void printConstExpAST(ConstExpAST* constExpAST);
void printExpAST(ExpAST* expAST);
void printLOrExpAST(LOrExpAST* lOrExpAST);
void printLAndExpAST(LAndExpAST * lAndExpAST);
void printEqExpAST(EqExpAST* eqExpAST);
void printRelExpAST(RelExpAST* relExpAST);
void printAddExpAST(AddExpAST* addExpAST);
void printMulExpAST(MulExpAST* mulExpAST);
void printUnaryExp(BaseAST* unaryExpAST);
void printUnaryExpAST_P(UnaryExpAST_P* unaryExpAST_P);
void printPrimaryExpAST(PrimaryExpAST* primaryExpAST);

void printConstDeclAST(ConstDeclAST*constDeclAST);
void printVarDeclAST(VarDeclAST* VarDeclAST,FuncTypeAST * funcTypeAST);
void printVarDeclAST(VarDeclAST* VarDeclAST);
void printMulVarDefAST(MulVarDefAST*mulVarDefAST);
void printSinVarDefAST(SinVarDefAST* sinVarDefAST);
void printInitValAST(InitValAST* initValAST);
void printMulConstDefAST(MulConstDefAST* mulConstDefAST);
void printSinConstDefAST(SinConstDefAST* sinConstDefAST);

void printStmtAST(StmtAST* stmtAST);
void printSinExpAST(SinExpAST* sinExpAST);
void printIfStmtAST(IfStmtAST* ifStmtAST);
void printSinIfStmtAST(SinIfStmtAST* sinIfStmtAST);
void printMultElseStmtAST(MultElseStmtAST*multElseStmtAST);
void printWhileStmtHeadAST(WhileStmtHeadAST*whileStmtHeadAST);
void printWhileStmtAST(WhileStmtAST*whileStmtAST);
void printInWhileAST(InWhileAST* inWhileAST);

void printBtypeAST(BtypeAST* BtypeAST);
void printRelOpAST(RelOpAST* relOpAST);
void printEqOpAST(EqOpAST* eqOpAST);
void printMulOpAST(MulOpAST*mulOpAST);
void printAddOpAST(AddOpAST * addOpAST);
void printUnaryOpAST(UnaryOpAST*unaryOpAST);

void printFuncExpAST(FuncExpAST*funcExpAST);
void printParamsAST(ParamsAST*paramsAST);
void printSinParamsAST(SinParamsAST*sinParamsAST);
void printLValRAST(LValRAST*lValRAST);
#endif