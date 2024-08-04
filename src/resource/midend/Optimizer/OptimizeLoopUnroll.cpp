#include "../../../include/midend/Optimizer/OptimizeLoopUnroll.h"
void LoopUnrolling::cal_domians(){
    //初始化domains
    for(auto it:this->func->basicblock){
        this->domains[it].insert(it);
        for(auto d_bb:it->domains){
            this->domains[it].insert(d_bb);
        }
    }    
    stack<RawBasicBlock*> stBB;
    stack<RawBasicBlock*> idxBB;
    stBB.push(this->func->basicblock.front());
    while (!stBB.empty())
    {
        RawBasicBlock* bb = stBB.top();stBB.pop();
        idxBB.push(bb);
        for(auto domainbb:bb->domains){
            stBB.push(domainbb);
        }
    }
    while (!idxBB.empty())
    {
        RawBasicBlock* bb = idxBB.top();idxBB.pop();
        RawBasicBlock* fa = (RawBasicBlock*)bb->preDomainNode;
        if(fa!=nullptr){
            for(auto domainbb:this->domains[bb]){
                this->domains[fa].insert(domainbb);
            }
        }
    }    
}
void LoopUnrolling::find_loops(){
    map<RawBasicBlockP,natureloop*> head_loop;//方便查找循环头对应的循环
    for(auto start_bb:this->func->basicblock){
        for(auto end_bb:start_bb->fbbs){
            if(this->domains[end_bb].find(start_bb)!=this->domains[end_bb].end()){
                if(head_loop.find(end_bb)==head_loop.end())//循环头已经存在
                {
                    // Loop * loop = new Loop((RawBasicBlock*)nextbb,(RawBasicBlock*)bb);
                    natureloop * loop = new natureloop(end_bb);
                    this->Loops.push_back(loop);
                    head_loop[end_bb] = loop;
                }
                head_loop[end_bb]->cal_loop(start_bb,end_bb);
            }
        }
    }
    // for(auto it:head_loop){
    //     cout<<"循环头"<<it.first->name<<endl;
    //     for(auto bb:it.second->body){
    //         cout<<bb->name<<",";
    //     }
    //     cout<<endl;
    // }
}
void natureloop::cal_loop(RawBasicBlock * start,RawBasicBlock * end){
    body.insert(start);
    body.insert(end);
    stack<RawBasicBlockP> stack_bbs;
    //查找不经过end能到达start的所有节点
    stack_bbs.push(start);
    while (!stack_bbs.empty())
    {
        RawBasicBlockP pbb = stack_bbs.top();stack_bbs.pop();
        for(auto pre_bb:pbb->pbbs)
        {
            if(body.count(pre_bb)==0)//未添加到loop集合中
            {
                body.insert(pre_bb);//添加到loop集合中
                stack_bbs.push(pre_bb);
            }
        }
    }
}
//循环优化
void OptimizeLoopUnroll(RawProgramme *IR){
    cout<<"循环展开优化"<<endl;
    for(auto func:IR->funcs)
    {
        if(func->basicblock.size()>0){
            LoopUnrolling loopUnroll;
            loopUnroll.setFunc(func);
            loopUnroll.cal_domians();
            loopUnroll.find_loops();
            for(auto loop:loopUnroll.Loops){
                cout<<(loop->head->inst.back()->value.tag==RVT_BRANCH)<<endl;
            }
        }
    }
    cout<<"循环展开优化结束"<<endl;
}