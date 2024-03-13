#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <unordered_map>
#include <vector>
#include "../ValueTable/ValueTable.h"
#include "../IR/IRGraph.h"
#include "../IR/ValueKind.h"
using namespace std;
#ifndef STORMY_BASEAST
#define STORMY_BASEAST
// 所有 AST 的基类
//这个是所有类别的标记
//如果是这样的，做出以下约定：
/*
  1. 每种分类必须以**AST开头，下划线后表示具体类型;
  2. 对于分类的名称，为了方便，除PrimaryExp外，需要改动
  3. 取用每种匹配方式的前三个字符的大写作为分类方式
*/
enum{
  UNARYEXP,
  LVAL,
  NUMBER,
  MULEXP,
  ADDMUL,
  DECLAST_CON,
  DECLAST_VAR,
  MULEXPAST_UNA,
  MULEXPAST_MUL,
  LOREXPAST_LAN,
  LOREXPAST_LOR,
  LANDEXPAST_EQE,
  LANDEXPAST_LAN,
  EQEXPAST_REL,
  EQEXPAST_EQE,
  EQOPAST_EQ,
  EQOPAST_NE,
  RELEXPAST_ADD,
  RELEXPAST_REL,
  RELOPAST_L,
  RELOPAST_G,
  RELOPAST_LE,
  RELOPAST_GE,
  SINBLOCKITEM_DEC,
  SINBLOCKITEM_STM,
  SINVARDEFAST_UIN,
  SINVARDEFAST_INI,
  SINVARDEFAST_FUNC,
  STMTAST_RET,
  STMTAST_LVA,
  STMTAST_SINE,
  STMTAST_BLO,
  STMTAST_IF,
  STMTAST_WHILE,
  STMTAST_BREAK,
  STMTAST_CONTINUE,
  STMTAST_INWHILE,
  SinIfAST_BE,
  SinIFAST_NO,
  LVALAST_LEFT,
  LVALAST_RIGHT,
  SINEXPAST_EXP,
  SINEXPAST_NULL,
  FUNC_SIN,
  FUNC_MUL,
  FUNC_EXP,
  COMP_FUNC,
  COMP_CON,
  COMP_VAR,
  FUNCTYPE_INT,
  FUNCTYPE_VOID,
  DECL_LOC,
  DECL_GLOB
}Kind;

extern int ScopeLevel;
extern IdentTableNode* IdentTable;
extern FuncTable funcTable;
extern int end_br[100];
extern int alloc_now;
extern int if_flag_level[200];
extern int if_level;
extern int ret_cnt;
extern int record_while[100];
extern int while_level;
extern int break_cnt;
extern int continue_cnt;
extern int ret_func;
extern int func_call_cnt;
extern int is_lva;
extern int rank_name;
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const {}//这个用来无返回值遍历
  virtual void Dump(string &sign) const {};//这个用来带有单个返回值的遍历
  virtual void Dump(string &sign1,string &sign2,string &sign) const{};
  //这个用来带有双目运算符的遍历
  virtual void Dump(string &sign,vector<string> &Para) const{};
  [[nodiscard]] virtual int calc() const {return 0;}//计算表达式的值
  virtual void Dump(int sign) const {}//这个用于函数时候判断参数
  virtual void generateGraph(RawProgramme &IR) const{}
  virtual void generateGraph(RawSlice &IR) const{}
  virtual void generateGraph(RawSlice &IR, string &sign) const{}
};

class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> multCompUnit;
  void Dump() const override {
    printf("decl @getint(): i32\n");
    printf("decl @getch(): i32\n");
    printf("decl @getarray(*i32): i32\n");
    printf("decl @putint(i32)\n");
    printf("decl @putch(i32)\n");
    printf("decl @putarray(i32, *i32)\n");
    printf("decl @starttime()\n");
    printf("decl @stoptime()\n");
    cout<<endl;
    //cout << "enter CompUnit" << endl;
    IdentTable = new IdentTableNode();
    ScopeLevel = 0;
    IdentTable->level = ScopeLevel;
    //alloc_now = -1;
    multCompUnit->Dump();
    delete IdentTable;
  }
  void generateGraph(RawProgramme &IR) const override;
};

// CompUnit 是 BaseAST
class MultCompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  vector<unique_ptr<BaseAST>> sinCompUnit;
  void Dump() const override {
    for(auto &sinComp : sinCompUnit) {
      //cout << "enter MultiCompUnit" << endl;
      sinComp->Dump();
    }
  }
  void generateGraph(RawSlice &IR) const override;
};

// FuncDef 也是 BaseAST
//这里就是返回值的问题，但是这里考虑可以把返回值设为string,直接将常数改为string返回就可以了
//对于OP类型的，如果是enum表示的type,返回type值，如果直接存储运算符，则返回运算符的值

class SinCompUnitAST : public BaseAST {
 public:
    unique_ptr<BaseAST> constGlobal;
    unique_ptr<BaseAST> funcDef;
    unique_ptr<BaseAST> varGlobal;
    unique_ptr<BaseAST> funcType;
    int type;
    void Dump() const override {
      switch(type){
        case COMP_FUNC: {
            int type = funcType->calc();
            funcDef->Dump(type);
            break;
        }
        case COMP_CON:
            constGlobal->Dump();
            break;
        case COMP_VAR:{
            varGlobal->Dump(DECL_GLOB);
            break;
        }
        default:
            assert(0);
      }
      
    }
    [[nodiscard]] int calc() const override{return type;}
    void generateGraph(RawSlice &IR) const override;
};

#endif








