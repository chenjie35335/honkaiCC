#include <iostream>
#include <cassert>
#include <string.h>
#include <queue>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include "../IR/Function.h"
#include "../IR/BasicBlock.h"
#include "../IR/Value.h"
#include "../../../include/midend/Optimizer/OptimizeInline.h"
bool judgementOutFunc(string fun_name)
{
    if("putch"==fun_name){
        return true;
    }
    else if("putint"==fun_name){
        return true;
    }
    else if("starttime"==fun_name){
        return true;
    }
    else if("stoptime"==fun_name){
        return true;
    }
    else if("getch"==fun_name){
        return true;
    }
    else if("getint"==fun_name){
        return true;
    }
    else if("putarray"==fun_name){
        return true;
    }
    else if("getarray"==fun_name){
        return true;
    }
    else
        return false;
}
void OptimizeFuncInline(RawProgramme * programme){
    //确定可以内联的函数
    unordered_set<RawFunction *>inline_funcs;
    // for(auto inline_func:inline_funcs){
    //     cout<<inline_func->name<<endl;
    // }
    MarkInlineFunc(programme->funcs,inline_funcs);
    //对函数进行内联
    for(auto func:programme->funcs){
        if(func->basicblock.size()>0)
        {
            InlineFunc(func,inline_funcs);
        }
    }
    
}
//判断函数列表中可以内联的函数集合
void MarkInlineFunc(list<RawFunction *> funcs,unordered_set<RawFunction *>&inline_funcs){
    for(auto func:funcs){
        if(func->basicblock.size()>0&&(strcmp(func->name,"main")!=0)){//非外部库函数且不是main函数
            // cout<<func->name<<":";
            // cout<<"条件分支个数，"<<CalCondNum(func)<<endl;
            // cout<<"value个数,"<<CalValueNum(func)<<endl;
            if(!HasLoopInFunc(func)){//不存在循环与回边
                if(!HasCallSelfFunc(func)){//非递归函数
                    if(BRANCH_SIZE>=CalCondNum(func)){//条件分支数量合适
                        if(VALUE_SIZE>=CalValueNum(func)){//value数量合适
                            inline_funcs.insert(func);
                            // cout<<"可以内联"<<endl;
                        }
                    }   
                }
                // else{
                //     cout<<"有递归，"<<endl;
                // }
            }
            // else{
            //     cout<<"有循环，"<<endl;
            // }
        }
    }
}
//判断函数中是否存在循环
bool HasLoopInFunc(RawFunction *func){
    for(auto startbb:func->basicblock){
        //已访问过的基本块
        unordered_set<RawBasicBlock *>vbbs;
        stack<RawBasicBlock*>bbs;
        bbs.push(startbb);//起始的基本块
        vbbs.insert(startbb);//标志访问
        while(!bbs.empty()){
            RawBasicBlock * nowbb = bbs.top();bbs.pop();
            for(auto bb:nowbb->fbbs){
                if(bb==startbb)
                {
                    return true;
                }
                if(vbbs.count(bb)==0)//已经访问过该基本块，存在回边和循环
                {
                    bbs.push(bb);
                    vbbs.insert(bb);
                }
            }
        }
    }
    return false;
}
//判断函数是否递归
bool HasCallSelfFunc(RawFunction * func){
    for(auto bb:func->basicblock){
        for(auto inst:bb->inst){
            if(inst->value.tag==RVT_CALL){//函数调用
                if(inst->value.call.callee==func){//函数调用自身
                    return true;
                }
            }
        }
    }
    return false;
}
//计算函数中条件判断的数量
int  CalCondNum(RawFunction * func){
    int condnum = 0;
    for(auto bb:func->basicblock){
        for(auto inst:bb->inst){
            if(inst->value.tag==RVT_BRANCH){//条件分支
                condnum++;
            }
        }
    } 
    return condnum;
}
//计算函数中value条数
int  CalValueNum(RawFunction * func){
    int valueNum = 0;
    for(auto bb:func->basicblock){
        valueNum+=bb->inst.size();
    } 
    return valueNum;
}
//分配内联函数共用的内部变量，参数变量，返回值变量
void InlineAllocShareVar(RawFunction * func){
    bool HasAllocRet = false;
    // cout<<func->name<<endl;
    //内联函数参数
    for(auto para:func->params){
        RawValue * shareVar = new RawValue(para);
        int bufSize = strlen(func->name)+25;
        shareVar->name = new char[bufSize];//(func_name)_arg_idx
        snprintf((char *)shareVar->name, bufSize, "%s_arg_%zu", func->name,para->value.funcArgs.index);
        shareVar->value.tag = RVT_ALLOC;//改变为分配变量
        InlineShareVar[para] = shareVar;
    }
    for(auto bb:func->basicblock){
        for(auto inst:bb->inst){
            // cout<<inst->value.tag<<endl;
            //内联函数内部分配变量
            if(inst->value.tag==RVT_ALLOC){
                RawValue * shareVar = new RawValue(inst);
                InlineShareVar[inst] = shareVar;
                // cout<<shareVar->name<<endl;
            }
            //内联函数返回值
            else if(inst->value.tag==RVT_RETURN){//需要考虑类型
                if(!HasAllocRet){//还未分配返回变量
                    RawValue * shareVar = new RawValue(inst);
                    int bufSize = strlen(func->name)+5;
                    shareVar->name = new char[bufSize];//(func_name)_ret
                    snprintf((char *)shareVar->name, bufSize, "%s_ret", func->name);
                    shareVar->value.tag = RVT_ALLOC;//改变为分配变量
                    shareVar->ty = func->ty->function.ret;//改变返回类型
                    RetValue[func] = shareVar;//共享一个变量
                    HasAllocRet = true;
                    // cout<< shareVar->name<<endl;
                }
            }
        }
    }
}
//对函数进行内联
void InlineFunc(RawFunction *func,unordered_set<RawFunction *>inline_funcs){
    RetValue.clear();
    InlineShareVar.clear();//清除上一次对函数做过内联的记录
    queue<RawBasicBlock*>inline_bbs;//需要内联的基本块
    unordered_set<RawBasicBlock*>hasInlineBBS;
    unordered_set<RawFunction*>HasInlineFuncVar;//已经分配过共享变量的内联函数
    for(auto bb:func->basicblock){
        for(auto inst:bb->inst){
            if(inst->value.tag==RVT_CALL){//函数调用
                RawFunction* callee = (RawFunction*)inst->value.call.callee;
                if(inline_funcs.find(callee)!=inline_funcs.end()){//调用的函数可以内联
                    if(HasInlineFuncVar.count(callee)==0){
                        InlineAllocShareVar(callee);
                        HasInlineFuncVar.insert(callee);
                    }
                    if(hasInlineBBS.count(bb)==0){
                        inline_bbs.push(bb);
                        hasInlineBBS.insert(bb);
                    }
                }
            }
        }
    }
    // cout<<func->basicblock.front()->name<<endl;
    for(auto inst:InlineShareVar){//内部变量，参数变量
        func->basicblock.front()->inst.push_front(inst.second);
    }
    for(auto inst:RetValue){//返回值变量
        if(inst.second->ty->tag!=RTT_UNIT)
            func->basicblock.front()->inst.push_front(inst.second);
    }

    while (!inline_bbs.empty())//对基本块内联
    {
        auto bb = inline_bbs.front();inline_bbs.pop();
        InlineBasicBlock(bb,func,inline_funcs);
    }
    // for(auto pair:sliceBlocks){
    //     cout<<pair.first->name<<"----------"<<pair.second->name<<endl;
    // }
    //更新原有的branch语句
    for(auto bb:func->basicblock){
        for(auto inst:bb->inst){
            if(inst->value.tag==RVT_BRANCH){
                RawBasicBlock* true_bb = (RawBasicBlock*)inst->value.branch.true_bb;
                RawBasicBlock* false_bb = (RawBasicBlock*)inst->value.branch.false_bb;
                if(sliceBlocks.count(true_bb)!=0){
                    inst->value.branch.true_bb = sliceBlocks[true_bb];
                }
                if(sliceBlocks.count(false_bb)!=0){
                    inst->value.branch.false_bb = sliceBlocks[false_bb];
                }
            }
        }
    }
}
//对基本块进行内联
void InlineBasicBlock(RawBasicBlock* bb,RawFunction *func,unordered_set<RawFunction *>inline_funcs){
    deque<RawBasicBlock *> slicebbs;
    RawBasicBlock * startbb = new RawBasicBlock(bb->name,inlinefunc_idx++);
    sliceBlocks[bb]=startbb;//保持分割前后的基本块对应
    for(auto pbb:bb->pbbs){//更新插入内联的基本块所有前驱的后继
        pbb->fbbs.remove(bb);
        pbb->fbbs.push_back(startbb);
        startbb->pbbs.push_back(pbb);//更新内联函数入口节点的前驱
    }
    slicebbs.push_back(startbb);
    //分割
    bool needkeep=true;
    for(auto inst:bb->inst){
        if(inst->value.tag==RVT_CALL){//函数调用
            RawFunction* callee = (RawFunction*)inst->value.call.callee;
            if(inline_funcs.find(callee)!=inline_funcs.end()){//调用的函数可以内联
                //初始化参数
                int idx = 0;
                for(auto para:inst->value.call.args){
                    RawValue * storeValue = new RawValue(para);
                    storeValue->value.tag = RVT_STORE;
                    if(para)
                    storeValue->value.store.value = para;
                    storeValue->value.store.dest = InlineShareVar[callee->params[idx++]];
                    slicebbs.back()->inst.push_back(storeValue);
                }

                //在内联点分割block
                RawBasicBlock * slicebb = new RawBasicBlock(bb->name,inlinefunc_idx++);
                InsertInlineFunc(callee,slicebbs,inlinefunc_idx-2,slicebb);
                
                slicebbs.push_back(slicebb);
                if(inst->ty->tag==RTT_UNIT){//内联的函数无返回值
                    needkeep=false;
                }
                else{
                    needkeep=true;
                    inst->value.tag=RVT_LOAD;
                    inst->value.load.src=RetValue[callee];
                }
            }
            else{
                needkeep=true;
            }
            // else if(judgementOutFunc(callee->name))//库函数
            // {
            //     needkeep=true;
            // }
            // else if(inst->ty->tag==RTT_UNIT){//调用的函数无返回值
            //     needkeep=false;
            // }
            // else{
            //     needkeep=true;
            // }
        }
        else{
            needkeep=true;
        }
        if(needkeep)
            slicebbs.back()->inst.push_back(inst);
    }
    //更新分割后的最后内联块的后继
    // cout<<slicebbs.back()->name<<endl;
    for(auto fbb:bb->fbbs){
        slicebbs.back()->fbbs.push_back(fbb);
        fbb->pbbs.remove(bb);
        fbb->pbbs.push_back(slicebbs.back());
    }
    //插入
    auto it = func->basicblock.begin();
    while (it != func->basicblock.end()) {
        if (*it==bb) {
            break;
        }
        ++it;
    }
    while(!slicebbs.empty())
    {
        RawBasicBlock * needInsertBB = slicebbs.front();slicebbs.pop_front();
        func->basicblock.insert(it, needInsertBB);
    }
    //删除
    // cout<<bb->name<<endl;
    func->basicblock.remove(bb);
}
void InsertInlineFunc(RawFunction *func,deque<RawBasicBlock *> &slicebbs,int inlinefunc_idx,RawBasicBlock* fbb){
    unordered_map<RawBasicBlock *,RawBasicBlock *> copybbs;
    bool HasCopyFirst =false; 
    for(auto bb:func->basicblock){
        RawBasicBlock * copyBlock = new RawBasicBlock(func->name,bb->name,inlinefunc_idx);
        copybbs[bb]=copyBlock;
        if(!HasCopyFirst){//内联函数的起始基本块
            //插入跳转语句
            RawValue * jumpValue = new RawValue();
            jumpValue->value = new ValueKind();
            jumpValue->value.tag =RVT_JUMP;
            jumpValue->value.jump.target = copyBlock;
            slicebbs.back()->inst.push_back(jumpValue);
            copyBlock->pbbs.push_back(slicebbs.back());//保持前驱

            slicebbs.back()->fbbs.push_back(copyBlock);//更新内联点前的block的后继
            HasCopyFirst = true;
        }
        slicebbs.push_back(copyBlock);
    }
    //更新插入的block的cfg
    for(auto copybb:copybbs){
        RawBasicBlock * oldBlock = copybb.first;
        RawBasicBlock * newBlock = copybb.second;
        //更新前驱
        for(auto oldpbb:oldBlock->pbbs){
            newBlock->pbbs.push_back(copybbs[oldpbb]);
        }
        //更新后继
        for(auto oldfbb:oldBlock->fbbs){
            newBlock->fbbs.push_back(copybbs[oldfbb]);
        }
    }
    //内联value
    InlineValue(copybbs,fbb,func);
}
//对内联block的value进行复制
void InlineValue(unordered_map<RawBasicBlock *,RawBasicBlock *> copybbs,RawBasicBlock* fbb,RawFunction *func){
    unordered_map<RawValue*,RawValue*>copyValues;
    //复制value
    for(auto copybb:copybbs){
        RawBasicBlock * oldBlock = copybb.first;
        RawBasicBlock * newBlock = copybb.second;
        for(auto value:oldBlock->inst){
            RawValue * copyValue = new RawValue(value);
            copyValues[value]=copyValue;
            newBlock->inst.push_back(copyValue);
        }
    }
    //改正结构
    for(auto copybb:copybbs){
        RawBasicBlock * oldBlock = copybb.first;
        RawBasicBlock * newBlock = copybb.second;
        list<RawValue*>needDel;
        for(auto value:oldBlock->inst){
            RawValue * copyValue = copyValues[value];
            uint32_t tag = value->value.tag;
            switch(tag){
                // case :{

                //     break;
                // }
                case RVT_INTEGER:{
                    copyValue->value.integer.value = value->value.integer.value;
                    break;
                }
                case RVT_FLOAT:{
                    copyValue->value.floatNumber.value = value->value.floatNumber.value;
                    break;
                }
                case RVT_CONVERT:{
                    // copyValue->value.Convert.src =
                    if(InlineShareVar.count((RawValue*)value->value.Convert.src)!=0){
                        copyValue->value.Convert.src=InlineShareVar[(RawValue*)value->value.Convert.src];
                    }
                    else if(copyValues.count((RawValue*)value->value.Convert.src)!=0)
                        copyValue->value.Convert.src=copyValues[(RawValue*)value->value.Convert.src];
                    else
                        copyValue->value.Convert.src=value->value.Convert.src;
                    break;
                }
                case RVT_ALLOC:{
                    needDel.push_back(copyValues[value]);
                    break;
                }
                case RVT_LOAD:{
                    if(InlineShareVar.count((RawValue*)value->value.load.src)!=0){
                        copyValue->value.load.src=InlineShareVar[(RawValue*)value->value.load.src];
                        // cout<<value->value.load.src->name<<endl;
                    }
                    else if(copyValues.count((RawValue*)value->value.load.src)!=0)
                        copyValue->value.load.src=copyValues[(RawValue*)value->value.load.src];
                    else
                        copyValue->value.load.src=value->value.load.src;
                    break;
                }
                case RVT_STORE:{
                    if(InlineShareVar.count((RawValue*)value->value.store.dest)!=0){
                        copyValue->value.store.dest = InlineShareVar[(RawValue*)value->value.store.dest];
                        // cout<<value->value.store.dest->name<<endl;
                    }
                    else if(copyValues.count((RawValue*)value->value.store.dest)!=0)
                        copyValue->value.store.dest = copyValues[(RawValue*)value->value.store.dest];
                    else
                        copyValue->value.store.dest = value->value.store.dest;
                        
                    if(copyValues.count((RawValue*)value->value.store.value)!=0)
                        copyValue->value.store.value = copyValues[(RawValue*)value->value.store.value];
                    else if(InlineShareVar.count((RawValue*)value->value.store.value)!=0){
                        RawValue * loadValue = new RawValue(value);
                        loadValue->value.tag = RVT_LOAD;
                        loadValue->value.load.src = InlineShareVar[(RawValue*)value->value.store.value];
                        newBlock->inst.push_front(loadValue);
                        // copyValue->value.store.value = InlineShareVar[(RawValue*)value->value.store.value];
                        copyValue->value.store.value = loadValue;
                        
                    }   
                    else
                        copyValue->value.store.value = value->value.store.value;
                    break;
                }
                case RVT_BINARY:{
                    if(copyValues.count((RawValue*)value->value.binary.lhs)!=0)
                        copyValue->value.binary.lhs = copyValues[(RawValue*)value->value.binary.lhs];
                    else{
                        RawValue* loadValue = new RawValue(value);
                        loadValue->value.tag = RVT_LOAD;
                        loadValue->value.load.src=InlineShareVar[(RawValue*)value->value.binary.lhs];
                        copyValue->value.binary.lhs = loadValue;
                        newBlock->inst.push_front(loadValue);
                    }
                    copyValue->value.binary.op = value->value.binary.op;
                    if(copyValues.count((RawValue*)value->value.binary.rhs)!=0)
                        copyValue->value.binary.rhs = copyValues[(RawValue*)value->value.binary.rhs];
                    else{
                        RawValue* loadValue = new RawValue(value);
                        loadValue->value.tag = RVT_LOAD;
                        loadValue->value.load.src=InlineShareVar[(RawValue*)value->value.binary.rhs];
                        copyValue->value.binary.rhs = loadValue;
                        newBlock->inst.push_front(loadValue);
                    }
                    break;
                }
                case RVT_RETURN:{
                    if(value->value.ret.value!=nullptr){//有返回值的时候需要store
                        RawValue * storeValue = new RawValue(value);
                        storeValue->value.tag = RVT_STORE;
                        storeValue->value.store.dest = RetValue[func];
                        if(copyValues.count((RawValue*)value->value.ret.value)!=0){
                            storeValue->value.store.value = copyValues[(RawValue*)value->value.ret.value];
                        }
                        else if(InlineShareVar.count((RawValue*)value->value.ret.value)!=0){
                            RawValue* loadValue = new RawValue(value);
                            loadValue->value.tag = RVT_LOAD;
                            loadValue->value.load.src=InlineShareVar[(RawValue*)value->value.ret.value];
                            storeValue->value.store.value = loadValue;
                            newBlock->inst.insert(--newBlock->inst.end(),loadValue);
                        }
                        newBlock->inst.insert(--newBlock->inst.end(),storeValue);
                    }
                    fbb->pbbs.push_back(newBlock);
                    newBlock->fbbs.push_back(fbb);
                    copyValue->value.tag = RVT_JUMP;
                    copyValue->value.jump.target = fbb;
                    break;
                }
                case RVT_BRANCH:{
                    copyValue->value.branch.cond = copyValues[(RawValue*)value->value.branch.cond];
                    copyValue->value.branch.true_bb = copybbs[(RawBasicBlock*)value->value.branch.true_bb];
                    copyValue->value.branch.false_bb = copybbs[(RawBasicBlock*)value->value.branch.false_bb];
                    break;
                }
                case RVT_JUMP:{
                    copyValue->value.jump.target = copybbs[(RawBasicBlock*)value->value.jump.target];
                    break;
                }
                case RVT_CALL:{
                    copyValue->value.call.callee = value->value.call.callee;
                    copyValue->value.call.args.clear();
                    for(auto arg:value->value.call.args){
                        copyValue->value.call.args.push_back(copyValues[(RawValue*)arg]);
                    }
                    break;
                }
                case RVT_FUNC_ARGS:{
                    assert(false);
                    break;
                }
                case RVT_GLOBAL:{
                    assert(false);
                }
                case RVT_GET_PTR:{
                    copyValue->value.getptr.src = copyValues[(RawValue*)value->value.getptr.src];
                    copyValue->value.getptr.index = value->value.getptr.index;
                    break;
                }
                case RVT_GET_ELEMENT:{
                    copyValue->value.getelement.index = value->value.getelement.index;
                    copyValue->value.getelement.src = copyValues[(RawValue*)value->value.getelement.src];
                    break;
                }
                case RVT_AGGREGATE:{
                    assert(false);
                    break;
                }
                default:{
                    // newBlock->inst.remove(copyValue);
                    assert(false);
                }
            }
        }
        for(auto value:needDel){
            newBlock->inst.remove(value);
        }
    }
    //改正use和def
    for(auto copybb:copybbs){
        RawBasicBlock * newBlock = copybb.second;
        for(auto value:newBlock->inst){
            uint32_t tag = value->value.tag;
            switch(tag){
                // case :{

                //     break;
                // }
                case RVT_LOAD:{// %1 = load src
                    MarkUse((RawValue*)value->value.load.src,value);
                    MarkDef(value,value);
                    newBlock->uses.insert((RawValue*)value->value.load.src);
                    break;
                }
                case RVT_CONVERT:{
                    MarkUse((RawValue*)value->value.Convert.src,value);
                    MarkDef(value,value);
                    newBlock->uses.insert((RawValue*)value->value.Convert.src);
                    break;
                }
                case RVT_STORE:{//store %1, @x
                    MarkUse((RawValue*)value->value.store.value,value);
                    MarkDef((RawValue*)value->value.store.dest,value);
                    newBlock->uses.insert((RawValue*)value->value.store.value);
                    break;
                }
                case RVT_BINARY:{// %1 = add %2,%3
                    MarkUse((RawValue*)value->value.binary.lhs,value);
                    MarkUse((RawValue*)value->value.binary.rhs,value);
                    MarkDef(value,value);
                    newBlock->uses.insert((RawValue*)value->value.binary.lhs);
                    newBlock->uses.insert((RawValue*)value->value.binary.rhs);
                    break;
                }
                case RVT_RETURN:{
                    MarkUse((RawValue*)value->value.ret.value,value);
                    newBlock->uses.insert((RawValue*)value->value.ret.value);
                    break;
                }
                case RVT_BRANCH:{
                    MarkUse((RawValue*)value->value.branch.cond,value);
                    newBlock->uses.insert((RawValue*)value->value.branch.cond);
                }
                default:
                    break;
            }
        }
    }
}