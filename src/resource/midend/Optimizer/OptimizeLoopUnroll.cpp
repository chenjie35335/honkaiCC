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
                    loop->func = func;
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
    if(loopIncreaseValue.count(lhs)==1){
        if(lhs->value.tag==RVT_PHI
            &&lhs->value.phi.phi[0].second->value.tag==RVT_INTEGER
            &&rhs->value.tag==RVT_INTEGER){
            loopType = LoopType::LoopValueEnd;
            return;
        }
        else{
            loopType = LoopType::LoopVarEnd;
            return;
        }
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
        // if(val->value.tag==RVT_PHI&&firstloop.find(val)!=firstloop.end()){
        //     loopIncreaseValue[val] = secondloop[val]-firstloop[val];
        //     // cout<<val->value.phi.target->name<<"自增"<<loopIncreaseValue[val]<<endl;
        // }
        if(firstloop.find(val)!=firstloop.end()){
            loopIncreaseValue[val] = secondloop[val]-firstloop[val];
            // cout<<val->value.phi.target->name<<"自增"<<loopIncreaseValue[val]<<endl;
        }
    }
}
void natureloop::unrollingValueLoop(){
    //计算展开次数
    RawValue* compareVal = *(++head->inst.rbegin());
    RawValue* loopIncVal = (RawValue*)compareVal->value.binary.lhs;
    int looptimes = loopTimes((RawValue*)compareVal->value.binary.lhs,compareVal);
    if(looptimes>500||looptimes<2)return;
    else{
        looptimes--;//自身存在一次
    }
    //初始化展开基本块
    RawBasicBlock* urollingBlock = new RawBasicBlock();
    size_t bufSize = strlen(head->name) + 15;
    urollingBlock->name = new char[bufSize];
    snprintf((char *)urollingBlock->name, bufSize, "%s_%s", head->name, "urolling");
    //查找循化体基本块
    RawBasicBlock* onebody = (RawBasicBlock*)head->inst.back()->value.branch.true_bb;
    //插入新基本块
    auto it = std::find(func->basicblock.begin(), func->basicblock.end(), onebody);
    func->basicblock.insert(++it,urollingBlock);
    //修正新基本块的跳转关系
    RawValue* oldJump = onebody->inst.back();
    // cout<<oldJump->value.jump.target->name<<endl;
    RawValue* jumpUnrolling = new RawValue(oldJump);
    jumpUnrolling->value.jump.target = head->inst.back()->value.branch.false_bb;
    urollingBlock->inst.push_back(jumpUnrolling);
    //修正循环头的基本块跳转关系
    head->inst.back()->value.tag = RVT_JUMP;
    head->inst.back()->value.jump.target = onebody;
    //修正循环体的基本块跳转关系
    onebody->inst.back()->value.jump.target = urollingBlock;
    //复制value
    // 初始化
    map<RawValue*,RawValue*> oTn;oTn.clear();
    oTn[loopIncVal] = loopIncVal->value.phi.phi[1].second;
    while(looptimes--){
    for(auto bb:body){
        for(auto inst:bb->inst){
            switch (inst->value.tag)
            {
            case RVT_BINARY:{
                if(inst==compareVal)
                    break;
                else{
                    RawValue* nbinary = new RawValue(inst);
                    nbinary->value.binary.op = inst->value.binary.op;
                    if(oTn.find((RawValue*)inst->value.binary.lhs)==oTn.end()){
                        nbinary->value.binary.lhs = inst->value.binary.lhs;
                    }else{
                        nbinary->value.binary.lhs = oTn[(RawValue*)inst->value.binary.lhs];
                    }

                    if(oTn.find((RawValue*)inst->value.binary.rhs)==oTn.end()){
                        nbinary->value.binary.rhs = inst->value.binary.rhs;
                    }else{
                        nbinary->value.binary.rhs = oTn[(RawValue*)inst->value.binary.rhs];
                    }
                    oTn[inst]=nbinary;
                    urollingBlock->inst.insert(--urollingBlock->inst.end(),nbinary);
                    if(inst==loopIncVal->value.phi.phi[1].second){
                        oTn[loopIncVal]=nbinary;
                    }

                }
                break;
            }
            case RVT_CALL:{
                RawValue* ncall = new RawValue(inst);
                ncall->value.call.callee = inst->value.call.callee;
                for(int i=0;i<inst->value.call.args.size();i++){
                    if(oTn.find((RawValue*)inst->value.call.args[i])==oTn.end()){
                        ncall->value.call.args.push_back(inst->value.call.args[i]);
                    }else{
                        ncall->value.call.args.push_back(oTn[(RawValue*)inst->value.call.args[i]]);
                    }
                }
                oTn[inst]=ncall;
                urollingBlock->inst.insert(--urollingBlock->inst.end(),ncall);
                break;
            }
            case RVT_CONVERT:{
                RawValue* nConvert = new RawValue(inst);
                nConvert->value.Convert.src = inst->value.Convert.src;
                oTn[inst]=nConvert;
                urollingBlock->inst.insert(--urollingBlock->inst.end(),nConvert);
                break;
            }
            default:
                break;
            }
        }
    }
    }
    //修改phi函数(放在赋值展开后)
    for(auto bb:body){
        for(auto phi=bb->phi.begin();phi!=bb->phi.end();){
            for (auto it = (*phi)->value.phi.phi.begin(); it != (*phi)->value.phi.phi.end(); ) {
                if ((*it).first == onebody) {
                    it = (*phi)->value.phi.phi.erase(it);
                } else {
                    ++it;
                }
            }
            if((*phi)->value.phi.phi.size()==0){
                phi = bb->phi.erase(phi);
            }else{
                ++phi;
            }
        }
    }
    
}
void natureloop::unrollingVarLoop(int unRollingFactor){
    //查找循化体基本块
    RawBasicBlock* onebody = (RawBasicBlock*)head->inst.back()->value.branch.true_bb;
    //初始化展开的head基本块
    RawBasicBlock* urollingHead = new RawBasicBlock();
    size_t bufSize = strlen(head->name) + 15;
    urollingHead->name = new char[bufSize];
    snprintf((char *)urollingHead->name, bufSize, "%s_%s", head->name, "urolling");
    //初始化展开的body基本块
    RawBasicBlock* urollingBody = new RawBasicBlock();
    bufSize = strlen(onebody->name) + 15;
    urollingBody->name = new char[bufSize];
    snprintf((char *)urollingBody->name, bufSize, "%s_%s", onebody->name, "urolling");
    //插入新基本块
    auto it = std::find(func->basicblock.begin(), func->basicblock.end(), head);
    func->basicblock.insert(it,urollingHead);
    func->basicblock.insert(it,urollingBody);
    //复制value
    cout<<"复制value"<<endl;
    map<RawValue*,RawValue*> oTn;oTn.clear();
    for(auto bb:body){
        RawBasicBlock* needInsertValueBlock;
        if(bb==head){
            needInsertValueBlock=urollingHead;
        }else if(bb==onebody){
            needInsertValueBlock=urollingBody;
        }
        for(auto p:bb->phi){
            RawValue* newPhi = new RawValue(p);
            newPhi->value.phi.target = p->value.phi.target;
            newPhi->value.phi.phi = p->value.phi.phi;
            needInsertValueBlock->phi.push_back(newPhi);
        }
        for(auto inst:bb->inst){
            switch (inst->value.tag)
            {
            case RVT_INTEGER:{
                RawValue* intVal = new RawValue(inst);
                intVal->value.integer.value=inst->value.integer.value;
                needInsertValueBlock->inst.push_back(intVal);
                oTn[inst] = intVal;
                break;
            }
            case RVT_FLOAT:{
                RawValue* floatVal = new RawValue(inst);
                floatVal->value.floatNumber.value=inst->value.floatNumber.value;
                needInsertValueBlock->inst.push_back(floatVal);
                oTn[inst] = floatVal;
                break;
            }
            case RVT_ALLOC:{
                RawValue* allocVal = new RawValue(inst);
                needInsertValueBlock->inst.push_back(allocVal);
                oTn[inst] = allocVal;
                break;
            }
            case RVT_BINARY:{
                RawValue* nbinary = new RawValue(inst);
                nbinary->value.binary.op = inst->value.binary.op;
                if(oTn.find((RawValue*)inst->value.binary.lhs)==oTn.end()){
                    nbinary->value.binary.lhs = inst->value.binary.lhs;
                }else{
                    nbinary->value.binary.lhs = oTn[(RawValue*)inst->value.binary.lhs];
                }
                if(oTn.find((RawValue*)inst->value.binary.rhs)==oTn.end()){
                    nbinary->value.binary.rhs = inst->value.binary.rhs;
                }else{
                    nbinary->value.binary.rhs = oTn[(RawValue*)inst->value.binary.rhs];
                }
                oTn[inst]=nbinary;
                needInsertValueBlock->inst.push_back(nbinary);
                break;
            }
            case RVT_CALL:{
                RawValue* ncall = new RawValue(inst);
                ncall->value.call.callee = inst->value.call.callee;
                for(int i=0;i<inst->value.call.args.size();i++){
                    if(oTn.find((RawValue*)inst->value.call.args[i])==oTn.end()){
                        ncall->value.call.args.push_back(inst->value.call.args[i]);
                    }else{
                        ncall->value.call.args.push_back(oTn[(RawValue*)inst->value.call.args[i]]);
                    }
                }
                oTn[inst]=ncall;
                needInsertValueBlock->inst.push_back(ncall);
                break;
            }
            case RVT_CONVERT:{
                RawValue* nConvert = new RawValue(inst);
                nConvert->value.Convert.src = inst->value.Convert.src;
                oTn[inst]=nConvert;
                needInsertValueBlock->inst.push_back(nConvert);
                break;
            }
            case RVT_BRANCH:{
                RawValue* nbranch = new RawValue(inst);
                if(oTn.find((RawValue*)inst->value.branch.cond)==oTn.end()){
                    nbranch->value.branch.cond = inst->value.branch.cond;
                }else{
                    nbranch->value.branch.cond = oTn[(RawValue*)inst->value.branch.cond];
                }
                nbranch->value.branch.true_bb = inst->value.branch.true_bb;
                nbranch->value.branch.false_bb = inst->value.branch.false_bb;
                needInsertValueBlock->inst.push_back(nbranch);
                break;
            }
            case RVT_JUMP:{
                RawValue* njump = new RawValue(inst);
                njump->value.jump.target = inst->value.jump.target;
                oTn[inst]=njump;
                needInsertValueBlock->inst.push_back(njump);
                break;
            }
            default:
                break;
            }
        }
    }
    //修改跳转目标
    for(auto pbb:head->pbbs){
        RawValue* bjValue = pbb->inst.back();
        if(bjValue->value.tag==RVT_JUMP){
            bjValue->value.jump.target=urollingHead;

        }else if(bjValue->value.tag==RVT_BRANCH){
            if(bjValue->value.branch.true_bb==head){
                bjValue->value.branch.true_bb=urollingHead;
            }else{
                bjValue->value.branch.false_bb=urollingHead;
            }
        }
    }
    urollingHead->inst.back()->value.branch.true_bb = urollingBody;
    urollingHead->inst.back()->value.branch.false_bb = head;
    urollingBody->inst.back()->value.jump.target = urollingHead;
}
int natureloop::loopTimes(RawValue* condVal,RawValue* cond){
    int start = condVal->value.phi.phi[0].second->value.integer.value;
    int delta = loopIncreaseValue[condVal];
    int endCond = cond->value.binary.rhs->value.integer.value;
    // cout<<start<<endl;
    // cout<<delta<<endl;
    // cout<<endCond<<endl;
    int res = 0;
    while(1){
        switch (cond->value.binary.op)
        {
            case RBO_LT:{
                if(start>=endCond)return res;
                break;
            }
            case RBO_LE:{
                if(start>endCond)return res;
                break;
            }
            case RBO_GT:{
                if(start<=endCond)return res;
                break;
            }
            case RBO_GE:{
                if(start<endCond)return res;
                break;
            } 
            default:{
                assert(!"条件错误");
                break;
            }
        }
        start+=delta;
        res++;
        if(res>500)
            return -1;
    }
    return -1;
}
//循环优化
void OptimizeLoopUnroll(RawProgramme *IR){
    // cout<<"循环展开优化"<<endl;
    for(auto func:IR->funcs)
    {
        if(func->basicblock.size()>0){
            LoopUnrolling loopUnroll;
            loopUnroll.setFunc(func);
            loopUnroll.cal_domians();
            loopUnroll.find_loops();
            for(auto loop:loopUnroll.Loops){
                if(loop->body.size()>2)
                    continue;
                // cout<<"计算循环增量"<<endl;
                loop->cal_loopIncreaseValue();
                // cout<<"计算循环类型"<<endl;
                loop->determineLoopType();
                if(LoopType::NotJudge==loop->loopType){
                    assert(!"未判断循环类型");
                }
                else if(LoopType::LoopValueEnd==loop->loopType){
                    loop->unrollingValueLoop();
                }else if(LoopType::LoopVarEnd==loop->loopType){
                    loop->unrollingVarLoop(UNROLLFACTOR);
                }
            }
        }
    }
    // cout<<"循环展开优化结束"<<endl;
}