#include "BaseAST.h"
#ifndef OPAST_STORMY
#define OPAST_STORMY
//比较运算符
class RelOpAST : public BaseAST {
  public:
    uint32_t type;
    int getType() const override {
      return type;
    }
} 
;
//单目运算符
class UnaryOpAST : public BaseAST {
  public:
    char op;
    int getType() const override{ return op; }
    void generateGraph(string &sign) const override;
};
//加减运算符
class AddOpAST : public BaseAST {
  public:
    char op;
    int getType() const override{ return (int)op;}
};
//乘除运算符
class MulOpAST : public BaseAST {
  public:
    char op;
    int getType() const override{return op;}
};

class EqOpAST : public BaseAST {
  public:
    uint32_t type;
    mutable unique_ptr<BaseAST> op;
    //void generateGraph(string &sign) const override;
    int getType() const override { return (int)type;}
} 
;

#endif
//现在存在两个问题：一个问题是是否使用共用体来完成这个过程
//op其实不需要这么弄，因为可以直接比较来判断，但是如果是其他的话
//需要添加enum数据结构，可以考虑弄一下，然后使用共用体。
