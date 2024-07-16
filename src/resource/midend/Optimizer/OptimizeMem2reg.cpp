#include "../../../include/midend/Optimizer/OptimizeMem2Reg.h"
//这里我们将要点先行列出
//首先这个phi的形式可能要改了，就是说必须要知道后面每个元素的来源，来源于哪个基本块
//然后是每个基本块后面的那个值可能不一定是copy,也可能是其他类型
//这时候恢复的时候就需要在前面的基本块当中插入store语句
//这时候为了保证正确性，这里alloc可能不能删除，否则整个中端要崩（目的是删掉phi的时候不会出错，其他的mem2reg后就不会出现大的问题了）
//如果不是copy的话，这里就可以直接转为reg不用负任何责任
//控制流图遍历，然后不用管phi函数，就当成一个临时变量
void ClearInst(RawFunction *&function);
mem2regBuilder builder;

RawValue *mem2regBuilder::lookup(RawValue *mem) {
    if(IncomingVals.find(mem) == IncomingVals.end()) {
        cerr << "Could not find mem's reg in programme" << endl;
        assert(false);
    } else return IncomingVals[mem];
}

void InsertAlloc(RawBasicBlock *block) {
    auto &insts = block->inst; 
    for(auto alloc : builder.allocs) {
        insts.push_front(alloc);
    }
}

void ReplaceReg(RawValue *&use,RawValue *reg,RawValue *mem) {
    auto tag = use->value.tag;
    switch(tag) {
        case RVT_RETURN: {
            auto &src = use->value.ret.value;
            if(src == mem) src = reg;
            break;
        }
        case RVT_BINARY: {
            auto &lhs = use->value.binary.lhs;
            auto &rhs = use->value.binary.rhs;
            if(lhs == mem) lhs = reg;
            if(rhs == mem) rhs = reg;
            break;
        }
        case RVT_STORE: {
            auto &src = use->value.store.value;
            if(src == mem) src = reg;
            break;
        }
        case RVT_BRANCH: {//branch这里会有额外的优化，不过目前先不考虑
            auto &cond = use->value.branch.cond;
            if(cond == mem) cond = reg;
            break;
        }
        case RVT_CALL:{
            auto &params = use->value.call.args;
            for(auto &param : params) {
                    if(param == mem) param = reg;
            }
            break;
        }
        case RVT_PHI: {
            auto &phis = use->value.phi.phi;
            for(auto &phi : phis) {
                if(phi.second == mem) phi.second = reg;
            }
        }
        default:
            break;
    }
}

void phi2reg(RawBasicBlock *block){
    for(auto phi : block->phi){
        auto mem = phi;
        auto reg = phi;
        builder.insert(mem,reg);
    }
    for(auto inst : block->inst) {
        auto InstTag = inst->value.tag;
        if(InstTag == RVT_LOAD) {
            auto src = (RawValue *)inst->value.load.src;
            auto SrcTag = src->value.tag;
            //cout << "SrcTag: " << SrcTag << endl;
            if(SrcTag == RVT_PHI) {
                auto reg = builder.lookup(src);
                for(auto use: inst->usePoints){
                    reg->usePoints.push_back(use);
                    ReplaceReg(use,reg,inst);
                }
                
            }
        }
    }
}

void getreg(RawBasicBlock *&block){
    for(auto inst : block->inst){
        auto InstTag = inst->value.tag;
        if(InstTag == RVT_STORE) {
            auto mem = (RawValue *)inst->value.store.dest;
            auto reg = (RawValue *)inst->value.store.value;
            inst->isDeleted = true;
            builder.insert(mem,reg);
        }
    }
}

void mem2reg(RawBasicBlock *&block) {
        for(auto &phi : block->phi) {
            auto &PhiElems = phi->value.phi.phi;
            for(auto &phiElem : PhiElems) {
               if(phiElem.second->value.tag != RVT_VALUECOPY) continue;
               else {
                   auto reg = builder.lookup(phiElem.second);
                   if(reg != nullptr)
                   phiElem.second = reg;
               }
            }
        }
        for(auto inst : block->inst) {//需不需要考虑phi函数？
        //实际上这里我们已经将所有的替换成了copy形式，也就是说一个store其实已经就是定值了，只不过现在需要的就是把使用的部分进行替换
        auto InstTag = inst->value.tag;
        //cout << "InstTag: " << InstTag << endl;
        switch(InstTag) {
            case RVT_LOAD: {
                auto src = (RawValue *) inst->value.load.src;
                auto SrcTag = src->value.tag;
                // cout << "SrcTag: " << SrcTag << endl;
                // if(SrcTag == RVT_PHI) {
                    // cout << "PHI target:" << src->value.phi.target->name << endl;
                // } else if(SrcTag == RVT_VALUECOPY) {
                    // cout << "Valuecopy target:" << src->value.valueCop.target->name << endl;
                // }
                inst->isDeleted = true;
                RawValue *reg = builder.lookup(src);
                //cout << "inst useNumber:" << inst->usePoints.size() << endl;
                for(auto use : inst->usePoints) {
                    //cout << "use tag" << use->value.tag << endl;
                    reg->usePoints.push_back(use);
                    ReplaceReg(use,reg,inst);
                }
                break;
            }
            case RVT_ALLOC: {
                builder.allocs.push_back(inst);
                break;
            }
            default: break;
        }
        }
    }


void mem2reg(RawFunction *func) {//这里我们做了一定的修改以后其实不需要考虑参数的问题了(仅限)
    auto &bbs = func->basicblock;
    if(bbs.empty()) return;
    builder.allocs.clear();
    for(auto &bb : bbs) {
        phi2reg(bb);
    }
    for(auto &bb : bbs) {
        getreg(bb);
    }
    for(auto &bb : bbs) {
       mem2reg(bb);
    }
    InsertAlloc(*bbs.begin());
}

void mem2regTop(RawProgramme *programme){
    auto &funcs = programme->funcs;
    for(auto func : funcs) {
        mem2reg(func);
    }
    for(auto &func : funcs) {
        if(!func->basicblock.empty())
        ClearInst(func);
    }
}
//mem2reg这个地方我的想法是将所有的alloc找出来，放到一个vector里面，然后全部放到entry的顶部
//因为我在前端的时候已经判断过这里是否正确了，因此不需要考虑作用域的问题
//这里这个phi应该怎么处理感觉有点困难
//首先是这个phi先定义后得load出来然后才能使用，也就是说这里将
//这个phi看成了临时变量，这样的话相当于说phi的reg和mem其实都是他自己
//这个地方感觉还是出现了点问题就是访问不完全的问题
//但是其实本质上是这个phi的问题，也就是说我们要用一轮遍历除掉phi的load,将其特殊转为