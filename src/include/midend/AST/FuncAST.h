<<<<<<< HEAD
#include "BaseAST.h"
#ifndef STORMY_FUNCAST
#define STORMY_FUNCAST
class FuncDefAST : public BaseAST
{
public:
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> FuncFParams;
  void generateGraph(int &retType) const override;
};

class FuncTypeAST : public BaseAST
{
public:
  int type;
  [[nodiscard]] int calc() const override
  {
    return type;
  }
  void generateGraph(int &retType) const override;
};

// function params

class FuncFParamsAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> para;
  void generateGraph() const override;
  //复合形式就是将单个形式进行拆分
};

class SinFuncFParamAST : public BaseAST
{
public:
  std::string ident;
  unique_ptr<BaseAST> paraType;
  unique_ptr<BaseAST> arrayDimen;
  int type;
  void generateGraph(int &index) const override;
  [[nodiscard]] int calc() {
    return 0;
  };
  [[nodiscard]] float fcalc() {
    return 0.0;
  };
};

#endif

=======
#include "BaseAST.h"
#ifndef STORMY_FUNCAST
#define STORMY_FUNCAST
class FuncDefAST : public BaseAST
{
public:
  std::string ident;
  std::unique_ptr<BaseAST> block;
  std::unique_ptr<BaseAST> FuncFParams;
  void generateGraph(int &retType) const override;
};

class FuncTypeAST : public BaseAST
{
public:
  int type;
  int getType() const override { return type;}
  void generateGraph(int &retType) const override;
};

// function params

class FuncFParamsAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> para;
  void generateGraph() const override;
};

class SinFuncFParamAST : public BaseAST
{
public:
  std::string ident;
  unique_ptr<BaseAST> paraType;
  unique_ptr<BaseAST> arrayDimen;
  int type;
  void generateGraph(int &index) const override;
};

#endif

>>>>>>> backold
