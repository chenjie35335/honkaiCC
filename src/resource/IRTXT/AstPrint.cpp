#include <iostream>
#include "../../include/IRTXT/AstPrint.h"
void Generator_ast(unique_ptr<BaseAST> &ast,int mode){
    if(mode == 1){// 文本
        astPrintGraphMode = false;
        printCompUnitAST((CompUnitAST*)ast.get());
    }
    else if(mode == 2){//图
        astPrintGraphMode = true;
        cout<<"digraph G {"<<endl;
        printCompUnitAST((CompUnitAST*)ast.get());
        cout<<"}"<<endl;
    }
}
void printNodeEdge(void * a,void *b,string lb){
    if(node_idx.count(b)==0)
        node_idx[b]=idx++;

    cout<<"node_"+to_string(node_idx[b])<<" [label="<<lb<<"];"<<endl;

    cout<<"node_"+to_string(node_idx[a])<<" -> "<<"node_"+to_string(node_idx[b])<<";"<<endl;
}
void printCompUnitAST(CompUnitAST *CompUnitAST){
    if(astPrintGraphMode)
    {
        node_idx[CompUnitAST]=idx++;
        cout<<"node_"+to_string(node_idx[CompUnitAST])<<" [label=CompUnit];"<<endl;
        printNodeEdge(CompUnitAST,CompUnitAST->multCompUnit.get(),"MultCompUnit");
        printMultCompUnitAST((MultCompUnitAST *)CompUnitAST->multCompUnit.get());
    }
    else{
        cout<<"CompUnitAST:{"<<endl;
        printMultCompUnitAST((MultCompUnitAST *)CompUnitAST->multCompUnit.get());
        cout<<"}"<<endl;
    }
}
void printMultCompUnitAST(MultCompUnitAST *multCompUnit){
    if(astPrintGraphMode){
        for(auto &sinCompUnit:multCompUnit->sinCompUnit){
            printNodeEdge(multCompUnit,sinCompUnit.get(),"sinCompUnit");
            printSinCompUnitAST((SinCompUnitAST *)sinCompUnit.get());
        }
    }
    else{
    cout<<"MultCompUnitAST:{"<<endl;
    for(auto &sinCompUnit:multCompUnit->sinCompUnit)
        printSinCompUnitAST((SinCompUnitAST *)sinCompUnit.get());
    cout<<"}"<<endl;
    }
}
void printSinCompUnitAST(SinCompUnitAST *sinCompUnit)
{
    // ConstDecl | FuncType FuncDef | FuncType VarDecl
    switch (sinCompUnit->type)
    {
    case COMP_FUNC: {
        if(astPrintGraphMode)
        {
            printNodeEdge(sinCompUnit,sinCompUnit->funcDef.get(),"FuncDef");
            printFuncDefAST((FuncDefAST*)sinCompUnit->funcDef.get(),(FuncTypeAST*)sinCompUnit->funcType.get());
        }
        else{
            cout<<"SinCompUnitAST:{"<<endl;
            printFuncDefAST((FuncDefAST*)sinCompUnit->funcDef.get(),(FuncTypeAST*)sinCompUnit->funcType.get());
            cout<<"}"<<endl;
        }
        break;
    }
    case COMP_CON:{
        if(astPrintGraphMode)
        {
            printNodeEdge(sinCompUnit,sinCompUnit->constGlobal.get(),"ConstDecl");
            printConstDeclAST((ConstDeclAST*)sinCompUnit->constGlobal.get());
        }
        else{
            cout<<"ConstDecl:{"<<endl;
            printConstDeclAST((ConstDeclAST*)sinCompUnit->constGlobal.get());
            cout<<"}"<<endl;
        }
        break;
    }
    case COMP_VAR:{
        if(astPrintGraphMode)
        {
            printNodeEdge(sinCompUnit,sinCompUnit->varGlobal.get(),"VarDecl");
            printVarDeclAST((VarDeclAST*)sinCompUnit->varGlobal.get(),(FuncTypeAST*)sinCompUnit->funcType.get());

        }
        else{
            cout<<"VarDecl:{"<<endl;
            printVarDeclAST((VarDeclAST*)sinCompUnit->varGlobal.get(),(FuncTypeAST*)sinCompUnit->funcType.get());
            cout<<"}"<<endl;
        }
        break;
        }
    case COMP_ARR:{
        if(astPrintGraphMode)
        {

        }
        else{
            cout<<"funcType:{"<<endl;
            cout<<"}"<<endl;
        }
        break;
    }
    default:
        assert(0);
    }
}
void printFuncDefAST(FuncDefAST * funcDef,FuncTypeAST * funcTypeAST)
{
    if(astPrintGraphMode){
        printNodeEdge(funcDef,&funcDef->ident,funcDef->ident);

        printNodeEdge(funcDef,funcTypeAST,"FuncType");
        if(funcTypeAST->type==FUNCTYPE_INT)
            printNodeEdge(funcTypeAST,&funcTypeAST->type,"INT");
        else if(funcTypeAST->type==FUNCTYPE_FLOAT)
            printNodeEdge(funcTypeAST,&funcTypeAST->type,"FLOAT");
        else if(funcTypeAST->type==FUNCTYPE_VOID)
            printNodeEdge(funcTypeAST,&funcTypeAST->type,"VOID");

        printNodeEdge(funcDef,funcDef->block.get(),"Block");
        printBlockAST((BlockAST*)funcDef->block.get());

        printNodeEdge(funcDef,funcDef->FuncFParams.get(),"FuncFParams");
        printFuncFParamsAST((FuncFParamsAST *)funcDef->FuncFParams.get());
    }
    else{
        cout<<"FuncDef:{"<<endl;

        cout<<"ident:"<<funcDef->ident<<","<<endl;
        if(funcTypeAST->type==FUNCTYPE_INT)
            cout<<"FuncType:INT,"<<endl;
        else if(funcTypeAST->type==FUNCTYPE_FLOAT)
            cout<<"FuncType:FLOAT,"<<endl;
        else if(funcTypeAST->type==FUNCTYPE_VOID)
            cout<<"FuncType:VOID,"<<endl;

        cout<<"Block:{"<<endl;
        printBlockAST((BlockAST*)funcDef->block.get());
        cout<<"},"<<endl;

        cout<<"FuncFParams:{"<<endl;
        printFuncFParamsAST((FuncFParamsAST *)funcDef->FuncFParams.get());
        cout<<"}"<<endl;

        cout<<"}"<<endl;
    }
}
void printFuncFParamsAST(FuncFParamsAST * funcFParams)
{
    if(astPrintGraphMode){
        for(auto &para:funcFParams->para)
        {
            printNodeEdge(funcFParams,para.get(),"para");
            printSinFuncFParamAST((SinFuncFParamAST *)para.get());
        }
    }
    else{
        cout<<"FuncFParamsAST:{"<<endl;
        for(auto &para:funcFParams->para)
        {
            printSinFuncFParamAST((SinFuncFParamAST *)para.get());
        }
        cout<<"}"<<endl;
    }
}
void printSinFuncFParamAST(SinFuncFParamAST * sinFuncFParamAST)
{

    if(astPrintGraphMode){
        printNodeEdge(sinFuncFParamAST,&sinFuncFParamAST->ident,sinFuncFParamAST->ident);
        switch(sinFuncFParamAST->type) {
            case PARA_VAR:{//整型或浮点
                printNodeEdge(sinFuncFParamAST,sinFuncFParamAST->paraType.get(),"paraType");
                ParaTypeAST * paraTypeAST = (ParaTypeAST*)sinFuncFParamAST->paraType.get();
                if(paraTypeAST->type==TYPE_INT){
                    printNodeEdge(sinFuncFParamAST->paraType.get(),&paraTypeAST->type,"INT");
                }
                else{
                    printNodeEdge(sinFuncFParamAST->paraType.get(),&paraTypeAST->type,"FLOAT");

                }
                break;
            }
            case PARA_ARR_SIN:{
                // printNodeEdge(sinFuncFParamAST,&sinFuncFParamAST->type,"PARA_ARR_SIN");
                printNodeEdge(sinFuncFParamAST,sinFuncFParamAST->paraType.get(),"paraType");
                ParaTypeAST * paraTypeAST = (ParaTypeAST*)sinFuncFParamAST->paraType.get();
                if(paraTypeAST->type==TYPE_INT){
                    printNodeEdge(sinFuncFParamAST->paraType.get(),&paraTypeAST->type,"INT_ARR_SIN");
                }
                else{
                    printNodeEdge(sinFuncFParamAST->paraType.get(),&paraTypeAST->type,"FLOAT_ARR_SIN");

                }
                break;
            }
            case PARA_ARR_MUL:{
                printNodeEdge(sinFuncFParamAST,sinFuncFParamAST->paraType.get(),"paraType");
                ParaTypeAST * paraTypeAST = (ParaTypeAST*)sinFuncFParamAST->paraType.get();
                if(paraTypeAST->type==TYPE_INT){
                    printNodeEdge(sinFuncFParamAST->paraType.get(),&paraTypeAST->type,"INT_ARR_MUL");
                }
                else{
                    printNodeEdge(sinFuncFParamAST->paraType.get(),&paraTypeAST->type,"FLOAT_ARR_MUL");

                }
                printNodeEdge(&paraTypeAST->type,sinFuncFParamAST->arrayDimen.get(),"ArrayDimen");
                printArrayDimenAST((ArrayDimenAST*)sinFuncFParamAST->arrayDimen.get());
                break;
            }
        }
    }
    else{
        cout<<"sinFuncFParamAST:{"<<endl;

        cout<<"ident:"<<sinFuncFParamAST->ident<<","<<endl;
        switch(sinFuncFParamAST->type) {
            case PARA_VAR:{
                ParaTypeAST * paraTypeAST = (ParaTypeAST*)sinFuncFParamAST->paraType.get();
                if(paraTypeAST->type==TYPE_INT)
                    cout<<"ParaType:INT,"<<endl;
                else
                    cout<<"ParaType:FLOAT,"<<endl;
                break;
            }
            case PARA_ARR_SIN:{
                ParaTypeAST * paraTypeAST = (ParaTypeAST*)sinFuncFParamAST->paraType.get();
                if(paraTypeAST->type==TYPE_INT)
                    cout<<"ParaType:INT_ARR_SIN,"<<endl;
                else
                    cout<<"ParaType:FLOAT_ARR_SIN,"<<endl;
                break;
            }
            case PARA_ARR_MUL:{
                ParaTypeAST * paraTypeAST = (ParaTypeAST*)sinFuncFParamAST->paraType.get();
                if(paraTypeAST->type==TYPE_INT)
                    cout<<"ParaType:INT_ARR_MUL:";
                else
                    cout<<"ParaType:FLOAT_ARR_MUL:";
                printArrayDimenAST((ArrayDimenAST*)sinFuncFParamAST->arrayDimen.get());
                cout<<","<<endl;
                break;
            }
        }
        cout<<"}"<<endl;
    }
}
void printArrayDimenAST(ArrayDimenAST* arrayDimenAST){
    if(astPrintGraphMode){
        for(auto &sinArrayDimen:arrayDimenAST->sinArrayDimen)
        {
            printNodeEdge(arrayDimenAST,sinArrayDimen.get(),"sinArrayDimen");
            printSinArrayDimenAST((SinArrayDimenAST*)sinArrayDimen.get());
        }
    }
    else{
        cout<<"ArrayDimen:{"<<endl;
        for(auto &sinArrayDimen:arrayDimenAST->sinArrayDimen)
        {
            cout<<"sinArrayDimen:"<<endl;
            printSinArrayDimenAST((SinArrayDimenAST*)sinArrayDimen.get());
            cout<<","<<endl;
        }
        cout<<"}"<<endl;
    }
}
void printSinArrayDimenAST(SinArrayDimenAST* sinArrayDimenAST){
     if(astPrintGraphMode){
        printNodeEdge(sinArrayDimenAST,sinArrayDimenAST->exp.get(),"ConstExp");
        printConstExpAST((ConstExpAST*)sinArrayDimenAST->exp.get());
     }
     else{
        cout<<"ConstExp:{"<<endl;
        printConstExpAST((ConstExpAST*)sinArrayDimenAST->exp.get());
        cout<<"}"<<endl;
     }
}
void printConstExpAST(ConstExpAST* constExpAST){
    if(astPrintGraphMode){
        printNodeEdge(constExpAST,constExpAST->Exp.get(),"Exp");
        printExpAST((ExpAST*)constExpAST->Exp.get());
    }
    else{
        cout<<"Exp:{"<<endl;
        printExpAST((ExpAST*)constExpAST->Exp.get());
        cout<<"}"<<endl;
    }
}
void printConstArrayInitAST(ConstArrayInitAST* constArrayInitAST){
    //'{' MultiArrayElement '}'| '{' '}'
    int type = constArrayInitAST->type;
    if(astPrintGraphMode){
        if(type==ConstArrayInitAST::INIT_MUL){
            printNodeEdge(constArrayInitAST,constArrayInitAST->multiArrayElement.get(),"MultiArrayElement");
            printMultiArrayElementAST((MultiArrayElementAST*)constArrayInitAST->multiArrayElement.get());
        }
        else if(type==ConstArrayInitAST::INIT_NULL){
            cout<<"node_"+to_string(idx++)<<" [label=\"{}\"];"<<endl;
            cout<<"node_"+to_string(node_idx[constArrayInitAST])<<" -> "<<"node_"+to_string(idx-1)<<";"<<endl;
        }
    }
    else{
        if(type==ConstArrayInitAST::INIT_MUL){
            cout<<"MultiArrayElement:{"<<endl;
            printMultiArrayElementAST((MultiArrayElementAST*)constArrayInitAST->multiArrayElement.get());
            cout<<"}"<<endl;
        }
        else if(type==ConstArrayInitAST::INIT_NULL){
            cout<<"{}"<<endl;

        }
    }
}
void printMultiArrayElementAST(MultiArrayElementAST*multiArrayElementAST){
    //SinArrayElement | MultiArrayElement ',' SinArrayElement
    if(astPrintGraphMode){
        for (auto &sinArrayElement:multiArrayElementAST->sinArrayElement)
        {
            printNodeEdge(multiArrayElementAST,sinArrayElement.get(),"SinArrayElement");
            printSinArrayElementAST((SinArrayElementAST*)sinArrayElement.get());
        }
        
    }
    else{
        for (auto &sinArrayElement:multiArrayElementAST->sinArrayElement)
        {
            cout<<"SinArrayElement:{"<<endl;
            printSinArrayElementAST((SinArrayElementAST*)sinArrayElement.get());
            cout<<"}"<<endl;
        }
    }
}
void printSinArrayElementAST(SinArrayElementAST*sinArrayElementAST){
    //ConstExp | ConstArrayInit
    int type = sinArrayElementAST->type;
    if(astPrintGraphMode){
        if(type==SinArrayElementAST::ARELEM_EX){
            printNodeEdge(sinArrayElementAST,sinArrayElementAST->constExp.get(),"ConstExp");
            printConstExpAST((ConstExpAST*)sinArrayElementAST->constExp.get());
        }
        else if(type==SinArrayElementAST::ARELEM_AI){
            printNodeEdge(sinArrayElementAST,sinArrayElementAST->constArrayInit.get(),"ConstArrayInit");
            printConstArrayInitAST((ConstArrayInitAST*)sinArrayElementAST->constArrayInit.get());
        }
    }
    else{
        if(type==SinArrayElementAST::ARELEM_EX){
            cout<<"ConstExp:{"<<endl;
            printConstExpAST((ConstExpAST*)sinArrayElementAST->constExp.get());
            cout<<"}"<<endl;
        }
        else if(type==SinArrayElementAST::ARELEM_AI){
            cout<<"ConstArrayInit:{"<<endl;
            printConstArrayInitAST((ConstArrayInitAST*)sinArrayElementAST->constArrayInit.get());
            cout<<"}"<<endl;
        }
    }
}

