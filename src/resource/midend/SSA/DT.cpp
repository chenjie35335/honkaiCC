#include "../../../include/midend/IR/Function.h"
#include "../../../include/midend/IR/BasicBlock.h"
#include "../../../include/midend/IR/Programme.h"
#include "../../../include/midend/SSA/DT.h"
#include <assert.h>
#include <stack>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
using namespace std;
unordered_set<RawBasicBlockP> cfgloop;
unordered_set<RawBasicBlock*>visitbb;
void cal_cfgloop(const RawBasicBlockP &start,const RawBasicBlockP &end)
{
    cfgloop.clear();//清空上一条回边对应的循环的计算的结果
    cfgloop.insert(start);
    cfgloop.insert(end);
    stack<RawBasicBlockP> stack_bbs;
    //查找不经过end能到达start的所有节点
    stack_bbs.push(start);
    while (!stack_bbs.empty())
    {
        RawBasicBlockP pbb = stack_bbs.top();stack_bbs.pop();
        for(auto pre_bb:pbb->pbbs)
        {
            // cout<<pre_bb->name<<endl;
            if(cfgloop.count(pre_bb)==0)//未添加到cfgloop集合中
            {
                cfgloop.insert(pre_bb);//添加到cfgloop集合中
                stack_bbs.push(pre_bb);
            }
        }
    }
    
}
bool BackEdge(const RawBasicBlockP &start,const RawBasicBlockP &end)
{
    //判断end是否控制start
    stack<RawBasicBlockP> pbbs;
    pbbs.push(end);
    while (!pbbs.empty())
    {
        RawBasicBlockP pbb = pbbs.top(); pbbs.pop();
        if(pbb == start) //end控制start,start->end是回边
            return true;
        for(auto domainbb:pbb->domains)
            pbbs.push(domainbb);
    }
    return false;
}
void traversalDF(const RawFunctionP &func)
{
    auto &bbs = func->basicblock;
    cout<<"支配边界 {"<<endl;
    for(RawBasicBlockP bb : bbs)//遍历每一个基本块
    {
        cout<<bb->name<<"节点的支配边界为:{"<<endl;
        for(auto pbb:bb->df)
        {
            cout<<pbb->name<<endl;
        }
        cout<<'}'<<endl;
    }
    cout<<'}'<<endl;
}
void traversalCFG(const RawFunctionP &func)
{
    auto &bbs = func->basicblock;
    unordered_set<RawBasicBlockP>vbbs;//判断是否访问过
    stack<RawBasicBlockP>stack_bbs;
    stack_bbs.push(*bbs.begin());//初始化,将起始节点入栈
    cout<<"digraph G {"<<endl;
    while (!stack_bbs.empty())
    {
        //获取并弹出栈顶元素
        RawBasicBlockP pbb = stack_bbs.top();stack_bbs.pop();
        //判断是否遍历过该元素
        if(vbbs.find(pbb)==vbbs.end())//未遍历过该元素
        {
            vbbs.insert(pbb);//标记当前访问过的元素
            
            auto& fbbs = pbb->fbbs;
            for (RawBasicBlockP npbb : fbbs){//广度优先
                cout<<pbb->name<<" -> "<<npbb->name;
                if(BackEdge(pbb,npbb))//判断是否是回边
                {
                    cout<<'['<<"color=\"red\"];"<<endl;
                    cal_cfgloop(pbb,npbb);
                    // cout<<cfgloop.size()<<endl;
                    for(auto loop_node:cfgloop){
                        cout<<loop_node->name<<" [color=red];"<<endl;
                    }
                }
                else
                    cout<<';'<<endl;
                if(vbbs.find(npbb)==vbbs.end()){//未访问过添加到栈中
                    stack_bbs.push(npbb);
                }
            }
        }
    }
    cout<<'}'<<endl;
}
void traversalDT(const RawFunctionP &func)
{
    auto &bbs = func->basicblock;
    unordered_set<RawBasicBlockP>vbbs;//判断是否访问过
    stack<RawBasicBlockP>stack_bbs;
    stack_bbs.push(*bbs.begin());//初始化,将起始节点入栈
    cout<<"digraph G {"<<endl;
    while (!stack_bbs.empty())
    {
        //获取并弹出栈顶元素
        RawBasicBlockP pbb = stack_bbs.top();stack_bbs.pop();
        //判断是否遍历过该元素
        if(vbbs.find(pbb)==vbbs.end())//未遍历过该元素
        {
            vbbs.insert(pbb);//标记当前访问过的元素
            if(pbb->preDomainNode!=NULL)
                cout<<pbb->name<<" -> "<<pbb->preDomainNode->name<<endl;
            auto& fbbs = pbb->domains;
            for (RawBasicBlockP npbb : fbbs){//广度优先
                cout<<pbb->name<<" -> "<<npbb->name<<endl;
                if(vbbs.find(npbb)==vbbs.end()){//未访问过添加到栈中
                    stack_bbs.push(npbb);
                }
            }
        }
    }
    cout<<'}'<<endl;
}
void GeneratorDT(RawProgramme *&programme,int genDot)
{
    auto &funcs = programme->funcs;
    //cerr << "func.len:" << funcs.len << endl;
    for (RawFunctionP func : funcs)//遍历所有函数
    {
        init_bbs(func);
        auto &bbs = func->basicblock;
        if(bbs.size()>0){
            list<RawBasicBlock*> RPO;//逆后序遍历
            visitbb.clear();//清除访问
            cal_RPO((*bbs.begin()),RPO);
            RawBasicBlock * s_bb = *bbs.begin();
            computeDF(s_bb);
            if(genDot==1)
                traversalDF(func);
            else if((genDot==2)&&(strcmp(func->name,"main") == 0))
                traversalDT(func);//生成支配树图
            else if(genDot==3&&(strcmp(func->name,"main") == 0))
                traversalCFG(func);//生成CFG图
        }
    }
}
void init_bbs(const RawFunctionP &func)
{
    auto &bbs = func->basicblock;
    for (auto pbb : bbs)
    {
        //添加所有的基本块，初始化为未标记
        auto &preDomainNode = pbb->preDomainNode;
        auto &domains = pbb->domains;
        domains.clear();
        preDomainNode = nullptr;//初始化节点前驱为零
    }
    
}
//计算逆后序遍历顺序
void cal_RPO(RawBasicBlock* nowbb,list<RawBasicBlock*> &RPO){
    visitbb.insert(nowbb);
    for(auto neighbor:nowbb->fbbs){
        if(visitbb.find(neighbor)==visitbb.end()){//未访问
            cal_RPO(neighbor,RPO);
        }
    }
    cout<<nowbb->name<<endl;
    RPO.push_front(nowbb);
}
//判断A是否支配B,或者B的必经结点是否是A
bool AisdomB(const RawBasicBlockP A, const RawBasicBlockP B)
{
    RawBasicBlockP prebb = B;
    while(1)//循化查找前驱直到树的根节点
    {
        // cout<<prebb->name<<endl;
        if(prebb == NULL)//没有前驱
            return false;
        if (prebb == A)//找到B的必经节点A
            return true;
        prebb = prebb->preDomainNode;
    }
}
//计算支配边界
void computeDF(RawBasicBlock * & n)
{
    //初始化支配边界集合为空
    n->df.clear();
    // 遍历当前节点在控制流图的后继
    for(RawBasicBlockP y : n->fbbs)//该循环计算DF_local[n]
    {
        // n 节点的后继节点y
        if(y->preDomainNode != n)// idom(y) != n
            n->df.insert(y);
    }
    // 遍历当前节点n在支配树的后继c
    for(RawBasicBlock * c : n->domains)
    {
        computeDF(c);
        //计算 DF_up[c]
        for(auto w : c->df)//DF[c]中的每个元素 w
        {
            // n 不是 w的必经节点，或者 n==w
            if(n==w||(!AisdomB(n,w)))
                n->df.insert(w);
        }

    }
}