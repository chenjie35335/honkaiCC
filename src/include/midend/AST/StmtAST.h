#include "BaseAST.h"
#ifndef STMTAST_STORMY
#define STMTAST_STORMY
//无论是左值还是右值，都需要在全体作用域中寻找
class StmtAST : public BaseAST {
 public:
    //int num;
    std::unique_ptr<BaseAST> SinExp;
    std::unique_ptr<BaseAST> Exp;
    std::unique_ptr<BaseAST> Lval;
    std::unique_ptr<BaseAST> Block;
    std::unique_ptr<BaseAST> ifStmt;
    std::unique_ptr<BaseAST> WhileHead;
    std::unique_ptr<BaseAST> InWhileStmt;
    uint32_t type;
    void Dump() const override {
      string sign;
      if(break_cnt == 0 && continue_cnt == 0){
      switch(type) {
          case STMTAST_RET:{
              SinExp->Dump(sign);
              cout << "  " << "ret " << sign << endl; 
              break;
          } 
          case STMTAST_LVA:{
            string sign1,sign2;
            Lval->Dump(sign1);
            Exp->Dump(sign2);
            int value = Exp->calc();
            IdentTable->VarAlter(sign1,sign2,value);
            break;
          }
        case STMTAST_SINE: {
            SinExp->Dump(sign);
            break;
        }
        case STMTAST_BLO: Block->Dump(); break;
        case STMTAST_IF: ifStmt->Dump(); break;
        case STMTAST_WHILE: WhileHead->Dump(); break;
        case STMTAST_INWHILE: InWhileStmt->Dump(); break;
        default:
              assert(0);
      } 
    }  
    }
    [[nodiscard]] int calc() const override{return type;};
    void generateGraph() const override;
};

//非终结符不存在类
class IfStmtAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> sinIfStmt;
    std::unique_ptr<BaseAST> multElseStmt;
    int type;
    void Dump() const override{
     switch(type) {
      case IFSTMT_SIN:
      sinIfStmt->Dump(); break;
      case IFSTMT_MUL:
      multElseStmt->Dump(); break;
      default:
      assert(0);
     }
    }
    void generateGraph() const override;
};

//非终结符不存在类
class SinIfStmtAST : public BaseAST{
  public:
     std::unique_ptr<BaseAST> exp;
     std::unique_ptr<BaseAST> stmt;
     void Dump() const override{
      string sign1;
      exp->Dump(sign1);
      //输出对应中间变量担任名称
      if(alloc_now < 0) alloc_now = 0;
      if_flag_level[if_level] = alloc_now;
      cout<<"\tbr " << sign1 <<", %then"<<if_flag_level[if_level]<<", %end"<<if_flag_level[if_level]<<endl;
      cout<<endl;
      cout<<"%then"<<if_flag_level[if_level]<<":"<<endl;
      //完成分支指令同时转移到下一行，中间表达式用数字序号代替
      if_level++;
      int tmp = stmt->calc();
      //执行完if条件跳转，接下来执行stmt中内容
      stmt->Dump();
      if_level--;
      //we need judge before jumping end
      if(tmp != STMTAST_RET){
        if(tmp == STMTAST_BLO && ret_cnt == 0 && break_cnt == 0 && continue_cnt == 0){
          //cout<<"continue_cnt"<<continue_cnt<<endl;
            cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
        if(ret_cnt > 0 || break_cnt > 0 || continue_cnt > 0){
          ret_cnt = 0;
          break_cnt = 0;
          continue_cnt = 0;
        } 
        if(tmp != STMTAST_BLO && tmp != STMTAST_BREAK && tmp != STMTAST_CONTINUE && tmp != STMTAST_INWHILE){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
      } 
      //end序列及其序号
      cout<<endl;
      cout<<"%end"<<if_flag_level[if_level]<<":"<<endl;
     }
     void generateGraph() const override;
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
      if(tmp1 != STMTAST_RET){
        if(tmp1 == STMTAST_BLO && ret_cnt == 0 && break_cnt == 0 && continue_cnt == 0){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
        if(ret_cnt > 0 || break_cnt > 0 || continue_cnt > 0){
          ret_cnt = 0;
          break_cnt = 0;
          continue_cnt = 0;
        }
        if(tmp1 != STMTAST_BLO && tmp1 != STMTAST_INWHILE && tmp1 != STMTAST_BREAK && tmp1 != STMTAST_CONTINUE){
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
        if(tmp2 == STMTAST_BLO && ret_cnt == 0 && break_cnt == 0 && continue_cnt == 0){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
        if(ret_cnt > 0 || break_cnt > 0 || continue_cnt > 0){
          ret_cnt = 0;
          break_cnt = 0;
          continue_cnt = 0;
        }
        if(tmp2 != STMTAST_BLO && tmp2 != STMTAST_INWHILE && tmp2 != STMTAST_BREAK && tmp2 != STMTAST_CONTINUE){
          cout<<"\tjump %end"<<if_flag_level[if_level]<<endl;
        }
      } 
      cout<<endl;
      cout<<"%end"<<if_flag_level[if_level]<<":"<<endl;
  }
  void generateGraph() const override; 
};

class WhileStmtHeadAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> WhileHead;
    //int type;
    void Dump() const override{
        WhileHead->Dump();
    }
  void generateGraph() const override; 
};

class WhileStmtAST : public BaseAST{
  public:
    std::unique_ptr<BaseAST> exp;
    std::unique_ptr<BaseAST> stmt;
    void Dump() const override{
        //首先生成while语句入口
        alloc_now++;
        int flag = alloc_now;
        record_while[while_level] = alloc_now;
        while_level++;
        cout<<"\tjump %while_entry"<<alloc_now<<endl;
        cout<<endl;
        cout<<"%while_entry"<<alloc_now<<":"<<endl;
        string sign1;
        exp->Dump(sign1);
        //处理cond while_body end
        cout<<"\tbr "<<sign1<<", %while_body"<<flag<<", %end"<<flag<<endl;
        cout<<endl;

        //while body
        cout<<"%while_body"<<flag<<":"<<endl;
        int tmp = stmt->calc();
        stmt->Dump();
        //cout<<"break_cnt + continue_cnt,ret_cnt: "<<break_cnt + continue_cnt << ret_cnt<<endl;
        if(tmp != STMTAST_RET){
          if(break_cnt + continue_cnt == 0 && ret_cnt == 0){
            cout<<"\tjump "<<"%while_entry"<<flag<<endl;
          }
          if(break_cnt + continue_cnt > 0 || ret_cnt > 0){
            break_cnt = 0;
            continue_cnt = 0;
            ret_cnt = 0;
          }
        }
        
        cout<<endl;
        //end
        //if(break_cnt + continue_cnt == 0)
        cout<<"%end"<<flag<<":"<<endl;
        while_level--;
      }
      void generateGraph() const override;
};

//cut stmt after break
class InWhileAST : public BaseAST{
  public:
    int type;
    void Dump() const override{
       switch(type){
        case STMTAST_BREAK:{
          //if(continue_cnt + break_cnt == 0)
          cout<<"\tjump "<<"%end"<<record_while[while_level - 1]<<endl;
          //while_level--;
          break_cnt++;
          //cout<<"break_cnt "<<break_cnt<<endl;
          break;
        }
        case STMTAST_CONTINUE:{
          cout<<"\tjump "<<"%while_entry"<<record_while[while_level - 1]<<endl;
          continue_cnt++;
          //while_level--;
          break;
        }
       }    
    }
};

#endif
