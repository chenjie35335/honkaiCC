#include "../../../include/backend/Generator/generator.h"
#include "../../../include/backend/hardware/HardwareManager.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <random>
#include<bits/stdc++.h>
using namespace std;
extern HardwareManager hardware;

int checkuse(RawValue * y,RawValueP xx,int op);
int ValueArea::getTargetOffset(const RawValueP &value) const
{
    if (StackManager.find(value) != StackManager.end())
        return StackManager.at(value);
    else
        assert(0);
}
/// @brief 返回
/// @param value
/// @return
int calArrLen(const RawTypeP &value) {
    assert(value->tag == RTT_ARRAY);
    auto ElemTag = value->array.base->tag;
    if (ElemTag == RTT_INT32 || ElemTag == RTT_FLOAT)
        return value->array.len * 4;
    else if (ElemTag == RTT_ARRAY)
        return calArrLen(value->array.base) * value->array.len;
    else
        return 0;
}
/// @brief 这个是计算数组的一个单元的大小
/// @param value 
/// @return 
int calBaseLen(const RawValueP &value){
    auto PointerTy = value->ty->pointer.base;
    auto PointerTyTag = PointerTy->tag;
    if (PointerTyTag == RTT_INT32 || PointerTyTag == RTT_FLOAT)
        return 4;
    else if (PointerTyTag == RTT_ARRAY)
        return calArrLen(PointerTy);
    else assert(0);
}

/// @brief //这个是单纯计算alloc需要分配的空间
/// @param value
/// @return
int calAllocLen(const RawValueP &value)
{
    auto PointerTy = value->ty->pointer.base;
    auto PointerTyTag = PointerTy->tag;
    if (PointerTyTag == RTT_INT32 || PointerTyTag == RTT_FLOAT)
        return 4;
    else if (PointerTyTag == RTT_ARRAY)
        return calArrLen(PointerTy);
    else if(PointerTyTag == RTT_POINTER)
        return 8;
    else assert(0);
}
// 这里需要修改
void calculateSize(int &ArgsLen, int &LocalLen, int &ReserveLen, const RawFunctionP &function)
{
    bool has_call;
    auto &params = function->params; // 给所有的参数分配空间
    LocalLen += 8 * params.size();
    for (auto bb : function->basicblock)
    {
        for (auto value : bb->inst)
        {
            if (value->value.tag == RVT_ALLOC)
            { // alloc 指令分配的内存,大小为4字节
                int len = calAllocLen(value);
                // cout << "save len =" << len << endl;
                LocalLen += len + 8; // 这里给每个指针值加上一个4字节用于存储指针
                hardware.SaveLen(value, len);
            }
            else if (value->ty->tag != RTT_UNIT)
            { // 指令的类型不为unit, 存在返回值，分配内存
                LocalLen += 8;
            }
            if (value->value.tag == RVT_CALL)
            {
                has_call = true;
                ArgsLen = max(ArgsLen, int(value->value.call.args.size() - 8)) * 8;
            }
        }
    }
    ReserveLen = 14 * 8; // 无论有没有，这个我们都保存一下返回地址
    // cout << "Args=" <<  ArgsLen << ",Local=" << LocalLen << ",Reserve=" << ReserveLen << endl;
}

int HardwareManager::init(const RawFunctionP &function)
{
    int ArgsLen = 0, LocalLen = 0, ReserveLen = 0;
    calculateSize(ArgsLen, LocalLen, ReserveLen, function);
    int StackLen = ArgsLen + LocalLen + ReserveLen;
    StackLen = (StackLen + 15) / 16 * 16;
    memoryManager.initStack(StackLen);
    int ArgsMin = 0, ArgsMax = ArgsLen - 8;
    memoryManager.initArgsArea(ArgsMin, ArgsMax);
    int LocalMin = ArgsMax + 8, LocalMax = LocalMin + LocalLen - 8;
    memoryManager.initLocalArea(LocalMin, LocalMax);
    int ReserveMin = LocalMax + 8, ReserveMax = ReserveMin + ReserveLen - 8;
    memoryManager.initReserveArea(ReserveMin, ReserveMax);
    registerManager.init();
    return StackLen;
}

void MemoryManager::initArgsArea(int min, int max)
{
    // cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    this->argsArea.minAddress = min;
    this->argsArea.maxAddress = max;
    this->argsArea.tempOffset = min;
    this->argsArea.StackManager.clear();
}

void MemoryManager::initReserveArea(int min, int max)
{
    // cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    auto &ReserveArea = this->reserveArea;
    ReserveArea.minAddress = min;
    ReserveArea.maxAddress = max;
    ReserveArea.tempOffset = max;
    ReserveArea.StackManager.clear();
}

void MemoryManager::initLocalArea(int min, int max)
{
    // cout << "minAddress = " << min << ",maxAddress = " << max << endl;
    auto &LocalArea = this->localArea;
    LocalArea.minAddress = min;
    LocalArea.maxAddress = max;
    LocalArea.tempOffset = min;
    reserveArea.StackManager.clear();
}

