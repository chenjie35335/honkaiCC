#ifndef STORMY_VALUETABLE
#define STORMY_VALUETABLE
#include <iostream>
#include <memory>
#include <string>
#include <cassert>
#include <unordered_map>
#include <vector>
using namespace std;
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
        int level;
        unordered_map<string,int> ConstTable; //常量表
        unordered_map<string,int> VarTable;  //变量表
        //构造函数 
        IdentTableNode() {
            father = nullptr;
            child  = nullptr;
        }  
};
#endif