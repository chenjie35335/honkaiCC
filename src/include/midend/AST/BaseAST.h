#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <unordered_map>
#include <vector>
#include <ValueTable.h>
using namespace std;
#pragma once
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
  STMTAST_RET,
  STMTAST_LVA,
  STMTAST_SINE,
  STMTAST_BLO,
  STMTAST_IF,
  SinIfAST_BE,
  SinIFAST_NO,
  LVALAST_LEFT,
  LVALAST_RIGHT,
  SINEXPAST_EXP,
  SINEXPAST_NULL
}Kind;

extern int ScopeLevel;
extern IdentTableNode* IdentTable;
extern int end_br[100];
//extern unordered_map<string,int> ValueTable;
//extern unordered_map<string,int> VarTable;
static int alloc_now = -1;
static int if_flag_level[200] = {0};
static int if_level = 0;
static int ret_cnt = 0;
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void Dump() const = 0;//这个用来无返回值遍历
  virtual void Dump(string &sign) const = 0;//这个用来带有单个返回值的遍历
  virtual void Dump(string &sign1,string &sign2,string &sign) const = 0;
  //这个用来带有双目运算符的遍历
  virtual void Dump(int value) const = 0; // 这个用来传递整形变量
  [[nodiscard]] virtual int calc() const = 0;//计算表达式的值
};
// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> func_def;
  void Dump() const override {
    IdentTable = new IdentTableNode();
    ScopeLevel = 0;
    IdentTable->level = ScopeLevel;
    alloc_now = -1;
    func_def->Dump();
    delete IdentTable;
  }
  void Dump(int value) const override{};
  void Dump(string &sign) const override {}//这两个不需要在此处重载
  void Dump(string &sign1,string &sign2,string &sign) const override{}
  [[nodiscard]] int calc() const override{return 0;}
};

// FuncDef 也是 BaseAST

//这里就是返回值的问题，但是这里考虑可以把返回值设为string,直接将常数改为string返回就可以了



//对于OP类型的，如果是enum表示的type,返回type值，如果直接存储运算符，则返回运算符的值