void HardwareManager::LoadFromMemory(const RawValueP &value,int id)
{
    AllocRegister(value,id);
    // string reg = GetRegister(value,id);
    // int TargetOffset = getTargetOffset(value);
    // cout << "  lw  " << reg << ", " << TargetOffset << "(sp)" << endl;
}
int m=0;
vector<RawValueP> midl;
void check(RawValueP y,map<RawValueP,int>&vdef){
                if(vdef[y]) return;
                uint32_t e=(y->ty->tag);
                if(e==RTT_INT32||e==RTT_POINTER){
                    if(y->value.tag==RVT_ALLOC){
                        return;
                    }
                    vdef[y]=1;
                    midl.push_back(y);
                  }
                  else{
                    auto x= y->value.tag; 
                        switch(x){
                            case RVT_ALLOC:{
                                break;
                            }
                            case RVT_LOAD:{
                                auto qq=(y->value.load.src);
                                check(qq,vdef);
                                break;
                            }
                            case RVT_STORE:{
                                auto qq=(y->value.store.value);    
                                check(qq,vdef);
                                qq=(y->value.store.dest);
                                check(qq,vdef);
                                break;
                            }
                            case RVT_RETURN:{
                                auto qq=(y->value.ret.value);
                                check(qq,vdef);
                                break;
                            }
                            case RVT_BINARY:{
                                auto qq=(y->value.binary.lhs);
                                auto qqq=(y->value.binary.rhs);
                                check(qq,vdef);check(qqq,vdef);
                                break;
                            }
                            case RVT_BRANCH:{
                                auto qq=(y->value.branch.cond);
                                check(qq,vdef);
                                list<RawValue *> lst=y->value.branch.true_bb->inst;
                                for(auto it :lst){
                                    check(it,vdef);
                                }
                                lst=y->value.branch.false_bb->inst;
                                for(auto it :lst){
                                    check(it,vdef);
                                }
                                break;
                                //block 处理
                            }
                            case RVT_JUMP:{
                                //block
                                list<RawValue *> lst=(y->value.jump.target->inst);
                                for(auto it:lst){
                                    check(it,vdef);
                                }
                                break;
                            }
                            case RVT_CALL:{
                                //func
                                RawFunctionP f=y->value.call.callee;
                                for(auto it:(f->params)) check(it,vdef);
                                for(auto it:(f->values)) check(it,vdef);
                                list<RawBasicBlock *> lst=(f->basicblock);
                                for(auto bb:lst){
                                    for(auto it:(bb->inst))
                                    check(it,vdef);
                                }
                                //vec
                                vector<RawValue *> vec=y->value.call.args;
                                for(auto  it:vec) check(it,vdef);
                                break;
                            }
                            case RVT_GET_PTR:{
                                auto qq=(y->value.getptr.index);
                                auto qqq=(y->value.getptr.src);
                                check(qq,vdef);check(qqq,vdef);
                                break;
                            }
                            case RVT_GET_ELEMENT:{
                                auto qq=(y->value.getelement.src);
                                auto qqq=(y->value.getelement.index);
                                check(qq,vdef);check(qqq,vdef);
                                break;
                            }
                            case RVT_AGGREGATE:{
                                //vec
                                vector<RawValue *> vec=y->value.aggregate.elements;
                                for(auto  it:vec) check(it,vdef);
                                break;
                            }
                            case RVT_VALUECOPY:{
                                auto qq=(y->value.valueCop.target);
                                check(qq,vdef);
                                break;
                            }
                            default:{
                                cout << "unknown kind: " << y->value.tag << endl;
                                assert(false); 
                            }
                        }
                  }
            }

const int M=50005,N=25;
vector<RawValueP> def[M],use[M];
    map<RawValueP,int> mp;
     int cnt=0;