void printBlockAST(BlockAST* blockAST){
     if(astPrintGraphMode){
        printNodeEdge(blockAST,blockAST->MulBlockItem.get(),"MulBlockItem");
        printMulBlockItemAST((MulBlockItemAST*)blockAST->MulBlockItem.get());
     }
     else{
        cout<<"MulBlockItem:{"<<endl;
        printMulBlockItemAST((MulBlockItemAST*)blockAST->MulBlockItem.get());
        cout<<"}"<<endl;
     }
}
void printMulBlockItemAST(MulBlockItemAST * mulBlockItemAST){
    // SinBlockItem | MulBlockItem SinBlockItem
    if(astPrintGraphMode){
        for (auto &sinBlockItem:mulBlockItemAST->SinBlockItem)
        {
            printNodeEdge(mulBlockItemAST,sinBlockItem.get(),"SinBlockItem");
            printSinBlockItemAST((SinBlockItemAST*)sinBlockItem.get());
        }
        if(0==mulBlockItemAST->SinBlockItem.size())
        {
            cout<<"node_"+to_string(idx++)<<" [label=\"{}\"];"<<endl;
            cout<<"node_"+to_string(node_idx[mulBlockItemAST])<<" -> "<<"node_"+to_string(idx-1)<<";"<<endl;
        }
     }
     else{
        for (auto &sinBlockItem:mulBlockItemAST->SinBlockItem)
        {
            cout<<"SinBlockItem:{"<<endl;
            printSinBlockItemAST((SinBlockItemAST*)sinBlockItem.get());
            cout<<"},"<<endl;
        }
        if(0==mulBlockItemAST->SinBlockItem.size()){
            cout<<"{}"<<endl;
        }
     }
}
void printSinBlockItemAST(SinBlockItemAST* sinBlockItemAST){
    // Decl | Stmt 
    uint32_t type = sinBlockItemAST->type;
    if(astPrintGraphMode){
        if(type==SINBLOCKITEM_DEC){
            printNodeEdge(sinBlockItemAST,sinBlockItemAST->decl.get(),"Decl");
            printDeclAST((DeclAST*)sinBlockItemAST->decl.get());
        }
        else if(type==SINBLOCKITEM_STM){
            printNodeEdge(sinBlockItemAST,sinBlockItemAST->stmt.get(),"Stmt");
            printStmtAST((StmtAST*)sinBlockItemAST->stmt.get());
        }
    }
    else{
        if(type==SINBLOCKITEM_DEC){
            cout<<"Decl:{"<<endl;
            printDeclAST((DeclAST*)sinBlockItemAST->decl.get());
            cout<<"}"<<endl;
        }
        else if(type==SINBLOCKITEM_STM){
            cout<<"Stmt:{"<<endl;
            printStmtAST((StmtAST*)sinBlockItemAST->stmt.get());
            cout<<"}"<<endl;
        }
    }
}
void printDeclAST(DeclAST * declAST){
    // ConstDecl | | Btype VarDecl
    uint32_t type = declAST->type;
    if(astPrintGraphMode){
        if(type==DECLAST_CON){
            printNodeEdge(declAST,declAST->ConstDecl.get(),"ConstDecl");
            printConstDeclAST((ConstDeclAST*)declAST->ConstDecl.get());
        }
        else if(type==DECLAST_VAR){
            printNodeEdge(declAST,declAST->btype.get(),"Btype");
            printBtypeAST((BtypeAST*)declAST->btype.get());

            printNodeEdge(declAST,declAST->VarDecl.get(),"VarDecl");
            printVarDeclAST((VarDeclAST*)declAST->VarDecl.get());
        }
    }
    else{
        if(type==DECLAST_CON){
            cout<<"ConstDecl:{"<<endl;
            printConstDeclAST((ConstDeclAST*)declAST->ConstDecl.get());
            cout<<"}"<<endl;
        }
        else if(type==DECLAST_VAR){
            cout<<"Btype:";
            printBtypeAST((BtypeAST*)declAST->btype.get());
            cout<<","<<endl;

            cout<<"VarDecl:{"<<endl;
            printVarDeclAST((VarDeclAST*)declAST->VarDecl.get());
            cout<<"}"<<endl;
        }
    }
}

