#ifndef STORMY_VALUETABLE
#define STORMY_VALUETABLE
#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <unordered_map>
#include <vector>
using namespace std;
class BaseAST;
class FuncTableCall;

enum{
    RET_VOID,
    RET_INT
}RET_TYPE;
/*
    这下改变一下策略：
    1、这里使用一个双向链表实现，当离开一个作用域的时候直接回收就行
    2、这里使用C++的普通指针，（主要rust还是有点难）
*/
/*
    1、对于作用域来说，如果是定义，只需要检测本作用域内是否有重复
    2、但是对于表达式赋值来说，需要做到
*/
class IdentTableNode {
    public:
        IdentTableNode *father; //父节点
        IdentTableNode *child; //子节点
        //IdentTableNode *child2;
        int level;
        unordered_map<string,int> ConstTable; //常量表
        unordered_map<string,int> VarTable;  //变量表
        //构造函数
        IdentTableNode() {
            father = nullptr;
            child  = nullptr;
            //child2 = nullptr;
        }
};

//这个FuncTable该如何处理？
/*
1、 FuncTableNode应该专指一个函数，应该有个hashmap用于指向FuncTableNode一个链表
2、 链表采用尾插法的方式实现，每个节点代表一次调用
*/

class FuncTable{
    public:
    unordered_map<string,bool> FuncTable; //bool表示函数是否存在返回值
    unordered_map<string,vector<int>> FuncIPara; //
    unordered_map<string,vector<int>> FuncRPara; //表示一个函数中的实参值,这里代表的是
    unordered_map<int,string> FuncRank;

     void init() {
        FuncTable["getch"] = true;
        FuncTable["getint"] = true;
        FuncTable["getarrary"] = true;
        FuncTable["putint"] = false;
        FuncTable["putch"] = false;
        FuncTable["putarrary"] = false;
        FuncTable["starttime"] = false;
        FuncTable["stoptime"] = false;
    }
};
#endif