void make_def_use(vector<RawBasicBlockP> bbbuffer){
    for(auto bb:bbbuffer){
        for(auto it:bb->inst){
            int xx=it->ty->tag;
            if(xx==0||xx==4){
                def[mp[it]].push_back(it);
            }
            auto x= it->value.tag;
            auto y=it;
                        switch(x){
                            case RVT_INTEGER:{
                                break;
                            }
                            case RVT_FLOAT:{
                                break;
                            }
                            case RVT_ALLOC:{
                                break;
                            }
                            case RVT_LOAD:{
                                auto qq=(y->value.load.src);
                                if(qq->ty!=NULL){
                                    if(qq->ty->tag==0||qq->ty->tag==4) use[mp[it]].push_back(qq);
                                    if(!mp[qq]){
                                        mp[qq]=++cnt;
                                        def[cnt].clear();use[cnt].clear();
                                        def[mp[it]].push_back(qq);
                                    }
                                }
                                
                                break;
                            }
                            case RVT_STORE:{
                                auto qq=(y->value.store.value);    
                                auto qqq=(y->value.store.dest);
                                if(qq->ty!=NULL){
                                    if(qq->ty->tag==0||qq->ty->tag==4) use[mp[it]].push_back(qq);
                                    if(!mp[qq]){
                                        mp[qq]=++cnt;
                                        def[cnt].clear();use[cnt].clear();
                                        def[mp[it]].push_back(qq);
                                    }
                                }
                                
                                if(qqq->ty!=NULL){
                                if(!mp[qqq]){
                                        mp[qqq]=++cnt;
                                        def[cnt].clear();use[cnt].clear();
                                        def[mp[it]].push_back(qqq);
                                    }
                                if(qqq->ty->tag==0||qqq->ty->tag==4) use[mp[it]].push_back(qqq);
                                }
                                break;
                            }
                            case RVT_RETURN:{
                                auto qq=(y->value.ret.value);
                                if(qq!=NULL&&qq->ty!=NULL){
                                if(qq->ty->tag==0||qq->ty->tag==4) use[mp[it]].push_back(qq);
                                }
                                break;
                            }
                            case RVT_BINARY:{
                                auto qq=(y->value.binary.lhs);
                                auto qqq=(y->value.binary.rhs);
                                if(qq->ty->tag==0||qq->ty->tag==4) use[mp[it]].push_back(qq);
                                if(qqq->ty->tag==0||qqq->ty->tag==4) use[mp[it]].push_back(qqq);
                                break;
                            }
                            case RVT_BRANCH:{
                                auto qq=(y->value.branch.cond);
                                if(qq->ty->tag==0||qq->ty->tag==4) use[mp[it]].push_back(qq);
                                break;
                                //block 处理
                            }
                            case RVT_JUMP:{
                                break;
                            }
                            case RVT_CALL:{
                                for(auto itt:y->value.call.args){
                                    use[mp[it]].push_back(itt);
                                }
                                break;
                            }
                            case RVT_GET_PTR:{
                                auto qq=(y->value.getptr.index);
                                auto qqq=(y->value.getptr.src);
                                use[mp[it]].push_back(qq);
                                use[mp[it]].push_back(qqq);
                                if(!mp[qq]){
                                        mp[qq]=++cnt;
                                        def[cnt].clear();use[cnt].clear();
                                        def[mp[it]].push_back(qq);
                                    }
                                if(!mp[qqq]){
                                        mp[qqq]=++cnt;
                                        def[cnt].clear();use[cnt].clear();
                                        def[mp[it]].push_back(qqq);
                                    }
                                break;
                            }
                            case RVT_GET_ELEMENT:{
                                auto qq=(y->value.getelement.src);
                                auto qqq=(y->value.getelement.index);
                                 use[mp[it]].push_back(qq);
                                 use[mp[it]].push_back(qqq); 
                                 if(!mp[qq]){
                                        mp[qq]=++cnt;
                                        def[cnt].clear();use[cnt].clear();
                                        def[mp[it]].push_back(qq);
                                    }
                                 if(!mp[qqq]){
                                        mp[qqq]=++cnt;
                                        def[cnt].clear();use[cnt].clear();
                                        def[mp[it]].push_back(qqq);
                                    }
                                break;
                            }
                            case RVT_AGGREGATE:{
                                break;
                            }
                            default:{
                                cout << "unknown kind: " << y->value.tag << endl;
                                assert(false); 
                            }
                        }
        }
    }
}
int eq(vector<RawValueP> x,vector<RawValueP> y){
    if(x.size()!=y.size()) return 1;
    sort(x.begin(),x.end());
    sort(y.begin(),y.end());
    for(int i=0;i<x.size();i++){
        if(x[i]!=y[i]) return 1;
    }
    return 0;
}
int OK=0;
int HardwareManager::struct_graph(vector<RawBasicBlockP> &bbbuffer,int id,vector<RawValue*> &cuf){
    for(int i=0;i<200000;i++){
        registerManager.g[i].clear();
    }
    registerManager.n=0;
    vector<RawValueP> in[M],out[M],lin[M],lout[M];
    mp.clear();
    vector<RawValueP> nxt[M];
    int ko=0;
    RawValueP yy;
    cnt=0;
    for(auto it:cuf){
        if(it->value.funcArgs.index>=8) break;
  //      cout<<it->value.funcArgs.index<<endl;
        cnt++;
        mp[it]=cnt;
        def[cnt].clear();use[cnt].clear();
        def[cnt].push_back(it);
    }
    // cout<<"!!!"<<endl;
    // int res=0;
    for(auto bb:bbbuffer){
        auto &insts=bb->inst;
        for(auto it=insts.begin();it!=insts.end();it++){
            if(!ko){
                ko=1;yy=*it;
            }
            // if((*it)->value.tag==RVT_GLOBAL){
            //     cout<<(*it)->ty->tag<<endl;
            //     cout<<"??"<<endl;
            // }
            mp[*it]=cnt;
            // if((*it)->value.tag==RVT_GET_ELEMENT){
            //     res++;
            //     if(cnt==423)
            //     cout<<res<<"!"<<endl;
            //     // cout<<mp[(*it)->value.getelement.index]<<" "<<mp[(*it)->value.getelement.src]<<endl;
            //     // cout<<(*it)->value.getelement.src->value.tag<<"?"<<endl;
            // }
            def[cnt].clear();use[cnt].clear();
            cnt++;
        }
    }
    make_def_use(bbbuffer);
    // for(auto i=cuf.begin();i!=cuf.end();i++){
    //     auto e=i;
    //     e++;
    //     if(e!=cuf.end())
    //     nxt[mp[*i]].push_back(*e);
    //     if(e==cuf.end()){
    //         nxt[mp[*i]].push_back(yy);
    //     }
    // }
    // int count=8;
     for(auto bb:bbbuffer){
        auto &insts=bb->inst;
        for(auto it=insts.begin();it!=insts.end();it++){
                // cout<<mp[*it]<<":"<<((*it)->ty->tag)<<endl;
                // cout<<"DEF:";
                // for(auto it:def[mp[*it]]) cout<<mp[it]<<" ";
                // cout<<endl;
                // cout<<"USE:";
                // for(auto it:use[mp[*it]]) cout<<mp[it]<<" ";
                // cout<<endl;
                // cout<<"!!!"<<endl;
            auto j=it;
            j++;
            int x=mp[*it];
            //求后继
            uint32_t flg=(*it)->value.tag;
            if(flg==RVT_JUMP){
                auto bbb=(*it)->value.jump.target;
                nxt[x].push_back(*(bbb->inst.begin()));
            }
            else if(flg==RVT_BRANCH){
                auto X=(*it)->value.branch.cond;
                auto b1=((*it)->value.branch.true_bb);
                auto b2=(*it)->value.branch.false_bb;
                auto e1=b1->inst.begin();
                auto e2=b2->inst.begin();
                nxt[x].push_back(*e1);
                nxt[x].push_back(*e2);
            }
            // else if(flg==RVT_CALL){
            //     auto x1=(*it)->value.call.callee;
            //     auto xx=x1->basicblock;
            //     if(xx.begin()!=xx.end()){
            //         auto y=*(xx.begin());
            //         auto e=y->inst.begin();
            //         nxt[x].push_back((*e));
            //     }
            // }
            else if(flg==RVT_RETURN){
                ;
            }
                else if(j!=insts.end()&&flg!=RVT_RETURN)
                nxt[x].push_back(*(j));
        }
    }

    cnt=0;
    //扫描IR 假設bbbufer是存放基本快的vector
    int changes=1;
    while(changes){
        changes=0;
        map<RawValueP,int>vdef;
        for(int i=0;i<bbbuffer.size();i++){
            auto & insts=(bbbuffer[i]->inst);
            for(auto it=insts.begin();it!=insts.end();it++){
                auto x=*it;
            int c1=mp[x];
            vector<RawValueP> toc=lout[c1];
            lout[c1].clear();
            map<RawValueP,bool> V;
            for(auto p:nxt[c1]){
                int c2=mp[p];
//                hb(out[c1],in[c2]);
                for(int j=0;j<in[c2].size();j++)if(!V[in[c2][j]]){
                    lout[c1].push_back(in[c2][j]);
                    V[in[c2][j]]=1;
                }
            }

                vector<RawValueP> tmp; //暂存in[c1]
                tmp=lin[c1];

                //判断是否为整型
                map<RawValueP,int> ivis;
                for(auto q:(def[mp[x]])){ 
                    midl.clear();
                    check(q,vdef);
                    for(auto it :midl) ivis[it]=1;
                }
                lin[c1].clear();
                for(auto q:out[c1]){
                    midl.clear();
                    check(q,vdef);
                    for(auto itt:midl)if(!ivis[itt]){
                        ivis[itt]=1;
                    lin[c1].push_back(itt);
                }
                }
                vdef.clear();
                for(auto q:(use[mp[x]])){
                    midl.clear();
                    check(q,vdef);
                    for(auto itt:midl)if(!ivis[itt]){
                    lin[c1].push_back(itt);
                    ivis[itt]=1;
                }
                }//||lout[c1]!=toc
        }
    }
    int res=0;
    for(int i=0;i<bbbuffer.size();i++){
            auto & insts=(bbbuffer[i]->inst);
            for(auto it=insts.begin();it!=insts.end();it++){
                int e=eq(in[mp[*it]],lin[mp[*it]]);
                e|=eq(out[mp[*it]],lout[mp[*it]]);
                if(e) changes=1;
                in[mp[*it]]=lin[mp[*it]];
                out[mp[*it]]=lout[mp[*it]];
            }
        }
    }

    int tot=0;
    registerManager.vp[id].clear();
    registerManager.rvp[id].clear();

    for(int i=0;i<bbbuffer.size();i++){
        auto blk=bbbuffer[i];
        for(auto x:bbbuffer[i]->inst){
        int p=mp[x];
        for(auto it:in[p]){
            if(!registerManager.vp[id][it]) registerManager.vp[id][it]=++tot,registerManager.rvp[id][tot]=it; 
            }
        for(auto it:out[p]){
            if(!registerManager.vp[id][it]) registerManager.vp[id][it]=++tot,registerManager.rvp[id][tot]=it; 
            }
        }
    }
    registerManager.n=tot;
    for(int i=1;i<=registerManager.n;i++) registerManager.g[i].clear();
    map<pair<RawValueP,RawValueP> ,int> vis;
    vis.clear();
    for(auto bb:bbbuffer){
        for(auto it:bb->inst){
        int cnt=mp[it];
        for(auto u1:in[cnt]){
            for(auto v1:in[cnt])if(u1!=v1){
                if(vis[{u1,v1}]) continue;
                int u=registerManager.vp[id][u1],v=registerManager.vp[id][v1];
                registerManager.g[u].push_back(v);
                registerManager.g[v].push_back(u);
                vis[{u1,v1}]=vis[{v1,u1}]=1;
              }
            }
        for(auto u1:out[cnt]){
            for(auto v1:out[cnt])if(u1!=v1){
                if(vis[{u1,v1}]) continue;
                int u=registerManager.vp[id][u1],v=registerManager.vp[id][v1];
                registerManager.g[u].push_back(v);
                registerManager.g[v].push_back(u);
                vis[{u1,v1}]=vis[{v1,u1}]=1;
               }
            }
        }
    }

    // cout<<tot<<endl;
    // for(int i=1;i<=tot;i++){
    //     cout<<i<<":";
    //     for(auto it:registerManager.g[i]) cout<<it<<" ";
    //     cout<<endl;
    // }
    // cout<<"!!!"<<endl;
   
    // for(int i=0;i<=registerManager.n;i++){
    //     for(auto it:registerManager.g[i]) cout<<it<<" ";
    //     cout<<endl;
    // }
    // cout<<"!!!"<<endl;
    InitallocReg(bbbuffer,id,cuf);
    if(!OK){
    HardwareManager::spill(bbbuffer,id,cuf);
    }
    if(OK){
        OK=0;
        return 1;
    }
    return 0;
    
}



