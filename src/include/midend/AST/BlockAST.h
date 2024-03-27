#include "BaseAST.h"

#ifndef BLOCKAST_STORMY
#define BLOCKAST_STORMY
//每个SinBlock生成一个TreeNode
class BlockAST : public BaseAST {
 public:
    std::unique_ptr<BaseAST> MulBlockItem;
    void generateGraph() const override;
};

class MulBlockItemAST : public BaseAST {
  public:
    vector <unique_ptr<BaseAST>> SinBlockItem;
    void generateGraph() const override;
};
//单个block生成一个作用域
class SinBlockItemAST : public BaseAST {
  public:
    unique_ptr<BaseAST> decl;
    unique_ptr<BaseAST> stmt;
    uint32_t type;
    void generateGraph() const override;
};

#endif