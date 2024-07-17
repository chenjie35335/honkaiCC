#include <map>
#include <list>
#include <vector>
#include <stack>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <iostream>
#include <unordered_set>
#include "../../../include/midend/Optimizer/OptimizeLoop.h"
//判断是否是循环不变量(常数)
bool isInVar(RawValue * value)
{
    switch (value->value.tag)
    {
        //整型常量
        case RVT_INTEGER:
        //浮点常量
        case RVT_FLOAT:
            return true;
        default:
            break;
    }
    return false;//在不变量集合中
}
//判断从start基本块到end基本块的边是否是回边
bool isBackEdge(const RawBasicBlockP &start,const RawBasicBlockP &end)
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
//计算start->end回边对应的循环的节点集合
void cal_loop(RawBasicBlock * &start,RawBasicBlock * &end,unordered_set<RawBasicBlock *> &loop)
{
    loop.insert(start);
    loop.insert(end);
    stack<RawBasicBlockP> stack_bbs;
    //查找不经过end能到达start的所有节点
    stack_bbs.push(start);
    while (!stack_bbs.empty())
    {
        RawBasicBlockP pbb = stack_bbs.top();stack_bbs.pop();
        for(auto pre_bb:pbb->pbbs)
        {
            if(loop.count(pre_bb)==0)//未添加到loop集合中
            {
                loop.insert(pre_bb);//添加到loop集合中
                stack_bbs.push(pre_bb);
            }
        }
    }
    
}
//计算循环中的所有value
void cal_loopValues(Loop * &loop)
{
    for(auto bb:loop->body)
    {
        for(auto value:bb->inst)
        {
            loop->loopValues[value] = bb;
        }
    }
}
//计算所有的出口节点
void cal_exitNodes(Loop * loop){
    for(auto bb:loop->body){
        for(auto fbb:bb->fbbs){
            if(loop->body.count(fbb)==0)//存在后继节点在循环外
            {
                loop->exitNode.insert(bb);
                break;
            }
        }
    }
    // cout<<"exitNode:"<<endl;
    // for(auto bb :loop->exitNode){

    //     cout<<bb->name<<endl;
    // }
}
//计算循环不变量
void cal_inVar(Loop * &loop){
    //所有代码初始化为未标记
    unordered_map<RawValue *,bool> v_values;//未标记的value
    int numNotInVar = loop->loopValues.size();//初始时所有value都不是不变量
    int last_numNotInVar = -1;
    for(auto value:loop->loopValues){//初始化
        if(isInVar(value.first)){//常数
            loop->inVar.push_back(value.first);
            v_values[value.first] = true;//标记为不变量
            numNotInVar--;
        }
        else
            v_values[value.first] = false;
    }

    while(1){
        //判断不变量集合是否有变化
        if(last_numNotInVar == numNotInVar)
            break;
        else
            last_numNotInVar = numNotInVar;
        //判断剩余的
        for(auto value:v_values)
        {
            if(value.second==false)//所有未标记的value
            {
                switch (value.first->value.tag)
                {
                case RVT_LOAD:{
                    if(v_values[(RawValue *)value.first->value.load.src]){
                        v_values[value.first]=true;
                    }
                    break;
                }
                case RVT_STORE:{
                    if(v_values[(RawValue *)value.first->value.store.value]){
                       v_values[value.first]=true;
                    }
                    break;
                }
                case RVT_BINARY:{
                    RawValue * lhs = (RawValue *)value.first->value.binary.lhs;
                    RawValue * rhs = (RawValue *)value.first->value.binary.rhs;
                    if(v_values[lhs]&&v_values[rhs]){
                        v_values[value.first]=true;
                    }
                    break;
                }
                case RVT_ALLOC:{
                    v_values[value.first]=true;
                    break;
                }
                case RVT_PHI:{
                    int numDefIsInLoop = 0;
                    for(auto defpoint:value.first->value.phi.phi){//判断所有定值点都在循环中的个数
                        if(loop->loopValues.count(defpoint.second)!=0)
                            numDefIsInLoop++;
                    }
                    if(numDefIsInLoop==0){//所有定值点在循环外
                        v_values[value.first]=true;
                    }
                    else if((value.first->value.phi.phi.size()==1)
                          &&(v_values[value.first->value.phi.phi.front().second])){
                        //只有一个定值点到达且是不变量
                        v_values[value.first]=true;
                    }
                    break;
                }
                default:
                    break;
                }
                //被标记
                if(v_values[value.first]){
                    numNotInVar--;
                    loop->inVar.push_back(value.first);
                }
            }
        }
    }
}
//计算活跃性集合
void cal_actVal(RawFunction*func,map<RawBasicBlock*,unordered_set<RawValue*>>&actValIn,map<RawBasicBlock*,unordered_set<RawValue*>>&actValOut)
{
    map<RawBasicBlock*,unordered_set<RawValue*>>TactValIn,TactValOut;
    for(auto bb:func->basicblock){//初始化为空
        actValIn[bb];
        actValOut[bb];
        TactValIn[bb];
        TactValOut[bb];
    }
    while(1){
        //遍历每个基本块
        for(auto bb:func->basicblock){
            // _in[n]=in[n]; _out[n]=out[n];
            TactValIn[bb]=actValIn[bb];
            TactValOut[bb]=actValOut[bb];
            //out[n]-def[n]
            unordered_set<RawValue*> difference_set,union_set;
            set_difference(actValOut[bb].begin(), actValOut[bb].end(), bb->defs.begin(), bb->defs.end(),std::inserter(difference_set, difference_set.end()));
            //in[n]=use[n]U(out[n]-def[n])
            set_union(difference_set.begin(), difference_set.end(), bb->uses.begin(), bb->uses.end(),std::inserter(union_set, union_set.end()));
            actValIn[bb] = union_set;
            //out[n] = U_in[s] 所有后继的入口活跃集合的并集
            union_set.clear();
            for(auto fbb:bb->fbbs){
                union_set.merge(actValIn[fbb]);
            }
            actValOut[bb] = union_set;
        }
        int count = 0;
        for(auto bb:func->basicblock){
            if(actValOut[bb]==TactValOut[bb]){
                if(TactValIn[bb]==actValIn[bb]){
                    count++;
                }
            }
        }
        if(count==func->basicblock.size())
            return;
    }
}
//判断A是否支配B,或者B的必经结点是否是A
bool AisdomB(RawBasicBlock * A, RawBasicBlock * B){
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
//判断是否满足外提条件
bool judgeCondition(RawValue * value,Loop * loop,map<RawBasicBlock*,unordered_set<RawValue*>> actValIn,map<RawBasicBlock*,unordered_set<RawValue*>> actValOut){
    //Phi函数不外提
    if(value->value.tag==RVT_PHI)return false;

    //所在基本块是所有出口节点的必经节点
    for(auto exitbb:loop->exitNode){
        if(!AisdomB(loop->loopValues[value],exitbb)){
            return false;
        }
        if(!actValOut[exitbb].count(value)){//在出口结点是出口活跃的
            return false;
        }
    }
    //在循环中没有其他定值语句(SSA中为单赋值---不需要考虑该条件(非phi函数时都只定值一次))
    //不属于循环前置节点的出口活跃集合
    return true;
}
//不变量外提
void move_inVar(Loop * &loop,map<RawBasicBlock*,unordered_set<RawValue*>> actValIn,map<RawBasicBlock*,unordered_set<RawValue*>> actValOut)
{
    RawBasicBlock * outloopNode = loop->outloopNode;
    for(auto value:loop->inVar)
    {
        if(judgeCondition(value,loop,actValIn,actValOut)){
            RawBasicBlock * oldBlock = loop->loopValues[value];
            outloopNode->inst.push_back(value);//添加到前置节点中
            oldBlock->inst.remove(value);//从原节点删除
        }
    }
}
//查找所有回边
void findBackEdges(const RawFunctionP &func,vector<Loop *> &natureLoops)
{
    unordered_map<RawBasicBlockP,Loop*> head_loop;//方便查找循环头对应的循环
    auto &bbs = func->basicblock;
    unordered_set<RawBasicBlockP>vbbs;//判断是否访问过
    stack<RawBasicBlockP>stack_bbs;
    stack_bbs.push(*bbs.begin());//初始化,将起始节点入栈
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
                if(isBackEdge(pbb,npbb))//判断是否是回边
                {
                    // backedges.push_back(make_pair((RawBasicBlock*)pbb,(RawBasicBlock*)npbb));
                    if(head_loop.count(npbb)>0)//循环头已经存在
                    {
                        auto loop = head_loop[npbb];
                        loop->backEdgeStart.insert((RawBasicBlock*)pbb);
                    }
                    else
                    {
                        Loop * loop = new Loop((RawBasicBlock*)npbb,(RawBasicBlock*)pbb);
                        natureLoops.push_back(loop);
                        head_loop[npbb] = loop;
                    }
                }
                if(vbbs.find(npbb)==vbbs.end()){//未访问过添加到栈中
                    stack_bbs.push(npbb);
                }
            }
        }
    }
}
//创建循环前置节点
void addLoopPreNode(RawBasicBlock * &bb,RawFunction* &func,Loop * &loop)
{
    unordered_set<RawBasicBlock *> &backEdgeStart = loop->backEdgeStart;
    RawBasicBlock * preLoopBlock = new RawBasicBlock();//创建循环前置节点
    preLoopBlock->name = (char *) malloc(sizeof(char) * 100);
    strcpy((char *)preLoopBlock->name, ("preLoopNode_"+string(bb->name)).c_str());
    //创建跳转value
    RawValue*jumpValue = new RawValue(bb->inst.front());
    jumpValue->value.tag = RVT_JUMP;
    jumpValue->value.jump.target = bb;
    preLoopBlock->inst.push_back(jumpValue);

    stack<RawBasicBlock *>needDel;
    //更新插入节点的前驱（非回边的节点）
    for(auto &prebb:bb->pbbs)
    {
        bool isBackEdgeNode = false;
        for(auto backedge:backEdgeStart)
        {
            if(backedge==prebb)//是回边
            {isBackEdgeNode=true;break;}
        }
        if(!isBackEdgeNode)
        {
            preLoopBlock->pbbs.push_back(prebb);
            needDel.push(prebb);
            RawValue * instbr = prebb->inst.back();
            if(instbr->value.tag==RVT_BRANCH){
                if(instbr->value.branch.true_bb==bb){
                    instbr->value.branch.true_bb=preLoopBlock;
                }
                else if(instbr->value.branch.false_bb==bb){
                    instbr->value.branch.false_bb=preLoopBlock;
                }
            }
            else if(instbr->value.tag==RVT_JUMP){
                if(instbr->value.jump.target==bb){
                    instbr->value.jump.target=preLoopBlock;
                }
            }
            else{//blcok最后的value不是分支或跳转
                assert(false);
            }
        }
    }
    preLoopBlock->fbbs.push_back(bb);//更新插入节点的后继
    while (!needDel.empty())//更新原节点的前驱
    {
        auto prebb = needDel.top();needDel.pop();
        bb->pbbs.remove(prebb);
    }

    bb->pbbs.push_back(preLoopBlock);//添加插入节点作为原节点的前驱之一
    for(auto &pbb:preLoopBlock->pbbs)
    {
        pbb->fbbs.push_back(preLoopBlock);
        pbb->fbbs.remove(bb);
    }
    //插入到循环头前面
    list<RawBasicBlock *>::iterator bbPos = std::find(func->basicblock.begin(),func->basicblock.end(),bb);
    func->basicblock.insert(bbPos,preLoopBlock);
    //初始化循环的外提节点
    loop->outloopNode = preLoopBlock;
}
//将子循环的前置结点添加到父循环中
void addPreBBToLoop(vector<Loop *> natureLoops){
    for(int i=0;i<natureLoops.size();i++){
        for (int j=0;j<natureLoops.size();j++)
        {
            if(i==j)continue;
            else{
                //判断父子关系(i包含j)
                bool isSubSet = std::includes(natureLoops[i]->body.begin(), natureLoops[i]->body.end(),
                natureLoops[j]->body.begin(), natureLoops[j]->body.end());
                if(isSubSet){
                    natureLoops[j]->fatherLoops.insert(natureLoops[i]);
                }
            }
        }
        
    }
    for(auto loop:natureLoops){//为该循环的父循环的基本块集合添加该循环的前置结点
        for(auto fa_loop:loop->fatherLoops){
            fa_loop->body.insert(loop->outloopNode);
        }
    }
}
//循环优化
void OptimizeLoop(RawProgramme *&IR){
    for(auto &func : IR->funcs){
        //查找该函数的所有回边并找出自然循环
        auto &bbs = func->basicblock;
        if(bbs.size()>0){//非空函数
            vector<Loop *>natureLoops;//对每个函数进行循环优化
            map<RawBasicBlock*,unordered_set<RawValue*>> actValIn,actValOut;//对每个函数进行活跃性分析
            // cout<<"111111111111111"<<endl;
            cal_actVal(func,actValIn,actValOut);
            //  cout<<"222222222222222222"<<endl;
            findBackEdges(func,natureLoops);
            for(auto &loop:natureLoops)//每个自然循环添加前置节点
            {
                addLoopPreNode(loop->head,func,loop);
            }
            addPreBBToLoop(natureLoops);
            //对自然循环处理
            for(auto &loop : natureLoops)
            {
                //计算循环的基本块集合
                for(auto start : loop->backEdgeStart){
                    cal_loop(start,loop->head,loop->body);
                }
                //计算循环的指令集合
                cal_loopValues(loop);
                //计算所有出口节点
                cal_exitNodes(loop);
                //计算不变量
                cal_inVar(loop);
                //移动不变量
                move_inVar(loop,actValIn,actValOut);
            }
        }
    }
}