void printExpAST(ExpAST* expAST){
    if(astPrintGraphMode){
        printNodeEdge(expAST,expAST->LOrExp.get(),"LOrExp");
        printLOrExpAST((LOrExpAST*)expAST->LOrExp.get());
    }
    else{
        cout<<"LOrExp:{"<<endl;
        printLOrExpAST((LOrExpAST*)expAST->LOrExp.get());
        cout<<"}"<<endl;
    }
}
void printLOrExpAST(LOrExpAST* lOrExpAST){
    uint32_t type = lOrExpAST->type;
    if(astPrintGraphMode){
        if(type==LOREXPAST_LAN){
            printNodeEdge(lOrExpAST,lOrExpAST->LAndExp.get(),"LAndExp");
            printLAndExpAST((LAndExpAST *)lOrExpAST->LAndExp.get());
        }
        else if(type==LOREXPAST_LOR){
            printNodeEdge(lOrExpAST,lOrExpAST->LOrExp.get(),"LOrExp");
            printLOrExpAST((LOrExpAST*)lOrExpAST->LOrExp.get());

            printNodeEdge(lOrExpAST,lOrExpAST->LAndExp.get(),"LAndExp");
            printLAndExpAST((LAndExpAST *)lOrExpAST->LAndExp.get());
        }
    }
    else{
        if(type==LOREXPAST_LAN){
            cout<<"LAndExp:{"<<endl;
            printLAndExpAST((LAndExpAST *)lOrExpAST->LAndExp.get());
            cout<<"}"<<endl;
        }
        else if(type==LOREXPAST_LOR){
            cout<<"LOrExp:{"<<endl;
            printLOrExpAST((LOrExpAST*)lOrExpAST->LOrExp.get());
            cout<<"},"<<endl;

            cout<<"LAndExp:{"<<endl;
            printLAndExpAST((LAndExpAST *)lOrExpAST->LAndExp.get());
            cout<<"}"<<endl;
        }
    }
}
void printLAndExpAST(LAndExpAST * lAndExpAST){
    uint32_t type = lAndExpAST->type;
    if(astPrintGraphMode){
        if(type==LANDEXPAST_EQE){
            printNodeEdge(lAndExpAST,lAndExpAST->EqExp.get(),"EqExp");
            printEqExpAST((EqExpAST*)lAndExpAST->EqExp.get());
        }
        else if(type==LANDEXPAST_LAN){
            printNodeEdge(lAndExpAST,lAndExpAST->LAndExp.get(),"LAndExp");
            printLAndExpAST((LAndExpAST*)lAndExpAST->LAndExp.get());
            printNodeEdge(lAndExpAST,lAndExpAST->EqExp.get(),"EqExp");
            printEqExpAST((EqExpAST*)lAndExpAST->EqExp.get());
        }
    }
    else{
        if(type==LANDEXPAST_EQE){
            cout<<"EqExp:{"<<endl;
            printEqExpAST((EqExpAST*)lAndExpAST->EqExp.get());
            cout<<"}"<<endl;
        }
        else if(type==LANDEXPAST_LAN){
            cout<<"LAndExp:{"<<endl;
            printLAndExpAST((LAndExpAST *)lAndExpAST->LAndExp.get());
            cout<<"},"<<endl;
            cout<<"EqExp:{"<<endl;
            printEqExpAST((EqExpAST*)lAndExpAST->EqExp.get());
            cout<<"}"<<endl;
        }
    }
}
void printEqExpAST(EqExpAST* eqExpAST){
    uint32_t type=eqExpAST->type;
    if(astPrintGraphMode){
        if(type==EQEXPAST_REL){
            printNodeEdge(eqExpAST,eqExpAST->RelExp.get(),"RelExp");
            printRelExpAST((RelExpAST*)eqExpAST->RelExp.get());
        }
        else if(type==EQEXPAST_EQE){
            printNodeEdge(eqExpAST,eqExpAST->EqExp.get(),"EqExp");
            printEqExpAST((EqExpAST*)eqExpAST->EqExp.get());

            printNodeEdge(eqExpAST,eqExpAST->EqOp.get(),"EqOp");
            printEqOpAST((EqOpAST*)eqExpAST->EqOp.get());

            printNodeEdge(eqExpAST,eqExpAST->RelExp.get(),"RelExp");
            printRelExpAST((RelExpAST*) eqExpAST->RelExp.get());
        }
    }
    else{
        if(type==EQEXPAST_REL){
            cout<<"RelExp:{"<<endl;
            printRelExpAST((RelExpAST*) eqExpAST->RelExp.get());
            cout<<"}"<<endl;
        }
        else if(type==EQEXPAST_EQE){
            cout<<"EqExp:{"<<endl;
            printEqExpAST((EqExpAST*)eqExpAST->EqExp.get());
            cout<<"},"<<endl;
            cout<<"EqOp:";
            printEqOpAST((EqOpAST*)eqExpAST->EqOp.get());
            cout<<","<<endl;
            cout<<"RelExp:{"<<endl;
            printRelExpAST((RelExpAST*) eqExpAST->RelExp.get());
            cout<<"}"<<endl;
        }
    }

}
void printRelExpAST(RelExpAST* relExpAST){
    uint32_t type=relExpAST->type;
    if(astPrintGraphMode){
        if(type==RELEXPAST_ADD){
            printNodeEdge(relExpAST,relExpAST->AddExp.get(),"AddExp");
            printAddExpAST((AddExpAST*)relExpAST->AddExp.get());
        }
        else if(type==RELEXPAST_REL){
            printNodeEdge(relExpAST,relExpAST->RelExp.get(),"RelExp");
            printRelExpAST((RelExpAST*) relExpAST->RelExp.get());

            printNodeEdge(relExpAST,relExpAST->RelOp.get(),"RelOp");
            printRelOpAST((RelOpAST*)relExpAST->RelOp.get());
            
            printNodeEdge(relExpAST,relExpAST->AddExp.get(),"AddExp");
            printAddExpAST((AddExpAST*)relExpAST->AddExp.get());
        }
    }
    else{
        if(type==RELEXPAST_ADD){
            cout<<"AddExp:{"<<endl;
            printAddExpAST((AddExpAST*)relExpAST->AddExp.get());
            cout<<"}"<<endl;
        }
        else if(type==RELEXPAST_REL){
            cout<<"RelExp:{"<<endl;
            printRelExpAST((RelExpAST*) relExpAST->RelExp.get());
            cout<<"},"<<endl;
            cout<<"RelOp:";
            printRelOpAST((RelOpAST*)relExpAST->RelOp.get());
            cout<<","<<endl;
            cout<<"AddExp:{"<<endl;
            printAddExpAST((AddExpAST*)relExpAST->AddExp.get());
            cout<<"}"<<endl;
        }
    }
}
void printAddExpAST(AddExpAST* addExpAST){
    uint32_t type=addExpAST->type;
    if(astPrintGraphMode){
        if(type==MULEXP){
            printNodeEdge(addExpAST,addExpAST->MulExp.get(),"MulExp");
            printMulExpAST((MulExpAST*)addExpAST->MulExp.get());
        }
        else if(type==ADDMUL){
            printNodeEdge(addExpAST,addExpAST->AddExp.get(),"AddExp");
            printAddExpAST((AddExpAST*)addExpAST->AddExp.get());

            printNodeEdge(addExpAST,addExpAST->AddOp.get(),"AddOp");
            printAddOpAST((AddOpAST*)addExpAST->AddOp.get());

            printNodeEdge(addExpAST,addExpAST->MulExp.get(),"MulExp");
            printMulExpAST((MulExpAST*)addExpAST->MulExp.get());

        }
    }
    else{
        if(type==MULEXP){
            cout<<"MulExp:{"<<endl;
            printMulExpAST((MulExpAST*)addExpAST->MulExp.get());
            cout<<"}"<<endl;
        }
        else if(type==ADDMUL){
            cout<<"AddExp:{"<<endl;
            printAddExpAST((AddExpAST*)addExpAST->AddExp.get());
            cout<<"},"<<endl;
            cout<<"AddOp:";
            printAddOpAST((AddOpAST*)addExpAST->AddOp.get());
            cout<<","<<endl;
            cout<<"MulExp:{"<<endl;
            printMulExpAST((MulExpAST*)addExpAST->MulExp.get());
            cout<<"}"<<endl;
        }
    }
}
void printMulExpAST(MulExpAST* mulExpAST){
    uint32_t type=mulExpAST->type;
    if(astPrintGraphMode){
        if(type==MULEXPAST_UNA){
            printNodeEdge(mulExpAST,mulExpAST->UnaryExp.get(),"UnaryExp");
            printUnaryExp(mulExpAST->UnaryExp.get());
        }
        else if(type==MULEXPAST_MUL){
            printNodeEdge(mulExpAST,mulExpAST->MulExp.get(),"MulExp");
            printMulExpAST((MulExpAST*) mulExpAST->MulExp.get());

            printNodeEdge(mulExpAST,mulExpAST->MulOp.get(),"MulOp");
            printMulOpAST((MulOpAST*)mulExpAST->MulOp.get());

            printNodeEdge(mulExpAST,mulExpAST->UnaryExp.get(),"UnaryExp");
            printUnaryExp(mulExpAST->UnaryExp.get());
        }
    }
    else{
        if(type==MULEXPAST_UNA){
            cout<<"UnaryExp:{"<<endl;
            printUnaryExp(mulExpAST->UnaryExp.get());
            cout<<"}"<<endl;
        }
        else if(type==MULEXPAST_MUL){
            cout<<"MulExp:{"<<endl;
            printMulExpAST((MulExpAST*) mulExpAST->MulExp.get());
            cout<<"},"<<endl;
            cout<<"MulOp:";
            printMulOpAST((MulOpAST*)mulExpAST->MulOp.get());
            cout<<","<<endl;
            cout<<"UnaryExp:{"<<endl;
            printUnaryExp(mulExpAST->UnaryExp.get());
            cout<<"}"<<endl;
        }
    }
}
void printUnaryExp(BaseAST* unaryExpAST){
    int type = unaryExpAST->UnaryExpType();
    if(astPrintGraphMode){
        if(type==1){//PrimaryExp
            printUnaryExpAST_P((UnaryExpAST_P*)unaryExpAST);
        }
        else if(type==2){//UnaryOp UnaryExp
            printNodeEdge(unaryExpAST,((UnaryExpAST_U*)unaryExpAST)->UnaryOp.get(),"UnaryOp");
            printUnaryOpAST((UnaryOpAST*)((UnaryExpAST_U*)unaryExpAST)->UnaryOp.get());

            printNodeEdge(unaryExpAST,((UnaryExpAST_U*)unaryExpAST)->UnaryExp.get(),"UnaryExp");
            printUnaryExp(((UnaryExpAST_U*)unaryExpAST)->UnaryExp.get());
        }
        else if(type==3){//FuncExp
            printNodeEdge(unaryExpAST,((UnaryExpAST_F*)unaryExpAST)->FuncExp.get(),"FuncExp");
            printFuncExpAST((FuncExpAST*)((UnaryExpAST_F*)unaryExpAST)->FuncExp.get());
        }
    }
    else{
        if(type==1){//PrimaryExp
            printUnaryExpAST_P((UnaryExpAST_P*)unaryExpAST);
        }
        else if(type==2){//UnaryOp UnaryExp
            cout<<"UnaryOp:";
            printUnaryOpAST((UnaryOpAST*)((UnaryExpAST_U*)unaryExpAST)->UnaryOp.get());
            cout<<","<<endl;
            cout<<"UnaryExp:{"<<endl;
             printUnaryExp((BaseAST*)((UnaryExpAST_U*)unaryExpAST)->UnaryExp.get());
            cout<<"}"<<endl;
        }
        else if(type==3){//FuncExp
            cout<<"FuncExp:{"<<endl;
            printFuncExpAST((FuncExpAST*)((UnaryExpAST_F*)unaryExpAST)->FuncExp.get());
            cout<<"}"<<endl;
        }
    }
}
void printUnaryExpAST_P(UnaryExpAST_P* unaryExpAST_P){
    if(astPrintGraphMode){   
        printNodeEdge(unaryExpAST_P,unaryExpAST_P->PrimaryExp.get(),"PrimaryExp");
        printPrimaryExpAST((PrimaryExpAST*)unaryExpAST_P->PrimaryExp.get());
    }
    else{
        cout<<"PrimaryExpAST:{"<<endl;
        printPrimaryExpAST((PrimaryExpAST*)unaryExpAST_P->PrimaryExp.get());
        cout<<"}"<<endl;
    }
}
void printPrimaryExpAST(PrimaryExpAST* primaryExpAST)
{
    uint32_t kind = primaryExpAST->kind;
    if(astPrintGraphMode){
        if(kind==UNARYEXP){
            printNodeEdge(primaryExpAST,primaryExpAST->Exp.get(),"Exp");
            printExpAST((ExpAST*)primaryExpAST->Exp.get());
        }
        else if (kind==LVAL)
        {
            printNodeEdge(primaryExpAST,primaryExpAST->Lval.get(),"LValR");
            printLValRAST((LValRAST*)primaryExpAST->Lval.get());
        }
        else if (kind==NUMBER)
        {
            printNodeEdge(primaryExpAST,&primaryExpAST->kind,"Number");
            printNodeEdge(&primaryExpAST->kind,&primaryExpAST->number,to_string(primaryExpAST->number));
        }
        else if (kind==FLOAT_NUMBER)
        {
            printNodeEdge(primaryExpAST,&primaryExpAST->kind,"FloatNumber");
            printNodeEdge(&primaryExpAST->kind,&primaryExpAST->floatNumber,to_string(primaryExpAST->floatNumber));
        }
    }
    else{
        if(kind==UNARYEXP){
            cout<<"Exp:{"<<endl;
            printExpAST((ExpAST*)primaryExpAST->Exp.get());
            cout<<"}"<<endl;
        }
        else if (kind==LVAL)
        {
            cout<<"LValR:{"<<endl;
            printLValRAST((LValRAST*)primaryExpAST->Lval.get());
            cout<<"}"<<endl;
        }
        else if (kind==NUMBER)
        {
            cout<<"Number:"<<primaryExpAST->number<<","<<endl;
        }
        else if (kind==FLOAT_NUMBER)
        {
            cout<<"FloatNumber:"<<primaryExpAST->floatNumber<<","<<endl;
        }
    }
}

