#include "BaseAST.h"
//无论是左值还是右值，都需要在全体作用域中寻找
class StmtAST : public BaseAST {
 public:
    //int num;
    std::unique_ptr<BaseAST> SinExp;
    std::unique_ptr<BaseAST> Exp;
    std::unique_ptr<BaseAST> Lval;
    std::unique_ptr<BaseAST> Block;
    std::unique_ptr<BaseAST> IfHead;
    uint32_t type;
    void Dump() const override {
      auto p = IdentTable;
      //auto& ValueTable = IdentTable->ConstTable;
      //auto& VarTable   = IdentTable->VarTable;
      
      string sign;
      switch(type) {
          case STMTAST_RET:
              SinExp->Dump(sign);
              cout << "  " << "ret " << sign << endl; 
              break;
          case STMTAST_LVA:{
            //cout << "parsing Lval" << endl;
            string sign1,sign2;
            Lval->Dump(sign1);
            Exp->Dump(sign2);
            int value = Exp->calc();
            //scope bianli
            while(p != nullptr) {
              auto &ValueTable = p->ConstTable;
              auto &VarTable   = p->VarTable; 
              int dep = p->level;
            if(ValueTable.find(sign1) != ValueTable.end()) {
              cerr << '"' << sign1 << "is a constant and can't be altered" << endl;
              exit(-1);
            }
            if(VarTable.find(sign1) != VarTable.end()){
              cout << "  " << "store " << sign2 << "," << "@"+sign1+"_"+to_string(dep) << endl;
              VarTable[sign1] = value;
              break;
              //exit(-1);
            }
              p = p->father;
            }
            //suo you scope zhong mei you zhe ge bian liang jiu shu chu cuo wu
            if(p == nullptr) {
              cerr << '"' << sign1 << "is not defined" << endl;
              exit(-1);
            }
            break;
          }
        case STMTAST_SINE: break;
        case STMTAST_BLO: Block->Dump(); break;
        case STMTAST_IF: IfHead->Dump(); break;
          default:
              assert(0);
      }
      
    }
    void Dump(string &sign) const override{
      
    }
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{return type;};
};

//非终结符不存在类
class IfStmtAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> if_head_stmt;
     void Dump() const override{
      //add your content
      //string sign;
      //cout<<"hello world"<<endl;
      if_head_stmt->Dump();
     }

    void Dump(string &sign) const override{}
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{return 0;}
};

//非终结符不存在类
class SinIfStmtAST : public BaseAST{
  public:
     std::unique_ptr<BaseAST> exp;
     std::unique_ptr<BaseAST> stmt;
     void Dump() const override{
      //add your content
      //调用calc接口
      //确定中间变量
      //中间变量的定义
      //must set end in beginning
      //if(end_br[alloc_now] == 1) return ;
      string sign1;
      exp->Dump(sign1);
      //输出对应中间变量担任名称
      if(alloc_now < 0) alloc_now = 0;
      if_flag_level[if_level] = alloc_now;
      //alloc_now++;
      cout<<"\tbr " << sign1 <<", %then"<<if_flag_level[if_level]<<", %end"<<if_flag_level[if_level]<<endl;
      cout<<endl;
      cout<<"%then"<<if_flag_level[if_level]<<":"<<endl;
      //完成分支指令同时转移到下一行，中间表达式用数字序号代替
      if_level++;
      int tmp = stmt->calc();
      //执行完if条件跳转，接下来执行stmt中内容
      stmt->Dump();
      //cout<<endl;
      if_level--;
      //we need judge before jumping end
      if(tmp != STMTAST_RET){
        if(tmp == STMTAST_BLO && ret_cnt == 0){
            cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
        if(ret_cnt > 0){
          ret_cnt = 0;
        } 
        if(tmp != STMTAST_BLO){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
      }
      
      //cout<<"tmp = "<<tmp;
        
      //end序列及其序号
      cout<<endl;
      cout<<"%end"<<if_flag_level[if_level]<<":"<<endl;

     }

    void Dump(string &sign) const override{}
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{return 20;}
};

class MultElseStmtAST : public BaseAST{
  public:
   std::unique_ptr<BaseAST> exp;
   std::unique_ptr<BaseAST> if_stmt;
   std::unique_ptr<BaseAST> else_stmt;
   void Dump() const override{
      //add your content
      //执行操作基本一直，只不过加了一个对else操作的处理
      //if(end_br[alloc_now] == 1) retrun ;
      if(alloc_now < 0) alloc_now = 0;
      string sign1;
      exp->Dump(sign1);
      if_flag_level[if_level] = alloc_now;
      //alloc_now++;
      cout<<"\tbr " << sign1 <<", %then"<<if_flag_level[if_level]<<", %else"<<if_flag_level[if_level]<<endl;
      cout<<endl;
      cout<<"%then"<<if_flag_level[if_level]<<":"<<endl;
      if_level++;
      //执行完if条件跳转，接下来执行if_stmt中内容
      int tmp1 = if_stmt->calc();
      if_stmt->Dump();
      if_level--;
      //cout<<endl;
      if(tmp1 != STMTAST_RET){
        if(tmp1 == STMTAST_BLO && ret_cnt == 0){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
        if(ret_cnt > 0){
          ret_cnt = 0;
        }
        if(tmp1 != STMTAST_BLO){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
      }
        


      //执行else_stmt序列的内容
      cout<<endl;
      cout<<"%else"<<if_flag_level[if_level]<<":"<<endl;
      if_level++;
      int tmp2 = else_stmt->calc();
      else_stmt->Dump(); 
      if_level--;
      //cout<<endl;
      if(tmp2 != STMTAST_RET){
        if(tmp2 == STMTAST_BLO && ret_cnt == 0){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
        if(ret_cnt > 0){
          ret_cnt = 0;
        }
        if(tmp2 != STMTAST_BLO){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
      }
        
      cout<<endl;
      cout<<"%end"<<if_flag_level[if_level]<<":"<<endl;
      
      //错误异常处理退出---不知咋处理，应该可以退出
      //end终止退出符

  }
    void Dump(string &sign) const override{}
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{ return 0;}

};

//遇到return不继续执行，直接返回
class If_return : BaseAST{
  public:
    std::unique_ptr<BaseAST> if_return_flag;
    void Dump() const override{
        if_return_flag->Dump();
    }

    void Dump(string &sign) const override{}
    void Dump(int value) const override{}
    void Dump(string &sign1,string &sign2,string &sign) const override{}
    [[nodiscard]] int calc() const override{return 7;}
    //返回7标识return标志
};