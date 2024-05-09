#include "../../../include/midend/IR/Function.h"
#include "../../../include/midend/IR/BasicBlock.h"
#include "../../../include/midend/SSA/DT.h"
#include <assert.h>
#include <stack>
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
using namespace std;
vector<pair<RawBasicBlockP,nodeSet>>node_nodes;
unordered_map<RawBasicBlockP,bool>visit_bbs;
void traversalDF(const RawFunctionP &func)
{
    cout<<"支配边界 {"<<endl;
    for(int i =0 ;i<func->bbs.len;i++)//遍历每一个基本块
    {
        RawBasicBlockP bb = (RawBasicBlockP)func->bbs.buffer[i];
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
    // cout<<"digraph G {"<<endl;
    // for(int i =0 ;i<func->bbs.len;i++)//遍历每一个 基本块
    // {
    //     RawBasicBlockP bb = (RawBasicBlockP)func->bbs.buffer[i];
    //     for(int j = 0;j<bb->fbbs.len;j++)//遍历每个基本块的后继基本块
    //     {
    //         RawBasicBlockP fbb = (RawBasicBlockP)bb->fbbs.buffer[j];
    //         cout<<bb->name<<" -> "<<fbb->name<<endl;
    //     }
    // }
    // cout<<'}'<<endl;
    unordered_set<RawBasicBlockP>vbbs;//判断是否访问过
    stack<RawBasicBlockP>stack_bbs;
    stack_bbs.push((RawBasicBlockP)func->bbs.buffer[0]);//初始化,将起始节点入栈
    cout<<"digraph G {"<<endl;
    while (!stack_bbs.empty())
    {
        //获取并弹出栈顶元素
        RawBasicBlockP pbb = stack_bbs.top();stack_bbs.pop();
        //判断是否遍历过该元素
        if(vbbs.find(pbb)==vbbs.end())//未遍历过该元素
        {
            vbbs.insert(pbb);//标记当前访问过的元素
            RawSlice fbbs = pbb->fbbs;
            for (uint32_t i = 0; i < fbbs.len ; i++){//广度优先
                RawBasicBlockP npbb = (RawBasicBlockP)fbbs.buffer[i];
                cout<<pbb->name<<" -> "<<npbb->name<<endl;
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
    unordered_set<RawBasicBlockP>vbbs;//判断是否访问过
    stack<RawBasicBlockP>stack_bbs;
    stack_bbs.push((RawBasicBlockP)func->bbs.buffer[0]);//初始化,将起始节点入栈
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
            RawSlice fbbs = pbb->domains;
            for (uint32_t i = 0; i < fbbs.len ; i++){//广度优先
                RawBasicBlockP npbb = (RawBasicBlockP)fbbs.buffer[i];
                cout<<pbb->name<<" -> "<<npbb->name<<endl;
                if(vbbs.find(npbb)==vbbs.end()){//未访问过添加到栈中
                    stack_bbs.push(npbb);
                }
            }
        }
    }
    cout<<'}'<<endl;
}
void GeneratorDT(const RawSlice &funcs)
{
    for (int i = 0; i < funcs.len; i++)//遍历所有函数
    {
        RawFunctionP func = (RawFunctionP)funcs.buffer[i];
        init_bbs(func);//初始化该函数所有节点的访问标志
        // cout<<func->name<<"函数对应的节点数:"<<visit_bbs.size()<<endl;
        // cout<<func->name<<"函数对应的支配节点集合"<<endl;
        if(func->bbs.len>0){
            func_domain_nodes(func);
            direct_domain_nodes();
            RawBasicBlock * s_bb = (RawBasicBlock *)func->bbs.buffer[0];
            computeDF(s_bb);
            // traversalDF(func);
            // traversalDT(func);//生成支配树图
            // traversalCFG(func);//生成CFG图
        }
    }
}
void init_bbs(const RawFunctionP &func)
{
    node_nodes.clear();//清除所有节点的支配节点集合
    visit_bbs.clear();//清除
    for (int i = 0; i < func->bbs.len; i++)
    {
        //添加所有的基本块，初始化为未标记
        RawBasicBlockP pbb = (RawBasicBlockP)func->bbs.buffer[i];
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
    RawBasicBlockP s_bbs = (RawBasicBlockP)func->bbs.buffer[0];
    for (int i = 0; i < func->bbs.len; i++)//遍历所有节点
    {
        RawBasicBlockP pbb = (RawBasicBlockP)func->bbs.buffer[i];
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
            RawSlice fbbs = pbb->fbbs;
            for (uint32_t i = 0; i < fbbs.len ; i++){//广度优先
                RawBasicBlockP npbb = (RawBasicBlockP)fbbs.buffer[i];
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
        RawBasicBlockP pbb = node_nodes[0].first;
        nodeSet domainNodes = node_nodes[0].second;
        for(auto bb:node_nodes[0].second)//去除非直接直配的节点
        {
            if(check_bb_inOther(bb)){//同时被其他节点支配;
                domainNodes.erase(bb);
            }
        }
        RawSlice * domains = (RawSlice *)&pbb->domains;
        //为节点分配空间
        // cout<<pbb->name<<"支配节点分配的空间大小"<<domainNodes.size()<<endl;
        domains->buffer = (const void **)malloc(sizeof(const void *)*domainNodes.size());
        // cout<<pbb->name<<"支配的节点{"<<endl;
        for(RawBasicBlockP bb:domainNodes){
            domains->buffer[domains->len++]=(const void *)bb;
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
    for(int i = 0;i<n->fbbs.len;i++)//该循环计算DF_local[n]
    {
        // n 节点的后继节点y
        RawBasicBlockP y = (RawBasicBlockP)n->fbbs.buffer[i];
        if(y->preDomainNode != n)// idom(y) != n
            n->df.insert(y);
    }
    // 遍历当前节点n在支配树的后继c
    for(int i = 0;i<n->domains.len;i++)
    {
        RawBasicBlock * c = (RawBasicBlock *)n->domains.buffer[i];
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