void printConstDeclAST(ConstDeclAST*constDeclAST){
    // CONST Btype MulConstDef ';'
    if(astPrintGraphMode){
        printNodeEdge(constDeclAST,constDeclAST->Btype.get(),"Btype");
        printBtypeAST((BtypeAST*)constDeclAST->Btype.get());

        printNodeEdge(constDeclAST,constDeclAST->MulConstDef.get(),"MulConstDef");
        printMulConstDefAST((MulConstDefAST*)constDeclAST->MulConstDef.get());
    }
    else{
        cout<<"Btype:";
        printBtypeAST((BtypeAST*)constDeclAST->Btype.get());
        cout<<","<<endl;
        cout<<"MulConstDef:{"<<endl;
        printMulConstDefAST((MulConstDefAST*)constDeclAST->MulConstDef.get());
        cout<<"}"<<endl;
    }
}
void printVarDeclAST(VarDeclAST* VarDeclAST,FuncTypeAST * funcTypeAST){
    //MulVarDef ';'
    if(astPrintGraphMode){
        printNodeEdge(VarDeclAST,funcTypeAST,"Type");
        if(funcTypeAST->type==FUNCTYPE_INT)
            printNodeEdge(funcTypeAST,&funcTypeAST->type,"INT");
        else if(funcTypeAST->type==FUNCTYPE_FLOAT)
            printNodeEdge(funcTypeAST,&funcTypeAST->type,"FLOAT");
        else if(funcTypeAST->type==FUNCTYPE_VOID)
            printNodeEdge(funcTypeAST,&funcTypeAST->type,"VOID");

        printNodeEdge(VarDeclAST,VarDeclAST->MulVarDef.get(),"MulVarDef");
        printMulVarDefAST((MulVarDefAST*)VarDeclAST->MulVarDef.get());

    }
    else{
        if(funcTypeAST->type==FUNCTYPE_INT)
            cout<<"Type:INT,"<<endl;
        else if(funcTypeAST->type==FUNCTYPE_FLOAT)
            cout<<"Type:FLOAT,"<<endl;
        else if(funcTypeAST->type==FUNCTYPE_VOID)
            cout<<"Type:VOID,"<<endl;
        cout<<"MulVarDef:{"<<endl;
        printMulVarDefAST((MulVarDefAST*)VarDeclAST->MulVarDef.get());
        cout<<"}"<<endl;
    }
}
void printVarDeclAST(VarDeclAST* VarDeclAST){
    //MulVarDef ';'
    if(astPrintGraphMode){
        printNodeEdge(VarDeclAST,VarDeclAST->MulVarDef.get(),"MulVarDef");
        printMulVarDefAST((MulVarDefAST*)VarDeclAST->MulVarDef.get());

    }
    else{
        cout<<"MulVarDef:{"<<endl;
        printMulVarDefAST((MulVarDefAST*)VarDeclAST->MulVarDef.get());
        cout<<"}"<<endl;
    }
}
void printMulVarDefAST(MulVarDefAST*mulVarDefAST){
    // SinVarDef | MulVarDef ',' SinVarDef
    if(astPrintGraphMode){
        for(auto &sinVarDef:mulVarDefAST->SinValDef){
            printNodeEdge(mulVarDefAST,sinVarDef.get(),"SinVarDef");
            printSinVarDefAST((SinVarDefAST*)sinVarDef.get());
        }
    }
    else{
        for(auto &sinVarDef:mulVarDefAST->SinValDef){
            cout<<"SinValDef:{"<<endl;
            printSinVarDefAST((SinVarDefAST*)sinVarDef.get());
            cout<<"}"<<endl;
        }
    }
}
void printSinVarDefAST(SinVarDefAST* sinVarDefAST){
    //IDENT | IDENT '=' InitVal | IDENT ArrayDimen | IDENT ArrayDimen '=' ConstArrayInit
    uint32_t type = sinVarDefAST->type; 
    if(astPrintGraphMode){
        printNodeEdge(sinVarDefAST,&sinVarDefAST->ident,sinVarDefAST->ident);
        if(type==SINVARDEFAST_UIN){
            // 非数组且未初始化
        }
        else if(type==SINVARDEFAST_INI){
            printNodeEdge(sinVarDefAST,sinVarDefAST->InitVal.get(),"InitVal");
            printInitValAST((InitValAST*)sinVarDefAST->InitVal.get());
        }
        else if(type==SINVARDEFAST_UNI_ARR){
            printNodeEdge(sinVarDefAST,sinVarDefAST->dimen.get(),"ArrayDimen");
            printArrayDimenAST((ArrayDimenAST*)sinVarDefAST->dimen.get());
        }
        else if(type==SINVARDEFAST_INI_ARR){
            printNodeEdge(sinVarDefAST,sinVarDefAST->dimen.get(),"ArrayDimen");
            printArrayDimenAST((ArrayDimenAST*)sinVarDefAST->dimen.get());

            printNodeEdge(sinVarDefAST,sinVarDefAST->constInit.get(),"ConstArrayInit");
            printConstArrayInitAST((ConstArrayInitAST*)sinVarDefAST->constInit.get());
        }
    }
    else{
        cout<<"ident:"<<sinVarDefAST->ident<<","<<endl;
        if(type==SINVARDEFAST_UIN){
            //非数组且未初始化
        }
        else if(type==SINVARDEFAST_INI){
            cout<<"InitVal:{"<<endl;
            printInitValAST((InitValAST*)sinVarDefAST->InitVal.get());
            cout<<"}"<<endl;
        }
        else if(type==SINVARDEFAST_UNI_ARR){
            cout<<"ArrayDimen:{"<<endl;
            printArrayDimenAST((ArrayDimenAST*)sinVarDefAST->dimen.get());
            cout<<"}"<<endl;
        }
        else if(type==SINVARDEFAST_INI_ARR){
            cout<<"ArrayDimen:{"<<endl;
            printArrayDimenAST((ArrayDimenAST*)sinVarDefAST->dimen.get());
            cout<<"},"<<endl;
            cout<<"ConstArrayInit:{"<<endl;
            printConstArrayInitAST((ConstArrayInitAST*)sinVarDefAST->constInit.get());
            cout<<"}"<<endl;
        }
    }
}
void printInitValAST(InitValAST* initValAST){
    if(astPrintGraphMode){
        printNodeEdge(initValAST,initValAST->Exp.get(),"Exp");
        printExpAST((ExpAST*)initValAST->Exp.get());
    }
    else{
        cout<<"Exp:{"<<endl;
        printExpAST((ExpAST*)initValAST->Exp.get());
        cout<<"}"<<endl;
    }
}
void printMulConstDefAST(MulConstDefAST* mulConstDefAST){
    //SinConstDef | MulConstDef ',' SinConstDef
    if(astPrintGraphMode){
        for(auto &SinConstDef:mulConstDefAST->SinConstDef){
            printNodeEdge(mulConstDefAST,SinConstDef.get(),"SinConstDef");
            printSinConstDefAST((SinConstDefAST*)SinConstDef.get());
        }
        
    }
    else{
        for(auto &SinConstDef:mulConstDefAST->SinConstDef){
            cout<<"SinConstDef:{"<<endl;
            printSinConstDefAST((SinConstDefAST*)SinConstDef.get());
            cout<<"},"<<endl;
        }
    }
}
void printSinConstDefAST(SinConstDefAST* sinConstDefAST){
    //IDENT '=' ConstExp|IDENT ArrayDimen '=' ConstArrayInit
    int type=sinConstDefAST->type;
    if(astPrintGraphMode){
        printNodeEdge(sinConstDefAST,&sinConstDefAST->ident,sinConstDefAST->ident);
        if(type==SinConstDefAST::SINCONST_VAR){
            printNodeEdge(sinConstDefAST,sinConstDefAST->constExp.get(),"ConstExp");
            printConstExpAST((ConstExpAST*)sinConstDefAST->constExp.get());
        }
        else if(type==SinConstDefAST::SINCONST_ARRAY){
            printNodeEdge(sinConstDefAST,sinConstDefAST->arrayDimen.get(),"ArrayDimen");
            printArrayDimenAST((ArrayDimenAST*) sinConstDefAST->arrayDimen.get());

            printNodeEdge(sinConstDefAST,sinConstDefAST->constArrayInit.get(),"ConstArrayInit");
            printConstArrayInitAST((ConstArrayInitAST*)sinConstDefAST->constArrayInit.get());
        }
    }
    else{
        cout<<"ident:"<<sinConstDefAST->ident<<","<<endl;
        if(type==SinConstDefAST::SINCONST_VAR){
            cout<<"ConstExp:{"<<endl;
            printConstExpAST((ConstExpAST*)sinConstDefAST->constExp.get());
            cout<<"}"<<endl;
        }
        else if(type==SinConstDefAST::SINCONST_ARRAY){
            cout<<"ArrayDimen:{"<<endl;
            printArrayDimenAST((ArrayDimenAST*)sinConstDefAST->arrayDimen.get());
            cout<<"},"<<endl;
            cout<<"ConstArrayInit:{"<<endl;
            printConstArrayInitAST((ConstArrayInitAST*)sinConstDefAST->constArrayInit.get());
            cout<<"}"<<endl;
        }
    }
}

