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
    [[nodiscard]] int calc() {
      return 0;
    };
    [[nodiscard]] float fcalc() {
      return 0.0;
    };
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
      [[nodiscard]]  ExpResult *Calculate() {
          switch(type) {
          case ARELEM_AI: 
              return constArrayInit->Calculate();
          case ARELEM_EX:{
              return constExp->Calculate();
          }
          default:  assert(0);
        }
      };
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

//arra para
class ArrParaAST : public BaseAST {
  public:
    vector<unique_ptr<BaseAST>> sinArrPara;
    void generateGraph() const override{}
};

class SinArrParaAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> exp;
    void generateGraph() const override{}
    [[nodiscard]]ExpResult *Calculate(){ return exp->Calculate();};
};

class ParaTypeAST : public BaseAST {
  public:
    int type;
    void generateGraph() const override {}
    ExpResult *Calculate() const override { return new ExpResult(type);}
};


#endif