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
void natureloop::determineLoopType(){
    if(body.size()>2){
        loopType = LoopType::LoopNoneEnd;
        return;
    }
    if(head->inst.back()->value.tag!=RVT_BRANCH){
        assert(!"循环条件错误");
    }
    if(head->inst.back()->value.branch.cond->value.tag==RVT_INTEGER){
        loopType = LoopType::LoopNoneEnd;
        // cout<<"死循环"<<endl;
        return;
    }
    RawValue* BranchCondValue = (RawValue*)head->inst.back()->value.branch.cond;
    if(BranchCondValue->value.tag!=RVT_BINARY){
        assert(!"循环条件不是比较条件");
    }
    switch (BranchCondValue->value.binary.op)
    {
        case RBO_EQ:
        case RBO_GE:
        case RBO_GT:
        case RBO_LE:
        case RBO_LT:
        case RBO_NOT_EQ:
            break;
        default:{
            assert(!"非关系比较条件");
        }
    }
    RawValue* lhs = (RawValue*)BranchCondValue->value.binary.lhs;
    RawValue* rhs = (RawValue*)BranchCondValue->value.binary.rhs;
    if(loopIncreaseValue.count(lhs)==1&&rhs->value.tag==RVT_INTEGER){
        loopType = LoopType::LoopValueEnd;
        return;
    }
    // cout<<"lhs:";
    // cout<<BranchCondValue->value.binary.lhs->value.tag<<endl;
    // cout<<"rhs:";
    // cout<<BranchCondValue->value.binary.rhs->value.tag<<endl;
    // cout<<"dadad"<<endl;

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
void natureloop::cal_loopIncreaseValue(){
    map<RawValue*,int> firstloop;firstloop.clear();
    for(auto bb:body){
        // cout<<bb->name<<endl;
        for(auto phi:bb->phi){
            RawValue* inst =(RawValue*)phi;
            if(inst->value.phi.phi.size()==2){
                firstloop[inst] = inst->value.phi.phi[0].second->value.integer.value;
            }
        }
        for(auto inst:bb->inst){
            // cout<<"指令类型:"<<inst->value.tag<<endl;
            switch (inst->value.tag)
            {
            case RVT_INTEGER:{
                firstloop[inst] = inst->value.integer.value;
                // cout<<firstloop[inst]<<endl;
                break;
            }
            case RVT_BINARY:{
                RawValue* lhs = (RawValue*)inst->value.binary.lhs;
                RawValue* rhs = (RawValue*)inst->value.binary.rhs;
                if(firstloop.find(lhs)==firstloop.end()||firstloop.find(rhs)==firstloop.end())
                {
                    break;
                }
                switch (inst->value.binary.op)
                {
                    case RBO_ADD:{
                        firstloop[inst] = firstloop[lhs]+firstloop[rhs];
                        break;
                    }
                    case RBO_SUB:{
                        firstloop[inst] = firstloop[lhs]-firstloop[rhs];
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            default:
                break;
            }
        }
    }
    // cout<<"first"<<endl;
    // for(auto it:firstloop){
    //     RawValue* val = it.first;
    //     if(val->value.tag==RVT_PHI)
    //     {
    //         cout<<val->value.phi.target->name<<"="<<it.second<<endl;
    //     }
    // }
    map<RawValue*,int> secondloop;
    for(auto bb:body){
        // cout<<bb->name<<endl;
        for(auto inst:bb->inst){
            for(auto phi:bb->phi){
                RawValue* inst =(RawValue*)phi;
                if(inst->value.phi.phi.size()==2){
                    secondloop[inst] = firstloop[inst->value.phi.phi[1].second];
                }
            }
            switch (inst->value.tag)
            {
            case RVT_INTEGER:{
                secondloop[inst] = inst->value.integer.value;
                break;
            }
            case RVT_BINARY:{
                RawValue* lhs = (RawValue*)inst->value.binary.lhs;
                RawValue* rhs = (RawValue*)inst->value.binary.rhs;
                if(secondloop.find(lhs)==secondloop.end()&&secondloop.find(rhs)==secondloop.end())
                {
                    break;
                }
                switch (inst->value.binary.op)
                {
                case RBO_ADD:{
                    secondloop[inst] = secondloop[lhs]+secondloop[rhs];
                    break;
                }
                case RBO_SUB:{
                    secondloop[inst] = secondloop[lhs]-secondloop[rhs];
                    break;
                }
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }
        }
    }
    // cout<<"second"<<endl;
    // for(auto it:secondloop){
    //     RawValue* val = it.first;
    //     if(val->value.tag==RVT_PHI)
    //     {
    //         cout<<val->value.phi.target->name<<"="<<it.second<<endl;
    //     }
    // }
    // cout<<"自增"<<endl;
    for(auto it:secondloop){
        RawValue* val = it.first;
        if(val->value.tag==RVT_PHI&&firstloop.find(val)!=firstloop.end()){
            loopIncreaseValue[val] = secondloop[val]-firstloop[val];
            // cout<<val->value.phi.target->name<<"自增"<<loopIncreaseValue[val]<<endl;
        }
    }
}
void natureloop::unrollingValueLoop(){

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
                cout<<"计算循环增量"<<endl;
                loop->cal_loopIncreaseValue();
                cout<<"计算循环类型"<<endl;
                loop->determineLoopType();
                if(LoopType::NotJudge==loop->loopType){
                    assert(!"未判断循环类型");
                }
                else if(LoopType::LoopValueEnd==loop->loopType){
                    loop->unrollingValueLoop();
                }
            }
        }
    }
    cout<<"循环展开优化结束"<<endl;
}