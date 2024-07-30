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

class RawArrayValue {
    public:
        RawValue* position;
        RawAggregate arrValue;
};

class IdentTableNode {
    public:
    //父节点
        IdentTableNode *father;
    //子节点
        IdentTableNode *child;
    //作用域层级
        int level;
    //常量变量表
        unordered_map<string,int> ConstTable;
        unordered_map<string,float>ConstFTable;
    //常量数组表
        //unordered_map<string,RawValue *> ConstArrTable;//这个分开存的方针肯定是对的但是要解决一些问题
        unordered_map<string, RawArrayValue *> ArrayTable;
    //变量表
        unordered_map<string,RawValue *> VarTable;
    //SSA表,存放存在单变量赋值的情况，创建一个对象，同时记录他是第几个被赋值的。
    //其实用int页可以，但是RawValue保存的信息更多些
        unordered_map<string, RawValue *> SSATable;
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
        RawValue * SearchVarR(const string &name);
    //查找当前常量表
        bool findValue(const string &ident){return ConstTable.find(ident) != ConstTable.end();}
        //浮点数常量
        bool findFValue(const string &ident){return ConstFTable.find(ident) != ConstFTable.end();}
    //查找当前变量表
        bool findVariable(const string &ident){return VarTable.find(ident) != VarTable.end();}
    //查找当前SSA表
        bool findSSATable(const string &ident){return SSATable.find(ident) != SSATable.end();}
    //插入变量
        void insertVar(const string &name, RawValue *&value){VarTable.insert(pair<string,RawValue *>(name,value));}
    //插入常量
        void insertValue(const string &name, int value){ConstTable.insert(pair<string,int>(name,value));}
        void insertFvalue(const string &name, float value){ConstFTable.insert(pair<string,float>(name,value));}
    //插入常量数组
        //void insertArr(const string &name,RawValue *&value) {ConstArrTable.insert(pair<string,RawValue *>(name,value));}
        void insertArr(const string &name,RawValue *&value) {
            RawArrayValue *r1 = new RawArrayValue();
            r1->position = value;
            ArrayTable.insert(pair<string,RawArrayValue*>(name,r1));
            //ArrayTable.at(name)->arrValue.elements.push_back(value);
            }
    //插入当前SSATable
        void insertSSA(const string &name, RawValue *&value) {SSATable.insert(pair<string, RawValue *>(name, value));}
    //查找当前ARRAY表
        //bool findConstArr(const string &ident) { return ConstArrTable.find(ident) != ConstArrTable.end();}
        bool findArr(const string &ident) {return ArrayTable.find(ident) != ArrayTable.end();}
        bool findConstArr(const string &ident) { return ArrayTable.find(ident) != ArrayTable.end();}
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
        RawValue *getVarR(const string &name);
        //插入变量值
        void insertVar(const string &name, RawValue *&value);
        //插入常数值
        void insertNumber(int number,RawValue *&value);
        void insertFnumber(float number,RawValue *&value);
        //判断常量是否重复定义
        void constMulDef(const string &ident);
        //判断变量是否重复定义
        void varMultDef(const string &ident);
        //插入常量
        void insertConst(const string &ident,int value);
        void insertFconst(const string &ident,float value);
        //插入常量数组
        void insertConstArr(const string &ident,RawValue *&value);
        //获取函数值
        RawFunction *getFunction(const string &ident);
        void insertArr(const string &ident, RawValue *r1);
        //清空中间变量表
        void clearMidVar() {MidVarTable.clear();}
};


#endif