int checkuse(RawValue * y,RawValueP xx,int op){
        if(op==1)
        for(auto it:def[mp[y]]){
        if(it==xx) return 1;
    }
    if(op==2)
    for(auto it:use[mp[y]]){
        if(it==xx) return 2;
    }
    return 0;
    //  uint32_t ee=(y->ty->tag);
    //             if(ee==RTT_INT32){
    //                 if((RawValueP)y==xx){
    //                     return op;
    //                 }
    //               }
                  
    //                 uint32_t e=(y->value.tag);
    //                 // cout<<e<<endl;
    //                     switch(e){
    //                         case RVT_INTEGER:{
    //                             return 0;
    //                         }
    //                         case RVT_ALLOC:{
    //                             return 0;
    //                         }
    //                         case RVT_LOAD:{
    //                             return checkuse((RawValue*)y->value.load.src,xx,2);
    //                         }
    //                         case RVT_STORE:{
    //                             return checkuse((RawValue*)y->value.store.value,xx,2)+checkuse((RawValue*)y->value.store.dest,xx,2);
    //                         }
    //                         case RVT_BRANCH:{
    //                             return checkuse((RawValue*)y->value.branch.cond,xx,2);
    //                         }
    //                         case RVT_RETURN:{
    //                             return checkuse((RawValue*)y->value.ret.value,xx,2);
    //                         }
    //                         case RVT_BINARY:{
    //                             auto qq=(y->value.binary.lhs);
    //                             auto qqq=(y->value.binary.rhs);
    //                             int sum=0;
    //                             sum=checkuse((RawValue *)qqq,xx,2)+checkuse((RawValue *)qq,xx,2);
    //                             return sum;
    //                         }
    //                         case RVT_JUMP:{
    //                             //块内处理
    //                             // list<RawValue *> lst=(y->value.jump.target->inst);
    //                             // solve(lst,xx);
    //                             return 0;
    //                         }
    //                         case RVT_CALL:{
    //                             RawFunctionP f=y->value.call.callee;
    //                             int sum=0;
    //                             for(auto it:(f->params)){
    //                                 sum+=checkuse(it,xx,2);
    //                             }
    //                             return sum;
    //                             // //func内处理：如何处理？
                                
                                
    //                             // // for(auto it:(f->values)) check(it,vdef);
    //                             // list<RawBasicBlock *> lst=(f->basicblock);
    //                             // for(auto bb:lst){
    //                             //     for(auto it:(bb->inst))
    //                             //     solve((bb->inst),xx);
    //                             // }
    //                             // //vec是否处理,如何处理？暂时假定不处理
                                
    //                         }
    //                         case RVT_FUNC_ARGS:{
    //                             return 0;
    //                         }
    //                         // case RVT_GET_ELEMENT:{
    //                         //     auto qq=(y->value.getelement.src);
    //                         //     auto qqq=(y->value.getelement.index);
    //                         //     int sum=checkuse((RawValue *)qq,xx,1)+checkuse((RawValue *)qqq,xx,2);
    //                         //     return sum;
    //                         // }
    //                         // case RVT_VALUECOPY:{
    //                         //     auto qq=(y->value.valueCop.target);
    //                         //     return checkuse((RawValue *)qq,xx,1);
    //                         // }
    //                         default:{
    //                             cout<<"checkusemiss"<<e<<endl;
    //                             exit(0);
    //                         }
    //                     }
}


