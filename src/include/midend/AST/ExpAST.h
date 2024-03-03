#include "BaseAST.h"
class ExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> LOrExp;
    void Dump() const override {}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{
      //cout << "enter Exp" << endl;
        LOrExp->Dump(sign);
    }
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{
        return LOrExp->calc();
    }
};

class SinExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> Exp;
    uint32_t type;
    void Dump() const override {}
    void Dump(int value) const override {
    }
    void Dump(string &sign) const override {
      switch(type) {
          case SINEXPAST_EXP:
          Exp->Dump(sign); break;
          case SINEXPAST_NULL:
          sign = ""; break;
          default: 
          assert(0); 
      }
    }
    void Dump(string &sign,string &sign1,string &sign2) const override{}
    int calc() const override {return 0; }
};

class LOrExpAST : public BaseAST {
  public:
   std::unique_ptr<BaseAST> LAndExp;
   std::unique_ptr<BaseAST> LOrExp;
   uint32_t type;
   void Dump() const override{}
   void Dump(int value) const override{}
   void Dump(string &sign) const override{
        //cout << "enter lorexp" << endl;
        string sign1;
        string sign2;
        switch(type) {
          case LOREXPAST_LAN:
          //cout << "enter lorexp2" << endl;
            LAndExp->Dump(sign);break;
          case LOREXPAST_LOR:
            {
              LOrExp->Dump(sign1);
              LAndExp->Dump(sign2);
              Dump(sign1,sign2,sign);
              break;
            }
        }
   }
   void Dump(string &sign1,string &sign2,string &sign)const override{
        alloc_now++;
        sign = "%"+to_string(alloc_now);
        cout << "  "+sign << " = " << "or " << sign1 << ", " << sign2 << endl;
        alloc_now++;
        cout << "  %"+to_string(alloc_now) << " = " << "ne " << sign  << ", " << 0 << endl;
        sign = "%"+to_string(alloc_now);
   }
   [[nodiscard]] int calc() const override{
       int value = 0;
       switch(type) {
           case LOREXPAST_LAN:
               value = LAndExp->calc();break;
           case LOREXPAST_LOR:
           {
               int value1 = LOrExp->calc();
               int value2 = LAndExp->calc();
               value = value1 || value2;
               break;
           }
       }
       return value;
   }
}
;

class LAndExpAST : public BaseAST {
  public:
   std::unique_ptr<BaseAST> EqExp;
   std::unique_ptr<BaseAST> LAndExp;
   uint32_t type;
   void Dump() const override{}
   void Dump(int value) const override{}
   void Dump(string &sign) const override{
    string s1,s2;
    switch(type) {
      case LANDEXPAST_EQE:
      //cout << "enter landexp1" << endl;
          EqExp->Dump(sign);break;
      case LANDEXPAST_LAN:
          {
            //cout << "enter landexp1" << endl;
            LAndExp->Dump(s1);
            EqExp->Dump(s2);
            Dump(s1,s2,sign);
            break;
          }
      default: 
          assert(0);
    }

   }
   void Dump(string &sign1,string &sign2,string &sign)const override{
    alloc_now++;
    cout << "  %"+to_string(alloc_now) << " = " << "ne " << sign1 << ", " << 0 << endl;
    sign1 = "%"+to_string(alloc_now);
    alloc_now++;
    cout << "  %"+to_string(alloc_now) << " = " << "ne " << sign2 << ", " << 0 << endl;
    sign2 = "%"+to_string(alloc_now);
    alloc_now++;
    cout << "  %"+to_string(alloc_now) << " = " << "and " << sign1 << ", " << sign2 << endl;
    sign = "%"+to_string(alloc_now);
   }
   [[nodiscard]] int calc() const override{
       int value = 0;
       switch(type) {
           case LANDEXPAST_EQE:
               value = EqExp->calc();break;
           case LANDEXPAST_LAN:
           {
               int value1 = EqExp->calc();
               int value2 = LAndExp->calc();
               value = value1 && value2;
               break;
           }
       }
       return value;
   }
}
;

class EqExpAST : public BaseAST {
  public:
   std::unique_ptr<BaseAST> EqExp;
   std::unique_ptr<BaseAST> RelExp;
   std::unique_ptr<BaseAST> EqOp;
   uint32_t type;
   void Dump() const override{}
   void Dump(int value) const override{}
   void Dump(string &sign) const override{
     string s1,s2;
     switch(type) {
        case EQEXPAST_REL: {
          RelExp->Dump(sign);break;
        }
        case EQEXPAST_EQE: {
          EqExp->Dump(s1);
          RelExp->Dump(s2);
          EqOp->Dump(s1,s2,sign);
          break;
        }
        default:
          assert(0);
     }
   }
   void Dump(string &sign1,string &sign2,string &sign)const override{

   }
   [[nodiscard]] int calc() const override{
       int value = 0;
       switch(type) {
           case EQEXPAST_REL:
               value = RelExp->calc();break;
           case EQEXPAST_EQE:
           {
               int value1 = EqExp->calc();
               int value2 = RelExp->calc();
               int OpType = EqOp->calc();
               switch(OpType) {
                   case EQOPAST_EQ: value = value1 == value2; break;
                   case EQOPAST_NE: value = value1 != value2; break;
                   default: break;
               }
               break;
           }
       }
       return value;
   }
}
;


