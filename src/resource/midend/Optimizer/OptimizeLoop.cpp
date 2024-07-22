#include <set>
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
//判断是否是循环不变量
bool isInVar(RawBasicBlock* bb,RawValue * value,Loop * loop,unordered_map<RawValue *,bool> v_values,map<RawBasicBlock *,set<RawValue *>> in)
{
    switch (value->value.tag)//常数
    {
        //整型常量
        case RVT_INTEGER:
        //浮点常量
        case RVT_FLOAT:
            return true;
        default:
            break;
    }
    int setValNum = 0;//到达定值个数
    int setValOutNum = 0;//到达定值在循化外的个数
    RawValue * FirstReachVal;
    for(auto setVal:in[bb]){//定值语句基本块的到达定值
        if(setVal->value.store.dest==value){//所有定值
            setValNum++;
            if(loop->loopValues.find(setVal)==loop->loopValues.end()){//定值在循环外
                setValOutNum++;
            }
            FirstReachVal = setVal;
        }
        if(setValNum==setValOutNum)//所有定值在循环外
        {
            return true;
        }else if(setValNum==1&&v_values[FirstReachVal]){//只有一个定值而且是不变量
            return true;
        }
    }
    return false;
}
//判断从start基本块到end基本块的边是否是回边 
bool isBackEdge(const RawBasicBlockP start,const RawBasicBlockP end)
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
void cal_inVar(Loop * &loop,map<RawBasicBlock *,set<RawValue *>> in){
    //所有代码初始化为未标记
    unordered_map<RawValue *,bool> v_values;//未标记的value
    int numNotInVar = loop->loopValues.size();//初始时所有value都不是不变量
    int last_numNotInVar = -1;
    for(auto value:loop->loopValues){//初始化
        RawValue * val = value.first;
        if(val->value.tag==RVT_INTEGER||val->value.tag==RVT_FLOAT){
            loop->inVar.push_back(val);
            v_values[val] = true;//标记为不变量
            numNotInVar--;
        }else if(val->value.tag==RVT_ALLOC){
            loop->inVar.push_back(val);
            v_values[val] = false;
        }
        else{
            v_values[val] = false;
        }
    }
    while(last_numNotInVar!=numNotInVar){
        last_numNotInVar=numNotInVar;
        for(auto value:v_values){
            if(value.second==false){//剩余value
            RawValue* val =value.first;
                switch (val->value.tag){
                    case RVT_STORE:{
                        if(v_values[(RawValue*)val->value.store.value]){
                            loop->inVar.push_back(val);
                            v_values[val] = true;
                            numNotInVar--;
                        }
                        break;
                    }
                    case RVT_LOAD:{
                        // cout<<value.first->value.store.value->value.integer.value<<endl;
                        // cout<<"asdasda"<<endl;
                        if(isInVar(loop->loopValues[val],(RawValue*)val->value.load.src,loop,v_values,in)){
                            loop->inVar.push_back(val);
                            v_values[val] = true;
                            numNotInVar--;
                        }
                        break;
                    }
                    case RVT_BINARY:{
                        RawValue * lhs = (RawValue *)val->value.binary.lhs;
                        RawValue * rhs = (RawValue *)val->value.binary.rhs;
                        if(v_values[lhs]&&v_values[rhs]){
                            loop->inVar.push_back(val);
                            v_values[val] = true;
                            numNotInVar--;
                        }
                        break;
                    }
                    default:
                        break;
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
        RawBasicBlock * oldBlock = loop->loopValues[value];
        outloopNode->inst.insert(--outloopNode->inst.end(),value);//添加到前置节点中
        oldBlock->inst.remove(value);//从原节点删除
        // if(judgeCondition(value,loop,actValIn,actValOut)){
        //     RawBasicBlock * oldBlock = loop->loopValues[value];
        //     outloopNode->inst.push_back(value);//添加到前置节点中
        //     oldBlock->inst.remove(value);//从原节点删除
        // }
    }
}
//查找所有回边 
void findBackEdges(const RawFunctionP func,vector<Loop *> &Loops)
{
    unordered_map<RawBasicBlockP,Loop*> head_loop;//方便查找循环头对应的循环
    //遍历所有边判断是否是回边
    for(auto bb:func->basicblock){
        for(auto nextbb:bb->fbbs){
            if(isBackEdge(bb,nextbb))//判断是否是回边
            {
                if(head_loop.count(nextbb)>0)//循环头已经存在
                {
                    auto loop = head_loop[nextbb];
                    loop->backEdgeStart.insert((RawBasicBlock*)bb);
                }
                else
                {
                    Loop * loop = new Loop((RawBasicBlock*)nextbb,(RawBasicBlock*)bb);
                    Loops.push_back(loop);
                    head_loop[nextbb] = loop;
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
//定值到达分析
void ReachDef(RawFunction* func,map<RawBasicBlock *,set<RawValue *>> &in,map<RawBasicBlock *,set<RawValue *>> &out){
    map<RawBasicBlock *,set<RawValue *>> gen,kill;
    map<RawBasicBlock*,int>outsize;
    for(auto bb:func->basicblock){//构造每个基本块的gen集合与kill集合
        outsize[bb]=0;
        for (auto it = bb->inst.rbegin(); it != bb->inst.rend(); ++it) {
            RawValue * value = *it;
            if(value->value.tag==RVT_STORE)//定值
            {
                if(kill[bb].find(value)==kill[bb].end())//gen[bb]=gen[val_n]U(gen[val_n-1]-kill[val_n~val_n-1])
                {
                    gen[bb].insert(value);
                }
                for(auto defpoint:value->value.store.dest->defPoints){//kill[bb]=kill[val_n]U...Ukill[val_1]
                    if(defpoint!=value)
                    {
                        kill[bb].insert(defpoint);
                    }
                }
            }
        }
    }
    bool needit = true;
    while(needit){//存在out变化就需要继续迭代
        needit = false;
        auto it = func->basicblock.begin();++it; // 跳过第一个元素
        // 从第二个元素开始遍历
        for (; it != func->basicblock.end(); ++it){//除了entry的所有基本块
            auto bb = (*it);
            for(auto prebb:bb->pbbs){//in[bb]= Uout[prebb]
                for(auto val:out[prebb])
                    in[bb].insert(val);
            }
            out[bb] = gen[bb];
            std::set_difference(in[bb].begin(), in[bb].end(),
                                kill[bb].begin(), kill[bb].end(),
                                std::inserter(out[bb],out[bb].begin()));
            if(out[bb].size()!=outsize[bb]){//存在out变化需要继续迭代
                outsize[bb] = out[bb].size();
                needit = true;
            }
        } 
    }
}
//循环优化
void OptimizeLoop(RawProgramme *IR){
    for(auto &func : IR->funcs){
        //对每个函数进行循环优化
        auto &bbs = func->basicblock;
        if(bbs.size()>0){//非空函数
            vector<Loop *>natureLoops;//当前函数的循环集合
            map<RawBasicBlock *,set<RawValue *>> in,out;
                map<RawBasicBlock*,unordered_set<RawValue*>> actValIn,actValOut;//对每个函数进行活跃性分析
                // cout<<"111111111111111"<<endl;
                // cal_actVal(func,actValIn,actValOut);
                //  cout<<"222222222222222222"<<endl;
            findBackEdges(func,natureLoops);//计算回边
            for(auto &loop:natureLoops)//每个自然循环添加前置节点
            {
                addLoopPreNode(loop->head,func,loop);
            }
            addPreBBToLoop(natureLoops);//将前置节点添加到父循环中
            ReachDef(func,in,out);//到达定值分析
            // for(auto bb:func->basicblock){
            //     cout<<"基本块："<<bb->name<<endl;
            //     cout<<"IN定值到达集合:"<<endl;
            //     for(auto val:in[bb]){
            //         cout<<"STROE"<<val->value.store.value->value.integer.value<<","
            //         <<val->value.store.dest->name<<endl;
            //     }
            //     cout<<"OUT定值到达集合:"<<endl;
            //     for(auto val:out[bb]){
            //         cout<<"STROE"<<val->value.store.value->value.integer.value<<","
            //         <<val->value.store.dest->name<<endl;
            //     }
            // }
            // 对自然循环处理
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
                cal_inVar(loop,in);
                //移动不变量
                move_inVar(loop,actValIn,actValOut);
            }
        }
    }
}