void printStmtAST(StmtAST* stmtAST){
    uint32_t type = stmtAST->type;
    switch (type)
    {
    // RETURN SinExp ';'
    case STMTAST_RET:{
        if(astPrintGraphMode){
            cout<<"node_"+to_string(idx++)<<" [label=RETURN];"<<endl;
            cout<<"node_"+to_string(node_idx[stmtAST])<<" -> "<<"node_"+to_string(idx-1)<<";"<<endl;
            printNodeEdge(stmtAST,stmtAST->SinExp.get(),"SinExp");
            printSinExpAST((SinExpAST*)stmtAST->SinExp.get());
        }
        else{
            cout<<"type:RETURN,"<<endl;
            cout<<"SinExp:{"<<endl;
            printSinExpAST((SinExpAST*)stmtAST->SinExp.get());
            cout<<"}"<<endl;
        }
        break;
    }
    // | IDENT '=' Exp ';'
    case STMTAST_LVA:{
        if(astPrintGraphMode){
            cout<<"node_"+to_string(idx++)<<" [label=ASSIGN];"<<endl;
            cout<<"node_"+to_string(node_idx[stmtAST])<<" -> "<<"node_"+to_string(idx-1)<<";"<<endl;
            printNodeEdge(stmtAST,&stmtAST->ident,stmtAST->ident);
            printNodeEdge(stmtAST,stmtAST->Exp.get(),"Exp");
            printExpAST((ExpAST*)stmtAST->Exp.get());
        }
        else{
            cout<<"type:ASSIGN,"<<endl;
            cout<<"ident:"<<stmtAST->ident<<","<<endl;
            cout<<"Exp:{"<<endl;
            printExpAST((ExpAST*)stmtAST->Exp.get());
            cout<<"}"<<endl;
        }
        break;
    }
    // | SinExp ';'
    case STMTAST_SINE:{
        if(astPrintGraphMode){
            cout<<"node_"+to_string(idx++)<<" [label=SINE_EXP];"<<endl;
            cout<<"node_"+to_string(node_idx[stmtAST])<<" -> "<<"node_"+to_string(idx-1)<<";"<<endl;

            printNodeEdge(stmtAST,stmtAST->SinExp.get(),"SinExp");
            printSinExpAST((SinExpAST*)stmtAST->SinExp.get());
        }
        else{
            cout<<"type:SINE_EXP,"<<endl;
            cout<<"SinExp:{"<<endl;
            printSinExpAST((SinExpAST*)stmtAST->SinExp.get());
            cout<<"}"<<endl;
        }
        break;
    }
    // | Block
    case STMTAST_BLO:{
        if(astPrintGraphMode){
            printNodeEdge(stmtAST,stmtAST->Block.get(),"Block");
            printBlockAST((BlockAST*)stmtAST->Block.get());
        }
        else{
            cout<<"Block:{"<<endl;
            printBlockAST((BlockAST*)stmtAST->Block.get());
            cout<<"}"<<endl;
        }
        break;
    }
    // | IfStmt
    case STMTAST_IF:{
        if(astPrintGraphMode){
            printNodeEdge(stmtAST,stmtAST->ifStmt.get(),"IfStmt");
            printIfStmtAST((IfStmtAST*)stmtAST->ifStmt.get());
        }
        else{
            cout<<"IfStmt:{"<<endl;
            printIfStmtAST((IfStmtAST*)stmtAST->ifStmt.get());
            cout<<"}"<<endl;
        }
        break;
    }
    // | WhileStmtHead
    case STMTAST_WHILE:{
        if(astPrintGraphMode){
            printNodeEdge(stmtAST,stmtAST->WhileHead.get(),"WhileStmtHead");
            printWhileStmtHeadAST((WhileStmtHeadAST*)stmtAST->WhileHead.get());
        }
        else{
            cout<<"WhileStmtHead:{"<<endl;
            printWhileStmtHeadAST((WhileStmtHeadAST*)stmtAST->WhileHead.get());
            cout<<"}"<<endl;
        }
        break;
    }
    // | InWhile
    case STMTAST_INWHILE:{
        if(astPrintGraphMode){
            printNodeEdge(stmtAST,stmtAST->InWhileStmt.get(),"InWhile");
            printInWhileAST((InWhileAST*)stmtAST->InWhileStmt.get());
        }
        else{
            cout<<"InWhile:";
            printInWhileAST((InWhileAST*)stmtAST->InWhileStmt.get());
            cout<<","<<endl;
        }
        break;
    }
    // |  IDENT ArrayDimen '=' Exp ';'
    case STMTAST_ARR:{
        if(astPrintGraphMode){
            printNodeEdge(stmtAST,&stmtAST->ident,stmtAST->ident);
            printNodeEdge(stmtAST,stmtAST->arrPara.get(),"ArrayDimen");
            printArrayDimenAST((ArrayDimenAST*) stmtAST->arrPara.get());

            printNodeEdge(stmtAST,stmtAST->Exp.get(),"Exp");
            printExpAST((ExpAST*)stmtAST->Exp.get());
        }
        else{
            cout<<"ident:"<<stmtAST->ident<<","<<endl;

            cout<<"ArrayDimen:{"<<endl;
            printArrayDimenAST((ArrayDimenAST*) stmtAST->arrPara.get());
            cout<<"},"<<endl;

            cout<<"Exp:{"<<endl;
            printExpAST((ExpAST*)stmtAST->Exp.get());
            cout<<"}"<<endl;
        }
        break;
    }
    default:
        break;
    }
}
void printSinExpAST(SinExpAST* sinExpAST){
    if(astPrintGraphMode){
        if(SINEXPAST_EXP==sinExpAST->type){
            printNodeEdge(sinExpAST,sinExpAST->Exp.get(),"Exp");
            printExpAST((ExpAST*)sinExpAST->Exp.get());
        }
    }
    else{
        if(SINEXPAST_EXP==sinExpAST->type){
        cout<<"Exp:{"<<endl;
        printExpAST((ExpAST*)sinExpAST->Exp.get());
        cout<<"}"<<endl;
        }
    }
}
void printIfStmtAST(IfStmtAST* ifStmtAST){
    // SinIfStmt | MultElseStmt
    uint32_t type= ifStmtAST->type;
    if(astPrintGraphMode){
        if (type==IFSTMT_SIN)
        {
            printNodeEdge(ifStmtAST,ifStmtAST->sinIfStmt.get(),"SinIfStmt");
            printSinIfStmtAST((SinIfStmtAST*)ifStmtAST->sinIfStmt.get());
        }
        else if(type==IFSTMT_MUL){
            printNodeEdge(ifStmtAST,ifStmtAST->multElseStmt.get(),"MultElseStmt");
            printMultElseStmtAST((MultElseStmtAST*)ifStmtAST->multElseStmt.get());
        }
        
    }
    else{
         if (type==IFSTMT_SIN)
        {
            cout<<"SinIfStmt:{"<<endl;
            printSinIfStmtAST((SinIfStmtAST*)ifStmtAST->sinIfStmt.get());
            cout<<"}"<<endl;
        }
        else if(type==IFSTMT_MUL){
            cout<<"MultElseStmt:{"<<endl;
            printMultElseStmtAST((MultElseStmtAST*)ifStmtAST->multElseStmt.get());
            cout<<"}"<<endl;
        }
    }
}
void printSinIfStmtAST(SinIfStmtAST* sinIfStmtAST){
    // IF '(' Exp ')' Stmt
    if(astPrintGraphMode){
        printNodeEdge(sinIfStmtAST,sinIfStmtAST->exp.get(),"\"Exp(if_condition)\"");
        printExpAST((ExpAST*)sinIfStmtAST->exp.get());

        printNodeEdge(sinIfStmtAST,sinIfStmtAST->stmt.get(),"Stmt");
        printStmtAST((StmtAST*)sinIfStmtAST->stmt.get());
    }
    else{
        cout<<"Exp(if_condition):{"<<endl;
        printExpAST((ExpAST*)sinIfStmtAST->exp.get());
        cout<<"}"<<endl;

        cout<<"Stmt:{"<<endl;
        printStmtAST((StmtAST*)sinIfStmtAST->stmt.get());
        cout<<"}"<<endl;
    }
}
void printMultElseStmtAST(MultElseStmtAST*multElseStmtAST){
    // IF '(' Exp ')' Stmt ELSE Stmt
    if(astPrintGraphMode){
        printNodeEdge(multElseStmtAST,multElseStmtAST->exp.get(),"\"Exp(if_condition)\"");
        printExpAST((ExpAST*)multElseStmtAST->exp.get());

        printNodeEdge(multElseStmtAST,multElseStmtAST->if_stmt.get(),"IF_Stmt");
        printStmtAST((StmtAST*)multElseStmtAST->if_stmt.get());

        printNodeEdge(multElseStmtAST,multElseStmtAST->else_stmt.get(),"ELSE_Stmt");
        printStmtAST((StmtAST*)multElseStmtAST->else_stmt.get());

    }
    else{
        cout<<"Exp(if_condition):{"<<endl;
         printExpAST((ExpAST*)multElseStmtAST->exp.get());
        cout<<"},"<<endl;

        cout<<"IF_Stmt:{"<<endl;
        printStmtAST((StmtAST*)multElseStmtAST->if_stmt.get());
        cout<<"},"<<endl;

        cout<<"ELSE_Stmt:{"<<endl;
        printStmtAST((StmtAST*)multElseStmtAST->else_stmt.get());
        cout<<"}"<<endl;
    }
}
void printWhileStmtHeadAST(WhileStmtHeadAST*whileStmtHeadAST){
    // WhileStmt
    if(astPrintGraphMode){
        printNodeEdge(whileStmtHeadAST,whileStmtHeadAST->WhileHead.get(),"WhileStmt");
        printWhileStmtAST((WhileStmtAST*)whileStmtHeadAST->WhileHead.get());
    }
    else{
        cout<<"WhileStmt:{"<<endl;
        printWhileStmtAST((WhileStmtAST*)whileStmtHeadAST->WhileHead.get());
        cout<<"}"<<endl;
    }
}
void printWhileStmtAST(WhileStmtAST*whileStmtAST){
    // WHILE '(' Exp ')' Stmt
    if(astPrintGraphMode){
        printNodeEdge(whileStmtAST,whileStmtAST->exp.get(),"\"Exp(while_condition)\"");
        printExpAST((ExpAST*)whileStmtAST->exp.get());

        printNodeEdge(whileStmtAST,whileStmtAST->stmt.get(),"\"Stmt(while_body)\"");
        printStmtAST((StmtAST*)whileStmtAST->stmt.get());
    }
    else{
        cout<<"Exp(while_condition):{"<<endl;
        printExpAST((ExpAST*)whileStmtAST->exp.get());
        cout<<"},"<<endl;

        cout<<"Stmt(while_body):{"<<endl;
        printStmtAST((StmtAST*)whileStmtAST->stmt.get());
        cout<<"}"<<endl;
    }
}
void printInWhileAST(InWhileAST* inWhileAST){
    // CONTINUE ';' | BREAK ';'
    int type=inWhileAST->type;
    if(astPrintGraphMode){
        if(type==STMTAST_CONTINUE){
            cout<<"node_"+to_string(idx++)<<" [label=CONTINUE];"<<endl;
            cout<<"node_"+to_string(node_idx[inWhileAST])<<" -> "<<"node_"+to_string(idx-1)<<";"<<endl;
        }
        else if(type==STMTAST_BREAK){
            cout<<"node_"+to_string(idx++)<<" [label=BREAK];"<<endl;
            cout<<"node_"+to_string(node_idx[inWhileAST])<<" -> "<<"node_"+to_string(idx-1)<<";"<<endl;
        }
    }
    else{
        if(type==STMTAST_CONTINUE){
            cout<<"CONTINUE";
        }
        else if(type==STMTAST_BREAK){
            cout<<"BREAK";
        }
    }
}