void chg(RawValueP &y,RawValueP &xx,RawValue* &u){

    RawValue* yy=(RawValue*) y;
                    
                    if(yy==xx){
                        y=u;return;
                    }
                    uint32_t e=(y->value.tag);
                        switch(e){
                            case RVT_ALLOC:{
                                return;
                            }
                            case RVT_INTEGER:{
                                return;
                            }
                            case RVT_LOAD:{
                                auto &src = yy->value.load.src;
                                chg(src,xx,u);
                                return;
                            }
                            case RVT_STORE:{
                                auto &value = yy->value.store.value;
                                auto &dest = yy->value.store.dest;
                                chg(value,xx,u);
                                chg(dest,xx,u);
                                return;
                            }
                            case RVT_RETURN:{
                                auto &ret = yy->value.ret.value;
                                chg(ret,xx,u);
                                return;
                            }
                            case RVT_BINARY:{
                                auto &qq=(yy->value.binary.lhs);
                                auto &qqq=(yy->value.binary.rhs);
                                int sum=0;
                                chg(qq,xx,u);
                                chg(qqq,xx,u);
                                return;
                            }
                            case RVT_BRANCH:{
                                auto &cond = yy->value.branch.cond;
                                return chg(cond,xx,u);
                            }
                            case RVT_JUMP:{
                                return;
                            }
                            case RVT_CALL:{
                                for(auto &it:yy->value.call.args){
                                    if(it==xx){
                                        it=u;
                                    }
                                }
                                return ;
                            }
                            case RVT_FUNC_ARGS:{
                                // cout<<"ARGS"<<endl;
                                return;
                            }
                            case RVT_GET_ELEMENT:{
                                auto &qq=(yy->value.getelement.src);
                                auto &qqq=(yy->value.getelement.index);
                                chg(qq,xx,u);
                                chg(qqq,xx,u);
                                return;
                            }
                            case RVT_GET_PTR:{
                                auto &qq=(yy->value.getptr.src);
                                auto &qqq=(yy->value.getptr.index);
                                chg(qq,xx,u);
                                chg(qqq,xx,u);
                                return;
                            }
                            // case RVT_VALUECOPY:{
                            //     auto qq=(y->value.valueCop.target);
                            //     return checkuse((RawValue *)qq,xx,1);
                            // }
                            default:{
                                cout<<e<<endl;
                                exit(0);
                            }
                        
                  }
}

