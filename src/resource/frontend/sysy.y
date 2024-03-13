%code requires {
  #include <memory>
  #include <string>
  #include <common.h>
  #include <string.h>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <string.h>
#include <common.h>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

%start CompUnit
// 定义 parser 函数和错误处理函数的附加参数
// 我们需要返回一个字符串作为 AST, 所以我们把附加参数定义成字符串的智能指针
// 解析完成后, 我们要手动修改这个参数, 把它设置成解析得到的字符串
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval 的定义, 我们把它定义成了一个联合体 (union)
// 因为 token 的值有的是字符串指针, 有的是整数
// 之前我们在 lexer 中用到的 str_val 和 int_val 就是在这里被定义的
// 至于为什么要用字符串指针而不直接用 string 或者 unique_ptr<string>?
// 请自行 STFW 在 union 里写一个带析构函数的类会出现什么情况
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}

// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN EQ LE GE NE AND OR CONST IF ELSE WHILE BREAK CONTINUE VOID
%token <str_val> IDENT
%token <int_val> INT_CONST 

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp
%type <ast_val> UnaryOp AddExp MulExp AddOp MulOp LOrExp LAndExp
%type <ast_val> EqExp EqOp RelExp RelOp Decl ConstDecl MulConstDef
%type <ast_val> SinConstDef ConstExp Btype MulBlockItem SinBlockItem LValL LValR
%type <ast_val> VarDecl SinVarDef MulVarDef InitVal SinExp 
%type <ast_val> IfStmt SinIfStmt MultElseStmt WhileStmt WhileStmtHead InWhile
%type <ast_val> FuncFParams ParaType SinFuncFParam 
%type <ast_val> FuncExp Params SinParams SinCompUnit MultCompUnit
%type <int_val> Number 

%%

// 开始符, CompUnit ::= FuncDef, 大括号后声明了解析完成后 parser 要做的事情
// 之前我们定义了 FuncDef 会返回一个 str_val, 也就是字符串指针
// 而 parser 一旦解析完 CompUnit, 就说明所有的 token 都被解析了, 即解析结束了
// 此时我们应该把 FuncDef 返回的结果收集起来, 作为 AST 传给调用 parser 的函数
// $1 指代规则里第一个符号的返回值, 也就是 FuncDef 的返回值
CompUnit
  : MultCompUnit {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->multCompUnit = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  } 
  ;

MultCompUnit
  : SinCompUnit {
    auto ast = new MultCompUnitAST();
    ast->sinCompUnit.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  } |  MultCompUnit SinCompUnit {
    auto ast = (MultCompUnitAST*)($1);
    ast->sinCompUnit.push_back(unique_ptr<BaseAST>($2));
    $$ = ast;
  }
  ;


SinCompUnit
  : ConstDecl {
    auto ast = new SinCompUnitAST();
    ast->constGlobal = unique_ptr<BaseAST>($1);
    ast->type = COMP_CON;
    $$ = ast;
  } | FuncType FuncDef {
    auto ast = new SinCompUnitAST();
    ast->funcType = unique_ptr<BaseAST>($1); 
    ast->funcDef = unique_ptr<BaseAST>($2);
    ast->type = COMP_FUNC;
    $$ = ast;
  } | FuncType VarDecl{
    auto ast = new SinCompUnitAST();
    ast->funcType = unique_ptr<BaseAST>($1); 
    ast->varGlobal = unique_ptr<BaseAST>($2);
    ast->type = COMP_VAR;
    $$ = ast;
  }
  ;