void printBtypeAST(BtypeAST* BtypeAST){
    int32_t type=BtypeAST->type;
    if(astPrintGraphMode){
        if(type==BTYPE_INT){
            printNodeEdge(BtypeAST,&BtypeAST->type,"INT");
        }
        else if(type==BTYPE_FLOAT){
            printNodeEdge(BtypeAST,&BtypeAST->type,"FLOAT");
        }
    }
    else{
        if(type==BTYPE_INT){
            cout<<"INT";
        }
        else if(type==BTYPE_FLOAT){
            cout<<"FLOAT";
        }
    }
}
void printRelOpAST(RelOpAST* relOpAST){
    uint32_t type=relOpAST->type;
    if(astPrintGraphMode){
        if(type==RELOPAST_L){
            printNodeEdge(relOpAST,&relOpAST->type,"\"<\"");
        }
        else if(type==RELOPAST_G){
            printNodeEdge(relOpAST,&relOpAST->type,"\">\"");
        }
        else if(type==RELOPAST_LE){
            printNodeEdge(relOpAST,&relOpAST->type,"\"<=\"");
        }
        else if(type==RELOPAST_GE){
            printNodeEdge(relOpAST,&relOpAST->type,"\">=\"");
        }
    }
    else{
        if(type==RELOPAST_L){
            cout<<"<";
        }
        else if(type==RELOPAST_G){
            cout<<">";
        }
        else if(type==RELOPAST_LE){
            cout<<"<=";
        }
        else if(type==RELOPAST_GE){
            cout<<">=";
        }
    }
}
void printEqOpAST(EqOpAST* eqOpAST){
    uint32_t type=eqOpAST->type;
    if(astPrintGraphMode){
        if(type==EQOPAST_EQ){
            printNodeEdge(eqOpAST,&eqOpAST->type,"\"==\"");
        }
        else if(type==EQOPAST_NE){
            printNodeEdge(eqOpAST,&eqOpAST->type,"\"!=\"");
        }
    }
    else{
        if(type==EQOPAST_EQ){
            cout<<"==";
        }
        else if(type==EQOPAST_NE){
            cout<<"!=";
        }
    }
}
void printMulOpAST(MulOpAST*mulOpAST){
    if(astPrintGraphMode){
        printNodeEdge(mulOpAST,&mulOpAST->op,"\""+string(1,mulOpAST->op)+"\"");
    }
    else{
        cout<<mulOpAST->op;
    }
}
void printAddOpAST(AddOpAST * addOpAST)
{
    if(astPrintGraphMode){
        // string str = std::string(1, addOpAST->op);
        printNodeEdge(addOpAST,&addOpAST->op,"\""+string(1, addOpAST->op)+"\"");
    }
    else{
        cout<<addOpAST->op;
    }
}
void printUnaryOpAST(UnaryOpAST*unaryOpAST){
    if(astPrintGraphMode){
        printNodeEdge(unaryOpAST,&unaryOpAST->op,"\""+string(1,unaryOpAST->op)+"\"");
    }
    else{
        cout<<unaryOpAST->op;
    }
}
void printFuncExpAST(FuncExpAST*funcExpAST){
    // IDENT '(' Params ')'
    if(astPrintGraphMode){
        printNodeEdge(funcExpAST,&funcExpAST->ident,funcExpAST->ident);
        printNodeEdge(funcExpAST,funcExpAST->para.get(),"Params");
        printParamsAST((ParamsAST*)funcExpAST->para.get());
    }
    else{
        cout<<"ident:"<<funcExpAST->ident<<","<<endl;
        cout<<"Params:{"<<endl;
        printParamsAST((ParamsAST*)funcExpAST->para.get());
        cout<<"}"<<endl;
    }
}
void printParamsAST(ParamsAST*paramsAST){
    // SinParams | Params ',' SinParams
    if(astPrintGraphMode){
        for(auto &sinParams:paramsAST->sinParams)
        {
            printNodeEdge(paramsAST,sinParams.get(),"SinParams");
            printSinParamsAST((SinParamsAST*)sinParams.get());
        }
    }
    else{
        for(auto &sinParams:paramsAST->sinParams)
        {
            cout<<"SinParams:{"<<endl;
            printSinParamsAST((SinParamsAST*)sinParams.get());
            cout<<"},"<<endl;
        }
    }
}
void printSinParamsAST(SinParamsAST*sinParamsAST){
    // Exp | | IDENT '[' ']' | IDENT '[' ']' ArrayDimen
    int type = sinParamsAST->type;
    if(astPrintGraphMode){
        if(type==1){
            printNodeEdge(sinParamsAST,sinParamsAST->exp.get(),"Exp");
            printExpAST((ExpAST*)sinParamsAST->exp.get());
        }
        else if(type==2){
            printNodeEdge(sinParamsAST,&sinParamsAST->ident,sinParamsAST->ident+"[]");
        }
        else if(type==3){
            printNodeEdge(sinParamsAST,&sinParamsAST->ident,sinParamsAST->ident);
            printNodeEdge(sinParamsAST,sinParamsAST->dimension.get(),"ArrayDimen");
            printArrayDimenAST((ArrayDimenAST*)sinParamsAST->dimension.get());
        }
    }
    else{
        if(type==1){
            cout<<"Exp:{"<<endl;
            printExpAST((ExpAST*)sinParamsAST->exp.get());
            cout<<"}"<<endl;
        }
        else if(type==2){
            cout<<"ident:"<<sinParamsAST->ident<<"[]"<<endl;
        }
        else if(type==3){
            cout<<"ident:"<<sinParamsAST->ident<<","<<endl;
            cout<<"ArrayDimen:{"<<endl;
            printArrayDimenAST((ArrayDimenAST*)sinParamsAST->dimension.get());
            cout<<"}"<<endl;
        }
    }
}
void printLValRAST(LValRAST*lValRAST){
    // IDENT | IDENT ArrayDimen
    int type = lValRAST->type;
    if(astPrintGraphMode){
        printNodeEdge(lValRAST,&lValRAST->ident,lValRAST->ident);
        if(type==LValRAST::IDENT){
        }
        else if(type==LValRAST::ARRAY){
            printNodeEdge(lValRAST,lValRAST->array.get(),"ArrayDimen");
            printArrayDimenAST((ArrayDimenAST*)lValRAST->array.get());
        }
    }
    else{
        cout<<"ident:"<<lValRAST->ident<<","<<endl;
        if(type==LValRAST::IDENT){

        }
        else if(type==LValRAST::ARRAY){
            cout<<"ArrayDimen:{"<<endl;
            printArrayDimenAST((ArrayDimenAST*)lValRAST->array.get());
            cout<<"}"<<endl;
        }
    }
}
