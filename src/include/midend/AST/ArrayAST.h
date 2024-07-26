#include "BaseAST.h"
#ifndef ARRAYAST_LXH
#define ARRAYAST_LXH

class ConstArrayInitAST : public BaseAST{
  public:
    enum {
      INIT_NULL,
      INIT_MUL
    }kind;
    unique_ptr<BaseAST> multiArrayElement;
    int type;
    void generateGraph(string &sign, int &retType) const override;
    void generateGraphGlobal(string &sign, int &retType) const override;
};

class MultiArrayElementAST : public BaseAST{
  public:
    vector<unique_ptr<BaseAST>> sinArrayElement;
    void generateGraph(vector<RawValueP> &elem, int &retType) const override;
    void generateGraphGlobal(vector<RawValueP> &elem, int &retType) const override;
};

class SinArrayElementAST : public BaseAST {
    public:
      enum {
        ARELEM_AI,
        ARELEM_EX,
        FARELEM_EX
      }kind;
      unique_ptr<BaseAST> constExp;
      unique_ptr<BaseAST> constArrayInit;
      int type;
      void generateGraph(string &sign, int &retType) const override;
      void generateGraphGlobal(string &sign, int &retType) const override;
};

class ArrayDimenAST : public BaseAST {
  public:
    vector<unique_ptr<BaseAST>> sinArrayDimen;
    void generateGraph(vector<int> &dimens) const override;
    void generateGraph(vector<RawValueP> &dimens) const override;
};

class SinArrayDimenAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;
    void generateGraph(string &name) const override;
    ExpResult *Calculate() const override { return exp->Calculate();}
};

class ParaTypeAST : public BaseAST {
  public:
    int type;
    void generateGraph() const override {}
    int getType() const override { return type;}
};


#endif