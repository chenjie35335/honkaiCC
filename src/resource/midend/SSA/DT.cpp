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
vector<pair<RawBasicBlockP,nodeSet>>node_nodes;
unordered_map<RawBasicBlockP,bool>visit_bbs;
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
            func_domain_nodes(func);
            direct_domain_nodes();
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
    node_nodes.clear();//清除所有节点的支配节点集合
    visit_bbs.clear();//清除
    for (RawBasicBlockP pbb : bbs)
    {
        //添加所有的基本块，初始化为未标记
        RawBasicBlockP * preDomainNode = (RawBasicBlockP *)&(pbb->preDomainNode);
        *preDomainNode = NULL;//初始化节点前驱为零
        visit_bbs[pbb]=false;
    }
    
}
void clearBbsFlag()
{
    for(auto& pbb:visit_bbs)
    {
        //将所有标志清零
        pbb.second = false;
    }
}
void func_domain_nodes(const RawFunctionP &func)
{
    auto &bbs = func->basicblock;
    RawBasicBlockP s_bbs = *bbs.begin();
    for (RawBasicBlockP pbb : bbs)//遍历所有节点
    {
        // cout<<pbb->name<<"支配的节点有{"<<endl;
        find_domain_nodes(s_bbs,pbb);//判断当前pbb节点所支配的节点(节点删除法)
        // cout<<'}'<<endl;
    }
}
void find_domain_nodes(const RawBasicBlockP &s_bbs,const RawBasicBlockP &delete_bbs)
{
    clearBbsFlag();//清除标记
    visit_bbs[delete_bbs] = true;//将删除的节点标志为已访问

    stack<RawBasicBlockP>stack_bbs;
    stack_bbs.push(s_bbs);//初始化,将起始节点入栈

    while (!stack_bbs.empty())
    {
        //获取并弹出栈顶元素
        RawBasicBlockP pbb = stack_bbs.top();stack_bbs.pop();
        //判断是否遍历过该元素
        if(visit_bbs[pbb]==false)//未遍历过该元素
        {
            // cout<<"当前访问的节点：{"<<pbb->name<<"}"<<endl;
            visit_bbs[pbb]=true;//标记当前访问过的元素
            auto &fbbs = pbb->fbbs;
            for (RawBasicBlockP npbb : fbbs){//广度优先
                if(!visit_bbs[npbb])//未访问过添加到栈中
                    stack_bbs.push(npbb);
            }
        }
    }
    nodeSet ns;
    // 未遍历的节点即是被pbb节点支配的节点
    for(auto pbb:visit_bbs)
    {
        if(!pbb.second)//未被访问的是支配的节点
        {
            // cout<<pbb.first->name<<endl;
            ns.insert(pbb.first);
        }
    }
    node_nodes.push_back(make_pair(delete_bbs,ns));
}
bool check_bb_inOther(const RawBasicBlockP &bb)
{
    for(int i = 1;i < node_nodes.size();i++)//遍历除自己外的每一个支配集
    {
        nodeSet nodes = node_nodes[i].second;
        if(nodes.find(bb)!= nodes.end())//找到元素
        {
            return true;
        }
    }
    return false;
}
void direct_domain_nodes()
{
    sort(node_nodes.begin(),node_nodes.end(),[](pair<RawBasicBlockP,nodeSet> &a,pair<RawBasicBlockP,nodeSet> &b){
        return a.second.size()>b.second.size();
    });
    while (!node_nodes.empty())
    {
        RawBasicBlock* pbb = (RawBasicBlock*)node_nodes[0].first;
        nodeSet domainNodes = node_nodes[0].second;
        for(auto bb:node_nodes[0].second)//去除非直接直配的节点
        {
            if(check_bb_inOther(bb)){//同时被其他节点支配;
                domainNodes.erase(bb);
            }
        }
        auto &domains = pbb->domains;
        //为节点分配空间
        // cout<<pbb->name<<"支配节点分配的空间大小"<<domainNodes.size()<<endl;
        // cout<<pbb->name<<"支配的节点{"<<endl;
        for(RawBasicBlockP bb : domainNodes){
            auto bbData = (RawBasicBlock *) bb;
            domains.push_back(bbData);
            // cout<<bb->name<<endl;
            RawBasicBlockP * preDomainNode = (RawBasicBlockP *)&(bb->preDomainNode);
            *preDomainNode = pbb;
        }
        // cout<<'}'<<endl;
        node_nodes.erase(node_nodes.begin());
    }
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