extern SignTable signTable;
int pos=1,mx=0;
void HardwareManager::spill(vector<RawBasicBlockP> &bbbuffer,int id,vector<RawValue*> &cuf){
    const int N=5;
    //  registerManager.Ccolor=registerManager.n;
    // if(registerManager.Ccolor<=N) return;
    pos=1,mx=0;
    for(int i=1;i<=m;i++){
        if(registerManager.g[i].size()>mx){
 //           if(nf[registerManager.rvp[i]]) continue;
            mx=registerManager.g[i].size();
            pos=i;
        }
    }
    //delete pos,and insert "load" and "store" to the position pos at.
    //需添加value所属block集合的map 假定为v_b
    // map<RawValueP,vector<RawBasicBlock*>> v_b;//改为*
    // vector<RawBasicBlock*> detb=v_b[registerManager.rvp[pos]];//被修改值所在块

    //找到所有修改值所在位置，如果是左值则在后加store，右值则在前加load，然后将指令修改为新值
    //checkuse找到其是左值1还是右值2，不存在即0
    RawValueP pvue=registerManager.rvp[id][pos];
    // cout<<mx<<endl;
    // cout<<pvue->value.tag<<endl;
    // cout<<pvue->ty->tag<<endl;
    // if(pvue->value.tag==RVT_GET_ELEMENT)
    // cout<<mp[pvue]<<endl;
    // for(auto it:registerManager.g[pos]){
    //     for(auto i=registerManager.g[it].begin();i!=registerManager.g[it].end();i++){
    //         if(*i==pos){
    //             i=registerManager.g[it].erase(i);
    //             if(i==registerManager.g[it].end()) break;
    //             }
    //     }
    // }
    // registerManager.g[pos].clear();
    //  registerManager.n--;
         RawValue* aloc;
         int okk=1;
    // if(pvue->value.tag==RVT_FUNC_ARGS){
    //     int id= pvue->value.funcArgs.index;
    //     if(1){
    //      RawBasicBlock* bb =(RawBasicBlock*)bbbuffer[0];
    //     auto &insts=bb->inst;
    //     auto start=(bb->inst).begin();
        
    //         RawValue *alloc = new RawValue();
    //         RawType *ty = new RawType();
    //         ty->tag = RTT_POINTER;
    //         RawType *pointerTy = new RawType();
    //         pointerTy->tag = RTT_INT32;
    //         ty->pointer.base = pointerTy;
    //         alloc->ty = (RawTypeP)ty;
    //         alloc->value.tag = RVT_ALLOC;
    //         alloc->name="ARG";
    //         aloc=alloc;

    //         RawValue *store = new RawValue();
    //         RawType *tyy = new RawType();
    //         tyy->tag = RTT_UNIT;
    //         store->ty = (RawTypeP)tyy;
    //         store->name = nullptr;
    //         store->value.tag = RVT_STORE;
    //         store->value.store.value = pvue;
    //         store->value.store.dest = alloc;
    //         RawValue *DestValue = (RawValue*)alloc;
    //         insts.insert(start,store);
    //         store->addr=alloc;
    //         okk=0;
    //         // chg(x,pvue,value);
    //     }
    // }
     //打标记
    int CNT=0,WOK=0;
    for(auto itt:bbbuffer){
        RawBasicBlock* it =(RawBasicBlock*)itt;
        for(auto j=(it->inst).begin();j!=(it->inst).end();j++){
        int pOK=0;
            auto x=*j;
            auto p=j;
            p++;
            if(checkuse(x,pvue,1)==1){
                CNT++;
                //l store
            //造alloc作为dest
            auto &insts = it->inst;
            RawValue *alloc = new RawValue();
            RawType *ty = new RawType();
            ty->tag = RTT_POINTER;
            RawType *pointerTy = new RawType();
            pointerTy->tag = RTT_INT32;
            if(pvue->ty!=NULL)
            ty->pointer.base = pvue->ty;
            else ty->pointer.base=pointerTy;
            alloc->ty = (RawTypeP)ty;
            alloc->value.tag = RVT_ALLOC;
            alloc->name="qqq";
            aloc=alloc;

            //store
            auto &instss = it->inst;
            RawValue *store = new RawValue();
            RawType *tyy = new RawType();
            tyy->tag = RTT_UNIT;
            store->ty = (RawTypeP)tyy;
            store->name = nullptr;
            store->value.tag = RVT_STORE;
            store->value.store.value = pvue;
            store->value.store.dest = alloc;
            it->inst.insert(p,store);
            store->addr=alloc;
            pOK=1;
            j++;
            }
           if(!pOK&&checkuse(x,pvue,2)==2){
                if(CNT==0){
                    WOK=1;
                    cout<<pvue->value.tag<<"uninit!"<<endl;
                    cout<<pvue->ty->tag<<endl;
                    cout<<pvue->name<<endl;
                }
                //r load
            auto &insts = it->inst;
            RawValue * load = new RawValue();
            RawType *tyy = new RawType();
            // if(pvue->ty->tag==RTT_POINTER)
            // tyy->tag=RTT_POINTER;
            // else
            tyy->tag = RTT_POINTER;
            if(aloc->ty!=NULL)
            load->ty = (RawTypeP) aloc->ty->pointer.base;
            else load->ty=(RawTypeP) tyy;
            load->name = "WWw";
            load->value.tag = RVT_LOAD;
            load->value.load.src =aloc;
            it->inst.insert(j,load);
            RawValueP X=(RawValueP)x;
            chg(X,pvue,load);
            }
        }
    }
    //struct graph again
 //   struct_graph(bbbuffer,id,cuf);
    //InitallocReg(bbbuffer,id);
}

