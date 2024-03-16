#include "BaseAST.h"

#ifndef DECLAST_STORMY
#define DECLAST_STORMY
class DeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> ConstDecl;
    std::unique_ptr<BaseAST> VarDecl;
    uint32_t type;
    void Dump() const override {
      //cout<< "declinto" << endl; 
        switch(type) {
            case DECLAST_CON: ConstDecl->Dump(); break;
            case DECLAST_VAR: VarDecl->Dump(DECL_LOC); break;
            default: assert(0);
        }
    }
    void generateGraph(RawSlice &IR) const override;
};

class ConstDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> Btype;
    std::unique_ptr<BaseAST> MulConstDef;
    void Dump() const override {
       MulConstDef->Dump();
    }
};

class BtypeAST : public BaseAST {
  public:
    string type;
};
//这里使用mulConstDef递归嵌套的方式实现多个，而且使用vector存储可以提高树的平衡性
class MulConstDefAST : public BaseAST {
  public:
    vector <unique_ptr<BaseAST>> SinConstDef;
    void Dump() const override{
        for(auto &sinConstDef : SinConstDef) {
          sinConstDef->Dump();
        }
    }
};

class SinConstDefAST : public BaseAST{
  public:
    string ident;
    unique_ptr<BaseAST>ConstExp;
    void Dump() const override{
      IdentTable->ValueMultDef(ident);
      int value = ConstExp->calc();
      IdentTable->addValue(ident,value);
    }
};

class VarDeclAST : public BaseAST {
public:
     unique_ptr <BaseAST> MulVarDef;
     void Dump(int sign) const override{
        MulVarDef->Dump(sign);
     }
};

class MulVarDefAST : public BaseAST {
public:
    vector <unique_ptr<BaseAST>> SinValDef;
    void Dump(int sign) const override{
      for(auto &sinValDef : SinValDef) {
          sinValDef->Dump(sign);
      }
    }
};

class SinVarDefAST : public BaseAST {
public:
    string ident;
    unique_ptr<BaseAST>InitVal;
    unique_ptr<BaseAST>func_exp;
    uint32_t type;
    void Dump(int sign) const override{
      int value = 0;
      int dep = IdentTable->level;
      IdentTable->VarContrdef(ident);
      IdentTable->VarMultDef(ident);
      string sign1;
      switch(type) {
        //we need add type
        case SINVARDEFAST_UIN: {
          if(sign == DECL_GLOB) {
            cout << "global @"+ident+"_"+to_string(dep) <<" = " << "alloc i32," <<  " zeroinit" << endl;
          }
          else {
            cout << "  @"+ident+"_"+to_string(dep) <<" = " << "alloc i32" << endl;
          }
              break;
        }
        case SINVARDEFAST_INI:
        {
          value = InitVal->calc();
          if(sign == DECL_GLOB) {
            cout << "global @"+ident+"_"+to_string(dep) <<" = " << "alloc i32," <<  " "<< value << endl;  
            cout<<endl;
          }else {
          cout << "  @"+ident+"_"+to_string(dep) <<" = " << "alloc i32" << endl;
          InitVal->Dump(sign1);
          cout << "  store " << sign1<< ", " << "@"+ident+"_"+to_string(dep)<<endl;
          }
          break;
        }
        default: assert(0);
      }
      IdentTable->addVariable(ident,value);
    }
};

class InitValAST : public BaseAST {
public:
    unique_ptr<BaseAST>Exp;
    void Dump(string & sign) const override {
      Exp->Dump(sign);
    }
    [[nodiscard]] int calc() const override {return Exp->calc();}
};

class ConstExpAST : public BaseAST {
  public:
    unique_ptr<BaseAST>Exp;
    [[nodiscard]] int calc() const override{
        return Exp->calc();
    }
};

// global vare
class GlobalDeclAST : public BaseAST
{
  public:
    std::unique_ptr<BaseAST> global;
    // std::unique_ptr<BaseAST> mul;
    void Dump() const override
    {
      cout << "global"
          << " ";
      global->Dump();
      cout << endl;
    } 
};

#endif
