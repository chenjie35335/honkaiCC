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
    RET_INT
} RET_TYPE;
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
        father = nullptr;
        child = nullptr;
    }
    // 添加常量
    void addValue(const string &ident, int value)
    {
        ConstTable.insert(pair<string, int>(ident, value));
    }
    // 添加变量
    void addVariable(const string &ident, int value)
    {
        VarTable.insert(pair<string, int>(ident, value));
    }
    // 查找常量
    bool findValue(const string &ident)
    {
        return ConstTable.find(ident) == ConstTable.end();
    }
    // 查找变量
    bool findVariable(const string &ident)
    {
        return VarTable.find(ident) == VarTable.end();
    }
    // 判断常量重复定义
    void ValueMultDef(const string &ident)
    {
        if (!findValue(ident))
        {
            cerr << '"' << ident << '"' << "has been defined" << endl;
            exit(-1);
        }
    }
    // 判断变量是否已经定义为常量
    void VarContrdef(const string &ident)
    {
        if (!findValue(ident))
        {
            cerr << '"' << ident << '"' << " has been defined as constant" << endl;
            exit(-1);
        }
    }
    // 判断变量重复定义
    void VarMultDef(const string &ident)
    {
        if (!findVariable(ident))
        {
            cerr << '"' << ident << '"' << " redefined" << endl;
            exit(-1);
        }
    }
    // 判断常量已被修改
    void ValueAlter(const string &ident)
    {
        if (!findValue(ident))
        {
            cerr << '"' << ident << " is a constant and can't be altered" << endl;
            exit(-1);
        }
    }
    // 修改变量值
    void VarAlter(const string &ident, const string &sign2, int value)
    {
        ValueAlter(ident);
        if (!findVariable(ident))
        {
            cout << "  "
                 << "store " << sign2 << ","
                 << "@" + ident + "_" + to_string(this->level) << endl;
            VarTable[ident] = value;
            return;
        }
        else if (!this->father)
        {
            cerr << '"' << ident << "is not defined" << endl;
            exit(-1);
        }
        else
        {
            this->father->VarAlter(ident, sign2, value);
        }
    }
    // load常量和变量值
    void IdentSearch(const string &ident, string &sign)
    {
        if (!findValue(ident))
        {
            int CalValue = ConstTable.at(ident);
            sign = to_string(CalValue);
            return;
        }
        else if (!findVariable(ident))
        {
            alloc_now++;
            sign = "%" + to_string(alloc_now);
            cout << "  " << sign << " = "
                 << "load "
                 << "@" + ident + "_" + to_string(this->level) << endl;
            return;
        }
        else if (!this->father)
        {
            cerr << "Error: " << '"' << ident << '"' << " is not defined" << endl;
            exit(-1);
        }
        else
        {
            this->father->IdentSearch(ident, sign);
        }
    }

    int IdentCalc(const string &ident)
    {
        if (!findValue(ident))
        {
            int CalValue = ConstTable.at(ident);
            return CalValue;
        }
        else if (!findVariable(ident))
        {
            return VarTable.at(ident);
        }
        else if (!this->father)
        {
            cerr << "Error: " << '"' << ident << '"' << " is not defined" << endl;
            exit(-1);
        }
        else
        {
           return this->father->IdentCalc(ident);
        }
    }
};

// 这个FuncTable该如何处理？
/*
1、 FuncTableNode应该专指一个函数，应该有个hashmap用于指向FuncTableNode一个链表
2、 链表采用尾插法的方式实现，每个节点代表一次调用
*/

class FuncTable
{
public:
    unordered_map<string, bool> FuncTable;        // bool表示函数是否存在返回值
    unordered_map<string, vector<int>> FuncIPara; //
    unordered_map<string, vector<int>> FuncRPara; // 表示一个函数中的实参值,这里代表的是

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