FuncDef
  : IDENT '(' FuncFParams')' Block {
    auto ast = new FuncDefAST();
    ast->ident = *unique_ptr<string>($1);
    ast->FuncFParams = unique_ptr<BaseAST>($3);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  ;

FuncFParams 
  : SinFuncFParam {
    auto ast = new FuncFParamsAST();
    ast->para.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  } | FuncFParams ',' SinFuncFParam {
    auto ast = (FuncFParamsAST *)($1);
    ast->para.push_back(unique_ptr<BaseAST>($3));
    $$ = ast;
  } | {
    auto ast = new FuncFParamsAST();
    $$ = ast;
  }
  ;

SinFuncFParam
  : ParaType IDENT {
    auto ast = new SinFuncFParamAST();
    ast->ident = *unique_ptr<string>($2);
    ast->ParaType = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  ;

// 同上, 不再解释
FuncType
  : INT {
    auto ast = new FuncTypeAST();
    ast->type = FUNCTYPE_INT;
    $$ = ast;
  } | VOID {
    auto ast = new FuncTypeAST();
    ast->type = FUNCTYPE_VOID;
    $$ = ast;
  }
  ;

ParaType 
  : INT {
    auto ast = new ParaTypeAST();
    $$ = ast;
  }
  ;

Decl
  : ConstDecl {
    auto ast       = new DeclAST();
    ast->ConstDecl = unique_ptr<BaseAST>($1);
    ast->type      = DECLAST_CON;
    $$             = ast;
  } | Btype VarDecl {
    auto ast       = new DeclAST();
    ast->VarDecl   = unique_ptr<BaseAST>($2);
    ast->type      = DECLAST_VAR;
    $$             = ast;
  }
  ;

ConstDecl
  : CONST Btype MulConstDef ';'{
    auto ast        = new ConstDeclAST();
    ast->Btype      = unique_ptr<BaseAST>($2);
    ast->MulConstDef= unique_ptr<BaseAST>($3);
    $$              = ast;
  }
  ;
//这里使用的是一个递归的方法，但是我们发现这个方法如果实际采用的话，可能会导致树的极度不平衡
//所以说助教说的Vec的方法应当想办法弄出来
//但是貌似也可以使用这种递归的方式，只是存储的时候就把SinConstDef存在一起就可以了
//也就是说MulConstDef不进行重新分配而是复用即可
MulConstDef
  : SinConstDef {
      auto ast = new MulConstDefAST();
      ast->SinConstDef.push_back(unique_ptr<BaseAST>($1));
      $$       = ast;
  } | MulConstDef ',' SinConstDef{
      auto ast = (MulConstDefAST*)($1);
      ast->SinConstDef.push_back(unique_ptr<BaseAST>($3));
      $$       = ast;
  }
  ;
//这个在语义分析阶段就用来写入常量表，而不输出任何汇编代码
SinConstDef
  : IDENT '=' ConstExp {
      auto ast      = new SinConstDefAST();
      ast->ident    = *unique_ptr<string>($1);
      ast->ConstExp = unique_ptr<BaseAST>($3);
      $$            = ast;
  }
  ;

VarDecl
  : MulVarDef ';'{
       auto ast     = new VarDeclAST();
       ast->MulVarDef = unique_ptr<BaseAST>($1);
       $$           = ast;
  }
  ;

MulVarDef
  : SinVarDef {
       auto ast     = new MulVarDefAST();
       ast->SinValDef.push_back(unique_ptr<BaseAST>($1));
       $$           = ast;
  } | MulVarDef ',' SinVarDef{
       auto ast     = (MulVarDefAST*)($1);
       ast->SinValDef.push_back(unique_ptr<BaseAST>($3));
       $$           = ast;
  }
  ;

SinVarDef
  : IDENT {
     auto ast   = new SinVarDefAST();
     ast->type  =  SINVARDEFAST_UIN;
     ast->ident = *unique_ptr<string>($1);
     $$         = ast;
  } | IDENT '=' InitVal {
    auto ast   = new SinVarDefAST();
    ast->type  =  SINVARDEFAST_INI;
    ast->ident = *unique_ptr<string>($1);
    ast->InitVal= unique_ptr<BaseAST>($3);
    $$         = ast;
  } 
  ;

InitVal
  : Exp {
    auto ast = new InitValAST();
    ast->Exp = unique_ptr<BaseAST>($1);
    $$       = ast;
  }
  ;


ConstExp 
  : Exp {
     auto ast = new ConstExpAST();
     ast->Exp = unique_ptr<BaseAST>($1);
     $$       = ast;
  }
  ;

Btype
  : INT {
    auto ast = new BtypeAST();
    ast->type = "int";
    $$        = ast;
  }
  ;

// FuncDef ::= FuncType IDENT '(' ')' Block;
// 我们这里可以直接写 '(' 和 ')', 因为之前在 lexer 里已经处理了单个字符的情况
// 解析完成后, 把这些符号的结果收集起来, 然后拼成一个新的字符串, 作为结果返回
// $$ 表示非终结符的返回值, 我们可以通过给这个符号赋值的方法来返回结果
// 你可能会问, FuncType, IDENT 之类的结果已经是字符串指针了
// 为什么还要用 unique_ptr 接住它们, 然后再解引用, 把它们拼成另一个字符串指针呢
// 因为所有的字符串指针都是我们 new 出来的, new 出来的内存一定要 delete
// 否则会发生内存泄漏, 而 unique_ptr 这种智能指针可以自动帮我们 delete
// 虽然此处你看不出用 unique_ptr 和手动 delete 的区别, 但当我们定义了 AST 之后
// 这种写法会省下很多内存管理的负担



Block
  : '{' MulBlockItem '}' {
    auto ast = new BlockAST();
    ast->MulBlockItem = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  ;
//这里使用递归的方式来描述多个，但是依旧使用vector来表示
MulBlockItem
  : SinBlockItem {
    auto ast = new MulBlockItemAST();
    ast->SinBlockItem.push_back(unique_ptr<BaseAST>($1));
    $$       = ast;
  } | MulBlockItem SinBlockItem{
    auto ast = (MulBlockItemAST*)($1);
    ast->SinBlockItem.push_back(unique_ptr<BaseAST>($2));
    $$       = ast;
  } | {
    auto ast = new MulBlockItemAST();
    $$       = ast;
  }
  ;

SinBlockItem 
  : Decl {
    auto ast = new SinBlockItemAST();
    ast->decl= unique_ptr<BaseAST>($1);
    ast->type= SINBLOCKITEM_DEC;
    $$ = ast;
  } | Stmt {
    auto ast = new SinBlockItemAST();
    ast->stmt= unique_ptr<BaseAST>($1);
    ast->type= SINBLOCKITEM_STM;
    $$ = ast;
  }
  ;

Stmt
  : RETURN SinExp ';' {
    auto ast = new StmtAST();
    ast->SinExp = unique_ptr<BaseAST>($2);
    ast->type= STMTAST_RET;
    $$       = ast;
  } | LValL '=' Exp ';' {
    auto ast = new StmtAST();
    ast->Exp = unique_ptr<BaseAST> ($3);
    ast->Lval= unique_ptr<BaseAST> ($1);
    ast->type= STMTAST_LVA;
    $$       = ast;
  } | SinExp ';' {
    auto ast = new StmtAST();
    ast->SinExp = unique_ptr<BaseAST>($1);
    ast->type = STMTAST_SINE;
    $$        = ast;
  } | Block {
    auto ast = new StmtAST();
    ast->Block = unique_ptr<BaseAST>($1);
    ast->type = STMTAST_BLO;
    $$        = ast;
  } | IfStmt {
    auto ast = new StmtAST();
    ast->IfHead = unique_ptr<BaseAST>($1);
    ast->type = STMTAST_IF;
    $$        = ast;
  } | WhileStmtHead {
    auto ast = new StmtAST();
    ast->WhileHead = unique_ptr<BaseAST>($1);
    ast->type = STMTAST_WHILE;
    $$        = ast;
  } | InWhile {
    auto ast = new StmtAST();
    ast->InWhileStmt = unique_ptr<BaseAST>($1);
    ast->type = STMTAST_INWHILE;
    $$        = ast;
  }
  ;

//if-else
IfStmt
  : SinIfStmt {
    auto ast =  new IfStmtAST();
    ast->if_head_stmt = unique_ptr<BaseAST> ($1);
    $$           = ast;
  } | MultElseStmt {
    auto ast = new IfStmtAST();
    ast->if_head_stmt = unique_ptr<BaseAST> ($1);
    $$          = ast;
  }
  ;

SinIfStmt
  : IF '(' Exp ')' Stmt { 
    auto ast = new SinIfStmtAST();
    ast->exp = unique_ptr<BaseAST> ($3);
    ast->stmt = unique_ptr<BaseAST> ($5);
    $$ = ast;
  }
  ;

MultElseStmt
  : IF '(' Exp ')' Stmt ELSE Stmt {
      auto ast = new MultElseStmtAST();
      ast->exp = unique_ptr<BaseAST> ($3);
      ast->if_stmt = unique_ptr<BaseAST> ($5);
      ast->else_stmt = unique_ptr<BaseAST> ($7);
      $$ = ast;
  } 
  ;

WhileStmtHead
  : WhileStmt {
    auto ast = new WhileStmtHeadAST();
    ast->WhileHead = unique_ptr<BaseAST> ($1);
    ast->type = STMTAST_WHILE;
  }
  ;

WhileStmt
  : WHILE '(' Exp ')' Stmt{
    auto ast = new WhileStmtAST();
    ast->exp = unique_ptr<BaseAST> ($3);
    ast->stmt = unique_ptr<BaseAST> ($5);
    $$ = ast;
  }
  ;

InWhile
  : CONTINUE ';'{
    auto ast = new InWhileAST();
    ast->type = STMTAST_CONTINUE;
    $$ = ast;
  } | BREAK ';'  {
    auto ast = new InWhileAST();
    ast->type = STMTAST_BREAK;
    $$ = ast;
  }

SinExp 
  : Exp {
    auto ast = new SinExpAST();
    ast->Exp = unique_ptr<BaseAST>($1);
    ast->type = SINEXPAST_EXP;
    $$ = ast;
  } | {
    auto ast = new SinExpAST();
    ast->type = SINEXPAST_NULL;
    $$ = ast;
  }
          
Exp
  : LOrExp {
      auto ast    = new ExpAST();
      ast->LOrExp = unique_ptr<BaseAST>($1);
      $$          = ast;
  }
  ;

LOrExp 
  : LAndExp {
      auto ast    = new LOrExpAST();
      ast->LAndExp= unique_ptr<BaseAST>($1);
      ast->type   = LOREXPAST_LAN;
      $$          = ast;
  } | LOrExp OR LAndExp {
      auto ast    = new LOrExpAST();
      ast->LOrExp = unique_ptr<BaseAST> ($1);
      ast->LAndExp= unique_ptr<BaseAST> ($3);
      ast->type   = LOREXPAST_LOR;
      $$          = ast;
  }
  ;

LAndExp
  : EqExp {
      auto ast    = new LAndExpAST();
      ast->EqExp  = unique_ptr<BaseAST>($1);
      ast->type   = LANDEXPAST_EQE;
      $$          = ast;
  } | LAndExp AND EqExp{
      auto ast    = new LAndExpAST();
      ast->EqExp  = unique_ptr<BaseAST>($3);
      ast->LAndExp= unique_ptr<BaseAST>($1);
      ast->type   = LANDEXPAST_LAN;
      $$          = ast;
  }
  ;

EqExp
  : RelExp {
      auto ast    = new EqExpAST();
      ast->RelExp = unique_ptr<BaseAST>($1);
      ast->type   = EQEXPAST_REL;
      $$          = ast;
  } | EqExp EqOp RelExp{
      auto ast    = new EqExpAST();
      ast->EqExp  = unique_ptr<BaseAST>($1);
      ast->EqOp   = unique_ptr<BaseAST>($2);
      ast->RelExp = unique_ptr<BaseAST>($3);
      ast->type   = EQEXPAST_EQE;
      $$          = ast;
  }
  ;

EqOp
  : EQ {
      auto ast  = new EqOpAST();
      ast->type = EQOPAST_EQ;
      $$        = ast;
  } | NE {
      auto ast  = new EqOpAST();
      ast->type = EQOPAST_NE;
      $$        = ast;
  }
  ;

RelExp
  : AddExp{
      auto ast    = new RelExpAST();
      ast->AddExp = unique_ptr<BaseAST>($1);
      ast->type   = RELEXPAST_ADD;
      $$          = ast;    
  } | RelExp RelOp AddExp{
      auto ast    = new RelExpAST();
      ast->AddExp  = unique_ptr<BaseAST>($3);
      ast->RelOp   = unique_ptr<BaseAST>($2);
      ast->RelExp = unique_ptr<BaseAST>($1);
      ast->type   = RELEXPAST_REL;
      $$          = ast;
  }
  ;

RelOp 
  : '<' {
    auto ast  = new RelOpAST();
    ast->type = RELOPAST_L;
    $$        = ast;
      } | '>' {
    auto ast  = new RelOpAST();
    ast->type = RELOPAST_G;
    $$        = ast;
      } | LE  {
    auto ast  = new RelOpAST();
    ast->type = RELOPAST_LE;
    $$        = ast;
      } | GE  {
    auto ast  = new RelOpAST();
    ast->type = RELOPAST_GE;
    $$        = ast;
  }
  ;

AddExp
  : MulExp {
    auto ast    = new AddExpAST();
    ast->MulExp = unique_ptr<BaseAST>($1);
    ast->type   = MULEXP;
    $$          = ast;
  } | AddExp AddOp MulExp {
    auto ast    = new AddExpAST();
    ast->AddExp = unique_ptr<BaseAST>($1);
    ast->AddOp  = unique_ptr<BaseAST>($2);
    ast->MulExp = unique_ptr<BaseAST>($3);
    ast->type   = ADDMUL;
    $$          = ast;
  }
  ;

MulExp 
  : UnaryExp {
    auto ast      = new MulExpAST();
    ast->UnaryExp = unique_ptr<BaseAST>($1);
    ast->type     = MULEXPAST_UNA;
    $$            = ast;
  } | MulExp MulOp UnaryExp{
    auto ast      = new MulExpAST();
    ast->MulExp   = unique_ptr<BaseAST>($1);
    ast->MulOp    = unique_ptr<BaseAST>($2);
    ast->UnaryExp = unique_ptr<BaseAST>($3);
    ast->type     = MULEXPAST_MUL;
    $$            = ast;
  }
  ;

LValL 
  : IDENT {
      auto ast   = new LValLAST();
      ast->ident = *unique_ptr<string>($1);
      $$ = ast;
  }
  ;

LValR
  : IDENT {
    auto ast   = new LValRAST();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  ;

PrimaryExp
  : '(' Exp ')' {
    auto ast  = new PrimaryExpAST();
    ast->kind = UNARYEXP;
    ast->Exp  = unique_ptr<BaseAST>($2);
    ast->number = 0;
    $$ = ast;
  } | LValR {
    auto ast = new PrimaryExpAST();
    ast->kind = LVAL;
    ast->Lval = unique_ptr<BaseAST>($1);
    $$ = ast;
  } | Number {
    auto ast  = new PrimaryExpAST();
    ast->kind = NUMBER;
    ast->number = $1;
    ast->Exp = nullptr;
    $$ = ast;
  }
  ;

Number
  : INT_CONST {
    $$ = $1;
  }
  ;

//using our function
UnaryExp 
  : PrimaryExp {
    auto ast        = new UnaryExpAST_P();
    ast->PrimaryExp = unique_ptr<BaseAST> ($1);
    $$ = ast; 
  } | UnaryOp UnaryExp {
    auto ast        = new UnaryExpAST_U();
    ast->UnaryOp    = unique_ptr<BaseAST>($1);
    ast->UnaryExp   = unique_ptr<BaseAST>($2);
    $$ = ast;
  } | FuncExp {
    auto ast        = new UnaryExpAST_F();
    ast->PrimaryExp = unique_ptr<BaseAST> ($1);
    $$ = ast; 
  }
  ;


FuncExp
  : IDENT '(' Params ')' {
    auto ast = new FuncExpAST();
    ast->ident = *unique_ptr<string>($1);
    ast->para  = unique_ptr<BaseAST>($3);
    $$       = ast;
  }
  ;


Params
  : SinParams {
    auto ast= new ParamsAST();
    ast->sinParams.push_back(unique_ptr<BaseAST>($1));
    $$ = ast;
  } | Params ',' SinParams {
    auto ast = (ParamsAST *)($1);
    ast->sinParams.push_back(unique_ptr<BaseAST>($3));
    $$ = ast;
  } | {
    auto ast = new ParamsAST();
    $$       = ast;
  }
  ;

SinParams 
  : Exp {
    auto ast = new SinParamsAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$  = ast;
  }
  ;

UnaryOp
  : '+' {
    auto ast = new UnaryOpAST();
    ast->op = '+';
    $$ = ast;
  } | '-' {
    auto ast = new UnaryOpAST();
    ast->op = '-';
    $$ = ast;
  } | '!' {
    auto ast = new UnaryOpAST();
    ast->op = '!';
    $$ = ast;
  }
  ;

AddOp
  : '+' {
    auto ast = new AddOpAST();
    ast->op  = '+';
    $$       = ast;
  } | '-' {
    auto ast = new AddOpAST();
    ast->op   = '-';
    $$       = ast;
  }
  ;

MulOp
  : '*' {
    auto ast = new MulOpAST();
    ast->op = '*';
    $$      = ast;
  } | '/' {
    auto ast  = new MulOpAST();
    ast->op   = '/';
    $$        = ast;
  } | '%' {
    auto ast = new MulOpAST();
    ast->op  = '%';
    $$       = ast;
  }
  ;
%%

// 定义错误处理函数, 其中第二个参数是错误信息
// parser 如果发生错误 (例如输入的程序出现了语法错误), 就会调用这个函数
//void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
//  cerr << "error: " << s << endl;
//}
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s) {
  
        extern int yylineno;    // defined and maintained in lex
        extern char *yytext;    // defined and maintained in lex
        int len=strlen(yytext);
        int i;
        char buf[512]={0};
        for (i=0;i<len;++i)
        {
            sprintf(buf,"%s%d ",buf,yytext[i]);
        }
        fprintf(stderr, "ERROR: %s at symbol '%s' on line %d\n", s, buf, yylineno);
}



