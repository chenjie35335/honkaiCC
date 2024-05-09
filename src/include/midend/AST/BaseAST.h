#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <unordered_map>
#include <vector>
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
  DECLAST_ARR,
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
  SINVARDEFAST_UNI_ARR,
  SINVARDEFAST_INI_ARR,
  STMTAST_RET,
  STMTAST_LVA,
  STMTAST_SINE,
  STMTAST_BLO,
  STMTAST_IF,
  STMTAST_WHILE,
  STMTAST_BREAK,
  STMTAST_CONTINUE,
  STMTAST_INWHILE,
  STMTAST_ARR,
  SinIfAST_BE,
  SinIFAST_NO,
  TYPE_INT,
  TYPE_FLOAT,
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
  COMP_ARR,
  PARA_VAR,
  PARA_ARR_SIN,
  PARA_ARR_MUL,
  FUNCTYPE_INT,
  FUNCTYPE_VOID,
  FUNCTYPE_FLOAT,
  FUNCTYPE_POINTER_INT,
  DECL_LOC,
  DECL_GLOB,
  IFSTMT_SIN,
  IFSTMT_MUL
}Kind;
extern int alloc_now;
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void generateGraph(RawProgramme *&IR) const{}
  virtual void generateGraph() const{}
  virtual void generateGraph(string &sign) const{}
  virtual void generateGraph(int &retType) const{}
  virtual void generateGraph(string &sign,int &type) const{}
  virtual void generateGraph(vector<RawValueP> &params) const {}
  virtual void generateGraph(RawValueP &value) const {}
  virtual void generateGraphGlobal() const{}
  virtual void generateGraph(vector<int> &dimen) const {}
  virtual int calc() const { return 0; }
};

class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> multCompUnit;
  void generateGraph(RawProgramme *&IR) const override;
};

// CompUnit 是 BaseAST
class MultCompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  vector<unique_ptr<BaseAST>> sinCompUnit;
  void generateGraph() const override;
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
    void generateGraph() const override;
};

#endif








