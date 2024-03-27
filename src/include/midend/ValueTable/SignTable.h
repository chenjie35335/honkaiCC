#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <unordered_map>
#include <vector>
#include "../IR/IRGraph.h"
using namespace std;
//这里将前面所有符号表打包，成为一个整体的符号表
#ifndef SIGNTABLE_STORMY
#define SIGNTABLE_STORMY
class BaseAST;
class FuncTableCall;

class IdentTableNode {
    public:
    //父节点
        IdentTableNode *father;
    //子节点
        IdentTableNode *child;
    //作用域层级
        int level;
    //常量表
        unordered_map<string,int> ConstTable;
    //变量表
        unordered_map<string,RawValue *> VarTable;
    //构造函数
        IdentTableNode() {
            this->father = nullptr;
            this->child = nullptr;
            this->level = 0;
        }
    //带参数的构造函数
        IdentTableNode(IdentTableNode *father,int level) {
            this->father = father;
            this->child = nullptr;
            this->level = level;
        }
    //查找变量左值
        RawValue *SearchVarL(string &name);
    //查找变量右值
        RawValue * SearchVarR(string &name);
    //查找当前常量表
        bool findValue(const string &ident){return ConstTable.find(ident) != ConstTable.end();}
    //查找当前变量表
        bool findVariable(const string &ident){return VarTable.find(ident) != VarTable.end();}
    //插入变量
        void insertVar(string &name, RawValue *&value){VarTable.insert(pair<string,RawValue *>(name,value));}
    //插入常量
        void insertValue(string &name, int value){ConstTable.insert(pair<string,int>(name,value));}
};

//SignTable定义（仅用于中端）目前来看，BasicBlock不需要添加
class SignTable {
    public:
        //表示当前可分配的作用域的
        int tempScope;
        //作用域表
        IdentTableNode * IdentTable;
        //临时变量表
        unordered_map <string,RawValue *> MidVarTable;
        //函数表
        unordered_map<string, RawFunction *> FuncTable;
        //构造函数
        SignTable() {
            this->tempScope = 0;
            this->IdentTable = new IdentTableNode();
            this->MidVarTable = unordered_map<string, RawValue*>();
            this->FuncTable = unordered_map<string, RawFunction*>();
        }
        //插入函数
        void insertFunction(const string &name, RawFunction *&function);
        //变量表创建子节点并进入子节点
        void identForward();
        //向父节点回退
        void identBackward();
        //删除子节点并返回父节点
        void deleteBackward();
        //获取中间值
        RawValue *getMidVar(string &name);
        //插入中间变量
        void insertMidVar(string &name, RawValue* &value);
        //获取变量左值
        RawValue *getVarL(string &name);
        //获取变量右值
        RawValue *getVarR(string &name);
        //插入变量值
        void insertVar(string &name, RawValue *&value);
        //插入常数值
        void insertNumber(int number,RawValue *&value);
};
#endif