class RelExpAST : public BaseAST {
  public:
   std::unique_ptr<BaseAST> AddExp;
   std::unique_ptr<BaseAST> RelExp;
   std::unique_ptr<BaseAST> RelOp;
   uint32_t type;
   void Dump() const override{}
   void Dump(int value) const override{}
   void Dump(string &sign) const override{
      string s1,s2;
      switch(type) {
      case RELEXPAST_ADD: {
        AddExp->Dump(sign);break;
      }
      case RELEXPAST_REL: {
        RelExp->Dump(s1);
        AddExp->Dump(s2);
        RelOp->Dump(s1,s2,sign);
        break;
      }
      default:
        assert(0);
      }
   }
   void Dump(string &sign1,string &sign2,string &sign)const override{}
   [[nodiscard]] int calc() const override{
       int value;
       int value1,value2;
       switch(type) {
           case RELEXPAST_ADD: {
               value = AddExp->calc();break;
           }
           case RELEXPAST_REL: {
               value1 = RelExp->calc();
               value2 = AddExp->calc();
               int OpRel = RelOp->calc();
               switch(OpRel) {
                   case RELOPAST_GE: value = value1 >= value2; break;
                   case RELOPAST_LE: value = value1 <= value2; break;
                   case RELOPAST_G:  value = value1 > value2; break;
                   case RELOPAST_L:  value = value1 < value2; break;
                   default: assert(0);
               }
               break;
           }
           default:
               assert(0);
       }
       return value;
   }

}
;

class AddExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> MulExp;
    std::unique_ptr<BaseAST> AddExp;
    uint32_t type;
    std::unique_ptr<BaseAST> AddOp;
    void Dump() const override{}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{
      //cout << "enter AddExp" << endl;
      switch(type) {
        case MULEXP:  
            MulExp->Dump(sign);break;
        case ADDMUL:
        {
            string sign1;
            string sign2;
            AddExp->Dump(sign1);
            MulExp->Dump(sign2);
            AddOp->Dump(sign1,sign2,sign);
            break;
        }
        default:
            assert(0);
      }
    }
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{
        int value;
        switch(type) {
            case MULEXP:
                value = MulExp->calc();break;
            case ADDMUL:
            {
                int value1,value2;
                value1 = AddExp->calc();
                value2 = MulExp->calc();
                int OpAdd = AddOp->calc();
                switch(OpAdd) {
                    case '+' : value = value1 + value2; break;
                    case '-' : value = value1 - value2; break;
                    default: assert(0);
                }
                break;
            }
            default:
                assert(0);
        }
        return value;
    }
};

class MulExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> UnaryExp;
    std::unique_ptr<BaseAST> MulExp;
    uint32_t type;
    std::unique_ptr<BaseAST> MulOp;
    void Dump() const override{}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{
      switch(type) {
        case MULEXPAST_MUL: {
          string sign1;
          string sign2;
          MulExp->Dump(sign1);
          UnaryExp->Dump(sign2);
          MulOp->Dump(sign1,sign2,sign);
          break;
        }
        case MULEXPAST_UNA: UnaryExp->Dump(sign);break;
        default: assert(0);
      }
    }
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override {
        int value;
        switch(type) {
            case MULEXPAST_MUL: {
                int value1,value2;
                value1 = MulExp->calc();
                value2 = UnaryExp->calc();
                int OpMul = MulOp->calc();
                switch(OpMul) {
                    case '*': value = value1 * value2;break;
                    case '/': value = value1 / value2;break;
                    case '%': value = value1 % value2;break;
                    default: assert(0);
                }
                break;
            }
            case MULEXPAST_UNA: value = UnaryExp->calc();break;
            default: assert(0);
        }
        return value;
    }
};

class UnaryExpAST_P : public BaseAST {
  public:
  //UnaryExp第一种情况
    std::unique_ptr<BaseAST> PrimaryExp;
    void Dump() const override {}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{
      PrimaryExp->Dump(sign);
    }
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{
        return PrimaryExp->calc();
    }
};

class UnaryExpAST_U : public BaseAST {
  public:
  //UnaryExp的递归第二种情况
    std::unique_ptr<BaseAST> UnaryOp;
    std::unique_ptr<BaseAST> UnaryExp;
    void Dump() const override{}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{
        UnaryExp->Dump(sign);
        UnaryOp->Dump(sign);
    }
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{
        int value;
        int OpUnary = UnaryOp->calc();
        int value1  = UnaryExp->calc();
        switch(OpUnary) {
            case '+' : value = value1; break;
            case '-' : value = -value1; break;
            case '!' : value = !value1; break;
            default :  assert(0);
        }
        return value;
    }
};

class PrimaryExpAST : public BaseAST {
  public:
    std::unique_ptr<BaseAST> Exp;
    std::unique_ptr<BaseAST> Lval;
    int number;
  uint32_t kind;
    /*这里不带参数的不实现*/
  void Dump() const override{}
  void Dump(int value) const override{}
    /*如果遍历结果为常数，直接返回，如果不是，继续遍历*/
  void Dump(string &sign) const override{
      //if(kind == UNARYEXP)
        //  Exp->Dump(sign);
      //else 
        //  sign = to_string(number);
      switch(kind) {
        case UNARYEXP: Exp->Dump(sign); break;
        case LVAL: Lval->Dump(sign);break;
        case NUMBER:sign = to_string(number);break;
      }
  }
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{
        int value;
        switch(kind) {
            case UNARYEXP: value = Exp->calc(); break;
            case LVAL:     value = Lval->calc();break;
            case NUMBER:   value = number;break;
        }
      return value;
  }
};