void HardwareManager::InitallocReg(vector<RawBasicBlockP> &bbbuffer,int id,vector<RawValue*> &cuf){
        stack<int> q;
        vector<pair<int,int> > in_rank;
         m=registerManager.n;
         registerManager.Ccolor=m;
         
         for(int i=1;i<=m;i++){
            registerManager.vis[id][i]=0;
         }

        //  for(int i=1;i<=m;i++){
        //     RawValueP ee=registerManager.rvp[id][i];
        //     int e=mp[ee];
        //     cout<<e<<":";
        //     cout<<registerManager.g[i].size()<<" ";
        //     for(auto it:registerManager.g[i]){
        //         cout<<it<<" ";
        //     }
        //     cout<<endl;
        // }
        
        for(int i=1;i<=m;i++){
            in_rank.push_back({registerManager.g[i].size(),i});
        }
        int ux[50000]={0};
        sort(in_rank.begin(),in_rank.end());
        int cnt=in_rank.size();
        while(cnt){
            int change=0;
        for(auto [v,x]:in_rank)if(!ux[x]){
            if(v<N){
                ux[x]=1;
                change=1;
                q.push(x);
                for(auto it:registerManager.g[x]){
                    in_rank[it].first--;
                }
                cnt--;
                }
            }
            if(!change){
                
                return;
            }
        }
    while(q.size()){
        int x=q.top();q.pop();
        registerManager.vis[id][x]=1;
        vector<int> w;
        for(auto it:registerManager.g[x])if(registerManager.vis[it]){
            w.push_back(registerManager.vis[id][it]);
        }
        sort(w.begin(),w.end());
        int pos=0;
        while(1){
            if(pos<w.size()&&registerManager.vis[id][x]==w[pos]){
                pos++;registerManager.vis[id][x]++;
            }
            else if(pos<w.size()&&registerManager.vis[id][x]>w[pos]){
                pos++;continue;
            }
            else break;
        }
    }
    OK=1;
}


void HardwareManager::AllocRegister(const RawValueP &value,int id)
{
    // cout << "alloc register for " << value->value.tag << endl;
    // if (registerManager.RegisterFull)
    // {
    //     int RandSelected;
    //     random_device rd;
    //     mt19937 gen(rd());
    //     uniform_int_distribution<int> dis(0, 31);
    //     do
    //     {
    //         RandSelected = dis(gen);
    //     } while (!isValid(RandSelected));
    //     StoreReg(RandSelected);
    //     registerManager.registerLook.insert(pair<RawValueP, int>(value, RandSelected));
    // }
    // else
    // {
    //     uint32_t &RegLoc = registerManager.tempRegister;
    //     registerManager.registerLook.insert(pair<RawValueP, int>(value, RegLoc));
    //     do
    //     {
    //         RegLoc++;
    //     } while (((RegLoc >= 10 && RegLoc <= 17) || registerManager.RegisterLock[RegLoc]) && RegLoc < 32);
    //     if (RegLoc == 32)
    //         registerManager.RegisterFull = true;
    // }
            int RegLoc = registerManager.vis[id][registerManager.vp[id][value]]+1;
         registerManager.registerLook.insert(pair<RawValueP, int>(value, RegLoc));
}

//fregs
void HardwareManager::AllocFRegister(const RawValueP &value)
{
    // cout << "alloc register for " << value->value.tag << endl;
    // if (registerManager.RegisterFull)
    // {
    //     int RandSelected;
    //     random_device rd;
    //     mt19937 gen(rd());
    //     uniform_int_distribution<int> dis(0, 31);
    //     do
    //     {
    //         RandSelected = dis(gen);
    //     } while (!isValid(RandSelected));
    //     StoreFReg(RandSelected);
    //     registerManager.registerLook.insert(pair<RawValueP, int>(value, RandSelected));
    // }
    // else
    // {
    //     uint32_t &RegLoc = registerManager.tempRegister;
    //     registerManager.registerLook.insert(pair<RawValueP, int>(value, RegLoc));
    //     do
    //     {
    //         RegLoc++;
    //     } while (((RegLoc >= 10 && RegLoc <= 17) || registerManager.RegisterLock[RegLoc]) && RegLoc < 32);
    //     if (RegLoc == 32)
    //         registerManager.RegisterFull = true;
    // }
}



