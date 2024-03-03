#include "BaseAST.h"

class DeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> ConstDecl;
    std::unique_ptr<BaseAST> VarDecl;
    uint32_t type;
    void Dump() const override {
      //cout<< "declinto" << endl; 
        switch(type) {
            case DECLAST_CON: ConstDecl->Dump(); break;
            case DECLAST_VAR: VarDecl->Dump(); break;
            default: assert(0);
        }
    }
    void Dump(string &sign) const override {}
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{return 0;}
};

class ConstDeclAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> Btype;
    std::unique_ptr<BaseAST> MulConstDef;
    void Dump() const override {
       MulConstDef->Dump();
    }
    void Dump(string &sign) const override {}
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{return 0;}
};

class BtypeAST : public BaseAST {
  public:
    string type;
    void Dump() const override {}
    void Dump(string &sign) const override {}
    void Dump(int value) const override{}
    void Dump(string &sign,string  &sign1,string &sign2) const override{}
    [[nodiscard]] int calc() const override{return 0;}
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
    void Dump(string &sign) const override {}
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{return 0;}
};

class SinConstDefAST : public BaseAST{
  public:
    string ident;
    unique_ptr<BaseAST>ConstExp;
    void Dump() const override{
      //cout<< "sinconstdef" << endl;
      auto &ValueTable = IdentTable->ConstTable;
      auto &VarTable   = IdentTable->VarTable;
      if(ValueTable.find(ident) != ValueTable.end()) {
        assert(0);
      } 
      int value = ConstExp->calc();
      ValueTable.insert(pair<string,int>(ident,value));
    }
    void Dump(string &sign) const override {} 
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{return 0;}
};

class VarDeclAST : public BaseAST {
public:
     unique_ptr <BaseAST> MulVarDef;
     void Dump() const override{
        MulVarDef->Dump();
     }
     void Dump(string & sign) const override {}
     void Dump(int value) const override{}
     void Dump(string &sign1, string &sign2,string &sign)const override{}
     [[nodiscard]] int calc() const override {return 0;}
};

class MulVarDefAST : public BaseAST {
public:
    vector <unique_ptr<BaseAST>> SinValDef;
    void Dump() const override{
      for(auto &sinValDef : SinValDef) {
          sinValDef->Dump();
      }
    }
    void Dump(string & sign) const override {}
    void Dump(int value) const override{}
    void Dump(string &sign1, string &sign2,string &sign)const override{}
    [[nodiscard]] int calc() const override {return 0;}
};

class SinVarDefAST : public BaseAST {
public:
    string ident;
    unique_ptr<BaseAST>InitVal;
    uint32_t type;
    void Dump() const override{
      //cout<< "sinconstdef" << endl;
      int value;
      auto &ValueTable = IdentTable->ConstTable;
      auto &VarTable = IdentTable->VarTable;
      int dep = IdentTable->level;
      if(ValueTable.find(ident) != ValueTable.end()){
          cerr << '"' << ident << '"' << " has been defined as constant" <<endl;
          exit(-1);
      }
       if(VarTable.find(ident) != VarTable.end()){
          cerr << '"' << ident << '"' << " redefined" <<endl;
          exit(-1);
        }
      cout << "  @"+ident+"_"+to_string(dep) <<" = " << "alloc i32" << endl;
      switch(type) {
        case SINVARDEFAST_UIN: break;
        case SINVARDEFAST_INI:
        {
          value = InitVal->calc();
          cout << "  store " << value<< ", " << "@"+ident+"_"+to_string(dep)<<endl;
          break;
        }
      }
      VarTable.insert(pair<string,int>(ident,value));
    }
    void Dump(int value) const override{}
    void Dump(string & sign) const override {}
    void Dump(string &sign1, string &sign2,string &sign)const override{}
    [[nodiscard]] int calc() const override {return 0;}
};

class InitValAST : public BaseAST {
public:
    unique_ptr<BaseAST>Exp;
    void Dump() const override{}
    void Dump(string & sign) const override {}
    void Dump(int value) const override{}
    void Dump(string & sign1, string & sign2,string & sign)const override{}
    [[nodiscard]] int calc() const override {return Exp->calc();}
};

class ConstExpAST : public BaseAST {
  public:
    unique_ptr<BaseAST>Exp;
    void Dump() const override{}
    void Dump(string &sign) const override {} 
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{
        return Exp->calc();
    }
};
