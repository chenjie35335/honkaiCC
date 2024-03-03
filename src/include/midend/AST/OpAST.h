#include "BaseAST.h"
//比较运算符
class RelOpAST : public BaseAST {
  public:
    uint32_t type;
    void Dump() const override{}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{}
    void Dump(string &sign1,string &sign2,string &sign)const override{
        alloc_now++;
        switch(type) {
          case RELOPAST_GE :
            cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "ge " << sign1 << ", " << sign2 << endl;
            break;
          case RELOPAST_LE :
            cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "le " << sign1 << ", " << sign2 << endl;
            break; 
          case RELOPAST_L :
            cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "lt " << sign1 << ", " << sign2 << endl;
            break; 
          case RELOPAST_G :
            cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "gt " << sign1 << ", " << sign2 << endl;
            break;
          default:
            assert(0);
        }
        sign = "%"+to_string(alloc_now);
    }
    [[nodiscard]] int calc() const override{return type;}
} 
;
//单目运算符
class UnaryOpAST : public BaseAST {
  public:
    char op;
    void Dump() const override {}
    void Dump(int value) const override{}
    void Dump(string &sign) const override {
      //在运算符处生成中间表示的语句
      //if(sign.at(0) == '%') {
        alloc_now++;
      //}
      switch(op) {
        case '+': break;
        case '-':
        cout << "  %" << (alloc_now) << ' '<< '=' << ' ' <<"sub " <<0<< ", " << sign << endl;
        sign = "%"+to_string(alloc_now);break;
        case '!':
        cout << "  %" << (alloc_now) << ' '<< '=' << ' ' <<"eq " <<sign<< ", " << 0 << endl;
        sign = "%"+to_string(alloc_now);break;
        default: assert(false);
      }
    }
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{
        return op;
    }
};
//加减运算符
class AddOpAST : public BaseAST {
  public:
    char op;
    void Dump() const override {}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{
        alloc_now++;
        switch(op) {
          case '+' :
            cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "add " << sign1 << ", " << sign2 << endl;
            break;
          case '-' :
            cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "sub " << sign1 << ", " << sign2 << endl;
            break; 
          default:
            assert(0);
        }
        sign = "%"+to_string(alloc_now);
    }
    [[nodiscard]] int calc() const override{return (int)op;}
};
//乘除运算符
class MulOpAST : public BaseAST {
  public:
    char op;
    void Dump() const override {}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override {
        alloc_now++;
        switch(op) {
         case '*' :
           cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "mul " << sign1 << ", " << sign2 << endl;
           break;
         case '/' :
           cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "div " << sign1 << ", " << sign2 << endl;
           break; 
         case '%' :
           cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "mod " << sign1 << ", " << sign2 << endl;
           break; 
         default:
           assert(0); 
        }   
        sign = "%"+to_string(alloc_now);
    }
    [[nodiscard]] int calc() const override{return op;}
};

class EqOpAST : public BaseAST {
  public:
    uint32_t type;
    void Dump() const override{}
    void Dump(int value) const override{}
    void Dump(string &sign) const override{}
    void Dump(string &sign1,string &sign2,string &sign)const override{
        alloc_now++;
        switch(type) {
          case EQOPAST_EQ :
            cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "eq " << sign1 << ", " << sign2 << endl;
            break;
          case EQOPAST_NE :
            cout << "  %" << (alloc_now) << ' '<< '=' << ' ' << "ne " << sign1 << ", " << sign2 << endl;
            break; 
          default:
            assert(0);
          }
        sign = "%"+to_string(alloc_now);
    }
    [[nodiscard]] int calc() const override{return (int)type;}
} 
;

//现在存在两个问题：一个问题是是否使用共用体来完成这个过程
//op其实不需要这么弄，因为可以直接比较来判断，但是如果是其他的话
//需要添加enum数据结构，可以考虑弄一下，然后使用共用体。
