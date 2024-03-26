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
extern int alloc_now;
enum
{
    RET_VOID,
    RET_INT,
    FIND_CONST,
    FIND_VAR
} TYPE;
/*
    这下改变一下策略：
    1、这里使用一个双向链表实现，当离开一个作用域的时候直接回收就行
    2、这里使用C++的普通指针，（主要rust还是有点难）
*/
/*
    1、对于作用域来说，如果是定义，只需要检测本作用域内是否有重复
    2、但是对于表达式赋值来说，需要做到
*/
class IdentTableNode
{
public:
    IdentTableNode *father; // 父节点
    IdentTableNode *child;  // 子节点
    // IdentTableNode *child2;
    int level;
    unordered_map<string, int> ConstTable; // 常量表
    unordered_map<string, int> VarTable;   // 变量表
    // 构造函数
    IdentTableNode()
    {
        this->father = nullptr;
        this->child = nullptr;
    }
    // 添加常量
    void addValue(const string &ident, int value);
    // 添加变量
    void addVariable(const string &ident, int value);
    // 查找常量
    bool findValue(const string &ident);
    // 查找变量
    bool findVariable(const string &ident);
    // 判断常量重复定义
    void ValueMultDef(const string &ident);
    // 判断变量是否已经定义为常量
    void VarContrdef(const string &ident);
    // 判断变量重复定义
    void VarMultDef(const string &ident);
    // 判断常量已被修改
    void ValueAlter(const string &ident);
    // 修改变量值
    void VarAlter(const string &ident, const string &sign2, int value);
    // 前端load常量和变量值
    void IdentSearch(const string &ident, string &sign);
    // 计算常量或变量值
    int IdentCalc(const string &ident);
    // 后端load常量和变量值
    void IdentSearch(const string &ident, string &sign,int &type);
};

// 这个FuncTable该如何处理？
/*
1、 FuncTableNode应该专指一个函数，应该有个hashmap用于指向FuncTableNode一个链表
2、 链表采用尾插法的方式实现，每个节点代表一次调用
*/

/*
    由于所有的函数均属于同一个作用域，所以可以使用以下的方式
    1、 只需要一个FuncTable来操作;
    2、 一个函数需要存储的信息如下：
    （1） 返回值类型
    （2） 参数类型表（这个是函数调用的时候来判断调用的函数是否合法的）
    （3） 参数名（这个可能需要弄，但是不一定）
    （3） 函数名（主键）
*/
class FuncTable
{
public:
    unordered_map<string, bool> FuncTable;        // bool表示函数是否存在返回值
    //unordered_map<string, vector<int>> FuncIPara; //
    //unordered_map<string, vector<int>> FuncRPara; // 表示一个函数中的实参值,这里代表的是

    void init()
    {
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