void HardwareManager::StoreReg(int RandSelected)
{
    const char *TargetReg;
    int TargetOffset;
    for (const auto &pair : registerManager.registerLook)
    {
        if (pair.second == RandSelected)
        {
            auto value = pair.first;
            auto ty = value->ty;
            TargetReg = RegisterManager::regs[RandSelected];
            if (IsMemory(value))
            {
                TargetOffset = getTargetOffset(pair.first);
            }
            else
            {
                TargetOffset = StackAlloc(pair.first);
            }
            registerManager.registerLook.erase(pair.first);
            // if (value->value.tag == RVT_FUNC_ARGS)
            //     return;
            if (!ty)
                return;
            else if (ty->tag == RTT_ARRAY)
                return;
            else if (ty->tag == RTT_POINTER)
            {
                auto PointerTy = ty->pointer.base;
                auto PointerTag = PointerTy->tag;
                if (PointerTag == RTT_ARRAY)
                    return;
                else{
                        if(TargetOffset > 2047) {
                        cout << "  li   " << "t0, " << TargetOffset << endl;
                        cout << "  add  " << "t0, sp, t0" << endl;
                        cout << "  sd  " <<  TargetReg << ", " << 0 << "(t0)" << endl; 
                    } else{
                    cout << "  sd   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
                        }
                    registerManager.LX.push_back({RandSelected, TargetOffset});
                    (registerManager.LY).push_back(value);
                }
                    
            }
            else if(ty->tag == RTT_INT32){
                if(TargetOffset > 2047) {
                        cout << "  li   " << "t0, " << TargetOffset << endl;
                        cout << "  add  " << "t0, sp, t0" << endl;
                        cout << "  sd  " <<  TargetReg << ", " << 0 << "(t0)" << endl; 
                } else{
                        cout << "  sd   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
                    }
                registerManager.LX.push_back({RandSelected, TargetOffset});
                registerManager.LY.push_back(value);
            }
            else { //float
            return;
                // cout<<ty->tag<<endl;
                // cout << "  fsw!   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
            }
                
            break;
        }
    }
}

void HardwareManager::StoreFReg(int RandSelected)
{
    const char *TargetReg;
    int TargetOffset;
    for (const auto &pair : registerManager.registerLook)
    {
        if (pair.second == RandSelected)
        {
            auto value = pair.first;
            auto ty = value->ty;
            TargetReg = RegisterManager::fregs[RandSelected];
            if (IsMemory(value))
            {
                TargetOffset = getTargetOffset(pair.first);
            }
            else
            {
                TargetOffset = StackAlloc(pair.first);
            }
            registerManager.registerLook.erase(pair.first);
            if (value->value.tag == RVT_FUNC_ARGS)
                return;
            if (!ty)
                return;
            else if (ty->tag == RTT_ARRAY)
                return;
            else if (ty->tag == RTT_POINTER)
            {
                auto PointerTy = ty->pointer.base;
                auto PointerTag = PointerTy->tag;
                if (PointerTag == RTT_ARRAY)
                    return;
                else
                    cout << "  sw   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
            }
            else { //float must be
                cout << "  fsw   " << TargetReg << ", " << TargetOffset << "(sp)" << endl;
            }
                
            break;
        }
    }
}



const char *RegisterManager::regs[32] = {
    "x0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

const char *RegisterManager::fregs[32] = {
    "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",
    "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",
    "f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",
    "f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31"
};
//64位往后貌似不用

const int RegisterManager::callerSave[7] = {
    5, 6, 7, 28, 29, 30, 31};

const int RegisterManager::calleeSave[12] = {
    8, 9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};

void RegisterArea::LoadRegister(int reg)
{
    assert(StackManager.find(reg) != StackManager.end());
    int offset = StackManager.at(reg);
    if (offset <= 2047)
    {
        cout << "  ld  " << RegisterManager::regs[reg] << ", " << offset << "(sp)" << endl;
    }
    else
    {
        cout << "  li  t0," << offset << endl;
        cout << "  add t0, sp, t0" << endl;
        cout << "  ld  " << RegisterManager::regs[reg] << ", " << 0 << "(t0)" << endl;
    }
}

//fregs
void RegisterArea::LoadFRegister(int reg)
{
    assert(StackManager.find(reg) != StackManager.end());
    int offset = StackManager.at(reg);
    if (offset <= 2047)
    {
        cout << "  flw  " << RegisterManager::fregs[reg] << ", " << offset << "(sp)" << endl;
    }
    else
    {
        cout << "  li  t0," << offset << endl;
        cout << "  add t0, sp, t0" << endl;
        cout << "  flw  " << RegisterManager::fregs[reg] << ", " << 0 << "(t0)" << endl;
    }
}


void RegisterArea::SaveRegister(int reg)
{
    if (tempOffset <= 2047)
    {
        cout << "  sd  " << RegisterManager::regs[reg] << ", " << tempOffset << "(sp)" << endl;
    }
    else
    {
        cout << "  li  t0," << tempOffset << endl;
        cout << "  add t0, sp, t0" << endl;
        cout << "  sd  " << RegisterManager::regs[reg] << ", " << 0 << "(t0)" << endl;
    } // 这个方法虽然蠢但是是正确的
    StackManager.insert(pair<int, int>(reg, tempOffset));
    tempOffset -= 8;
}

//fregs
void RegisterArea::SaveFRegister(int reg)
{
    if (tempOffset <= 2047)
    {
        cout << "  fsw  " << RegisterManager::fregs[reg] << ", " << tempOffset << "(sp)" << endl;
    }
    else
    {
        cout << "  li  t0," << tempOffset << endl;
        cout << "  add t0, sp, t0" << endl;
        cout << "  fsw  " << RegisterManager::fregs[reg] << ", " << 0 << "(t0)" << endl;
    } // 这个方法虽然蠢但是是正确的
    StackManager.insert(pair<int, int>(reg, tempOffset));
    tempOffset -= 4;
}

// init要做的事：
/*
1、 给被调用者保存寄存器分配内存空间(这个由于在最上层最后弄)
2、 访问所有参数，确定args的空间
3、 访问所有insts,创建local区域的大小
*/