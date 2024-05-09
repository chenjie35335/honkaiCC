#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <cstring>
#include <unordered_map>
extern SignTable signTable;
void fillAggregate(RawValueP &raw, RawValue *target);
/// @brief 这个是通过查符号表获取的RawValue(包括中间变量和常数)
/// @param value
/// @param sign
void getMidVarValue(RawValueP &value, string &name)
{
    value = (RawValueP) signTable.getMidVar(name);
}
/// @brief 查符号表获取变量左值
/// @param value 
/// @param name 
void getVarValueL(RawValueP &value,string &name) 
{
    value = (RawValueP) signTable.getVarL(name);
}
/// @brief 查符号表获取变量右值
/// @param value 
/// @param name 
void getVarValueR(RawValueP &value,string &name)
{
    value = (RawValueP) signTable.getVarR(name);
}
/// @brief 创建return型value
/// @param value
/// @param src
void generateRawValue(RawValueP src)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue * value = (RawValue *)malloc(sizeof(RawValue));
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    value->name = nullptr;
    value->value.tag = RVT_RETURN;
    value->value.data.ret.value = src;
    value->ty = (RawTypeP) ty;
    insts.buffer[insts.len++] = (const void *)value;
}
/// @brief binary型value
/// @param sign 
/// @param lhs 
/// @param rhs 
/// @param op 
void generateRawValue(string &name, RawValueP lhs, RawValueP rhs, uint32_t op)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue * value = (RawValue *)malloc(sizeof(RawValue));
    value->name = nullptr;
    value->value.tag = RVT_BINARY;
    value->value.data.binary.lhs = lhs;
    value->value.data.binary.rhs = rhs;
    value->value.data.binary.op = op;
    RawType *ty = new RawType();
    ty->tag = RTT_INT32;
    value->ty = (RawTypeP) ty;
    insts.buffer[insts.len++] = (const void *)value;
    signTable.insertMidVar(name,value);
}
/// @brief 
/// @param number 
/// @return 
RawValue * generateNumber(int32_t number) {
    RawValue *value = (RawValue *)malloc(sizeof(RawValue));
    value->name = nullptr;
    value->value.tag = RVT_INTEGER;
    value->value.data.integer.value = number;
    RawType *ty = new RawType();
    ty->tag = RTT_INT32;
    value->ty = ty;
    return value;
}

/// @brief number型value
/// @param number 
void generateRawValue(int32_t number)
{
        auto bb = getTempBasicBlock();
        RawValue * value = generateNumber(number);
        if(bb != nullptr) {
        auto &insts = bb->insts;
        insts.buffer[insts.len++] = (const void *)value;
        }
        signTable.insertNumber(number,value);
}

void generateRawValueArr(int32_t number) {
    RawValue * value = generateNumber(number);
    signTable.insertNumber(number,value);
}

/// @brief store型value
/// @param src 
/// @param dest 
void generateRawValue(RawValueP &src, RawValueP &dest)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *store = (RawValue *)malloc(sizeof(RawValue));
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    store->ty = (RawTypeP)ty;
    store->name = nullptr;
    store->value.tag = RVT_STORE;
    store->value.data.store.value = src;
    store->value.data.store.dest = dest;
    RawValue *DestValue = (RawValue*)dest;
    bb->defs.insert(DestValue);
    insts.buffer[insts.len++] = (const void *)store;
}
/// @brief alloc型value
/// @param sign 
void generateRawValue(string& name)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *alloc = (RawValue *)malloc(sizeof(RawValue));
    RawType *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = RTT_INT32;
    ty->data.pointer.base = pointerTy;
    alloc->ty = (RawTypeP)ty;
    alloc->value.tag = RVT_ALLOC;
    insts.buffer[insts.len++] = (const void *)alloc;
    signTable.insertVar(name,alloc);
}
//这里建立的这个aggregate的type没有任何意义，因为无法判断！
//从当前来看，这个当前是要转换成getptr的，不过考虑全局变量的问题需要特殊对待。这里ty暂时不管
//这个部分属于初始部分，没有补0,所以没有ty没有用处
void generateRawValue(vector<RawValueP>elements,string &sign) {
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *aggregate = (RawValue *)malloc(sizeof(RawValue));
    aggregate->value.tag = RVT_AGGREGATE;
    aggregate->name = nullptr;
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    aggregate->ty = ty;
    auto &elem = aggregate->value.data.aggregate.elements;
    elem.buffer = (const void **) malloc(sizeof(const void *)*500);
    elem.len = 0;
    elem.kind = RSK_BASICVALUE;
    for(auto &element : elements) {
        elem.buffer[elem.len++] = (const void *)element;
    }
    //insts.buffer[insts.len++] = (const void *)aggregate;
    alloc_now++;sign = "%"+to_string(alloc_now);
    signTable.insertMidVar(sign,aggregate);
}

/// @brief 这里递归创建这个ty
/// @param ty 
/// @param dimen 
void generateArrType(RawType *&ty,vector<int> &dimens,int index) {
    assert(ty->tag == RTT_ARRAY);
    if(index >= dimens.size()) {return;}
    else {
        int dimen = dimens[index];
        RawType *subTy = new RawType();
        ty->data.array.base = subTy;
        ty->data.array.len = dimen;
        if(index == dimens.size()-1) {
            subTy->tag = RTT_INT32; return;
        } else {
            subTy->tag = RTT_ARRAY;
            generateArrType(subTy,dimens,index+1);
        }
    }
}

/// @brief globalalloc分配数组
/// @param name 
/// @param dimen 
void generateRawValueArrGlobal(const char *name,vector<int> &dimen,RawValue *&init){
        auto programme = getTempProgramme();
        auto &values = programme->Value;
        RawValue *global = (RawValue *)malloc(sizeof(RawValue));
        RawType *ty = new RawType();
        ty->tag = RTT_ARRAY;
        generateArrType(ty,dimen,0);
        global->ty = (RawTypeP)ty;
        global->value.tag = RVT_GLOBAL;
        global->value.data.global.Init = (RawValueP) init;
        global->name = (char *) malloc(sizeof(char)*50);
        char *sign = (char *)global->name;
        strcpy(sign,name);
        values.buffer[values.len++] = (const void *)global;
        signTable.insertVar(name,global);
}
/// @brief 
/// @param zeroinit 
void generateZeroInit(RawValue *&zeroinit) {
    zeroinit = new RawValue();
    zeroinit->value.tag = RVT_ZEROINIT;
}

/// @brief alloc型分配数组
/// @param name 
void generateRawValueArr(string &name,vector<int> &dimen) {
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *alloc = (RawValue *)malloc(sizeof(RawValue));
    alloc->value.tag = RVT_ALLOC;
    RawType *ty = new RawType();
    ty->tag = RTT_ARRAY;
    generateArrType(ty,dimen,0);
    alloc->ty = ty;
    insts.buffer[insts.len++] = (const void *) alloc;
    signTable.insertVar(name,alloc);
}  
/// @brief load型value
/// @param sign 
/// @param src 
void generateRawValue(string &name, RawValueP &src)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue * load = new RawValue();
    RawType *ty = new RawType();
    ty->tag = RTT_INT32;
    load->ty = (RawTypeP) ty;
    load->name = nullptr;
    load->value.tag = RVT_LOAD;
    load->value.data.load.src = src;
    insts.buffer[insts.len++] = (const void *)load;
    RawValue *SrcValue = (RawValue*) src;
    bb->uses.insert(SrcValue);
    signTable.insertMidVar(name,load);
}
/// @brief branch型value
/// @param cond 
/// @param Truebb 
/// @param Falsebb 
void generateRawValue(RawValueP &cond, RawBasicBlock* &Truebb, RawBasicBlock* &Falsebb){
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *br = new RawValue();
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    br->ty = (RawTypeP) ty;
    br->name = nullptr;
    br->value.tag = RVT_BRANCH;
    br->value.data.branch.cond = cond;
    br->value.data.branch.true_bb = (RawBasicBlockP)Truebb;
    br->value.data.branch.false_bb = (RawBasicBlockP)Falsebb;
    insts.buffer[insts.len++] = br;
}
/// @brief jump型value
/// @param TargetBB 
void generateRawValue(RawBasicBlock* &TargetBB){
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *jump = new RawValue();
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    jump->ty = (RawTypeP) ty;
    jump->name = nullptr;
    jump->value.tag = RVT_JUMP;
    jump->value.data.jump.target = (RawBasicBlockP)TargetBB;
    insts.buffer[insts.len++] = jump;
}
/// @brief call型value
void generateRawValue(RawFunctionP callee,vector<RawValueP> paramsValue,string &sign){
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *call = new RawValue();
    call->name = nullptr;
    call->value.tag = RVT_CALL;
    call->value.data.call.callee = callee;
    auto &params = call->value.data.call.args;
    params.buffer = (const void **) malloc(sizeof(const void *)*500);
    params.kind = RSK_BASICVALUE;
    params.len = 0;
    auto &calleeParams = callee->ty->data.function.params;
    if(calleeParams.len != paramsValue.size()) {
        cerr << "wrong variables, has " <<  paramsValue.size() << ", expect " << calleeParams.len << endl;
        assert(0);
    }
    for(int i = 0; i < paramsValue.size();i++) {
        auto funcParamType = reinterpret_cast<RawTypeP>(callee->ty->data.function.params.buffer[i]); 
        auto expectType = funcParamType->tag;
        auto actualType = paramsValue[i]->ty->tag;
        //if(expectType == actualType) {
            params.buffer[params.len++] = (const void *) paramsValue[i];
        //}
        //else {//如果是浮点数的话，这里需要类型转换,但是由于存在指针的问题，这里不进行类型转换
        //}
    } 
    auto retType = callee->ty->data.function.ret->tag;
    RawType *ty = new RawType();
    ty->tag = retType;
    call->ty = ty;
    switch(retType) {
        case RTT_INT32:
        {
            alloc_now++;
            sign =  "%" + to_string(alloc_now);
            signTable.insertMidVar(sign,call);
            break;
        }
        case RTT_UNIT:  
            break;
        default:
            assert(0);
    } 
    insts.buffer[insts.len++] = (const void *) call;
}//call的类型和function的返回值相同

/// @brief args型value
void generateRawValueArgs(const string &ident,int index){
    auto function = getTempFunction();
    auto &params = function->params;
    RawValue *value = new RawValue();
    value->value.tag = RVT_FUNC_ARGS;
    value->value.data.funcArgs.index = index;
    value->name = nullptr;
    RawType *ty = new RawType();
    ty->tag = RTT_INT32;
    value->ty = ty;
    params.buffer[params.len++] = (const void *)value;
    auto &paramsTy = function->ty->data.function.params;
    paramsTy.buffer[paramsTy.len++] = (const void *) ty;
    signTable.insertVar(ident,value);
}

void generateRawValueSinArr(const string &ident,int index) {
    auto function = getTempFunction();
    auto &params = function->params;
    RawValue *value = new RawValue();
    value->value.tag = RVT_FUNC_ARGS;
    value->value.data.funcArgs.index = index;
    value->name = nullptr;
    RawType *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = RTT_INT32;
    ty->data.pointer.base = pointerTy;
    value->ty = ty;
    params.buffer[params.len++] = (const void *)value;
    auto &paramsTy = function->ty->data.function.params;
    paramsTy.buffer[paramsTy.len++] = (const void *) ty;
    signTable.insertVar(ident,value);
}

void generateRawValueMulArr(const string &ident,int index,vector<int>dimens) {
    auto function = getTempFunction();
    auto &params = function->params;
    RawValue *value = new RawValue();
    value->value.tag = RVT_FUNC_ARGS;
    value->value.data.funcArgs.index = index;
    value->name = nullptr;
    RawType *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = RTT_ARRAY;
    generateArrType(pointerTy,dimens,0);
    ty->data.pointer.base = pointerTy;
    value->ty = ty;
    params.buffer[params.len++] = (const void *)value;
    auto &paramsTy = function->ty->data.function.params;
    paramsTy.buffer[paramsTy.len++] = (const void *) ty;
    signTable.insertVar(ident,value);
}

void createRawProgramme(RawProgramme *&Programme) {
    //Programme = (RawProgramme *) malloc(sizeof(RawProgramme));
    Programme = new RawProgramme();
    auto &funcs = Programme->Funcs;
    funcs.kind = RSK_FUNCTION;
    funcs.len  = 0;
    funcs.buffer = (const void **) malloc(sizeof(const void *) * 100);
    auto &value = Programme->Value;
    value.kind = RSK_BASICVALUE;
    value.len = 0;
    value.buffer = (const void **) malloc(sizeof(const void *)*500);
}


void generateRawBasicBlock(RawBasicBlock *&bb, const char * name){
    bb = new RawBasicBlock();
    bb->name = (char *) malloc(sizeof(char) * 100);
    char *sign = (char *)bb->name;
    strcpy(sign,name);
    auto &insts = bb->insts;
    insts.kind = RSK_BASICVALUE;
    insts.len = 0;
    insts.buffer = (const void **) malloc(sizeof(const void *) * 1000);
    auto &params = bb->params;
    params.buffer = (const void **) malloc(sizeof(const void *) * 1000);
    params.len = 0;
    params.kind = RSK_BASICVALUE;
}
//将basicBlock挂在Function下
void PushRawBasicBlock(RawBasicBlock *&bb) {
    auto function = getTempFunction();
    auto &bbs = function->bbs;
    bbs.buffer[bbs.len++] = (const void *)bb;
}
//初始化时不进行对于类型的操作，等到后面一起修改
void generateRawFunction(RawFunction *&function, const char *name,int type) {
    auto programme = getTempProgramme();
    auto &funcs = programme->Funcs;
    function = new RawFunction();
    auto &bbs = function->bbs;
    bbs.kind = RSK_BASICBLOCK;
    bbs.len = 0;
    bbs.buffer = (const void **) malloc(sizeof(const void *)*500);
    auto &params = function->params;
    params.kind = RSK_BASICVALUE;
    params.len = 0;
    params.buffer = (const void **) malloc(sizeof(const void *)*500);
    function->name = (char*) malloc(sizeof(char) *100);
    char *sign = (char *)function->name;
    strcpy(sign,name);
    RawType *ty = new RawType();
    ty->tag = RTT_FUNCTION;
    RawType *retTy = new RawType();
    switch(type) {
        case FUNCTYPE_INT:
            retTy->tag = RTT_INT32; break;
        case FUNCTYPE_VOID:
            retTy->tag = RTT_UNIT; break;
        default:
            assert(0);
    }
    ty->data.function.ret = retTy;
    auto &ParamTy = ty->data.function.params;
    ParamTy.kind = RSK_TYPE;
    ParamTy.buffer = (const void **) malloc(sizeof(const void *)*500);
    ParamTy.len = 0;
    function->ty = ty;
    funcs.buffer[funcs.len++] = (const void *) function;
    setTempFunction(function);
}
//对于全局变量，只有初值是常量，其他如同其他变量一样，使用load或者store
void generateRawValueGlobal(const char *name,int init) {
    auto programme = getTempProgramme();
    auto &values = programme->Value;
    //RawValue *global = (RawValue *)malloc(sizeof(RawValue));
    RawValue *global = new RawValue();
    RawType  *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = RTT_INT32;
    ty->data.pointer.base = pointerTy;
    global->ty = (RawTypeP)ty;
    global->value.tag = RVT_GLOBAL;
    RawValue *initValue = new RawValue();
    initValue->ty = pointerTy;
    initValue->value.tag = RVT_INTEGER;
    initValue->value.data.integer.value = init;
    global->value.data.global.Init = (RawValueP) initValue;
    global->name = (char *) malloc(sizeof(char)*50);
    char *sign = (char *)global->name;
    strcpy(sign,name);
    values.buffer[values.len++] = (const void *)global;
    signTable.insertVar(name,global);
}
//type这边暂时不写,然后这里使用直接初始化为0
//我感觉这个还是没有什么问题的
void generateRawValue(RawValue *&src,vector<int> &dimen,int index) {
    src->name = nullptr;
    src->value.tag = RVT_AGGREGATE;
    int elemLen = dimen[index];
    //cout << "elemLen: " << elemLen << endl;
    auto &elements = src->value.data.aggregate.elements;
    elements.buffer = (const void **) malloc(sizeof(const void *) *elemLen);
    elements.len = 0;
    elements.kind = RSK_BASICVALUE;
    bool isEnd = index+1 >= dimen.size();
    for(int i = 0; i < elemLen;i++) {
        RawValue* subSrc = new RawValue();
        if(isEnd) {
            subSrc->value.tag = RVT_INTEGER;
            subSrc->value.data.integer.value = 0;
        }
        else generateRawValue(subSrc,dimen,index+1);
        elements.buffer[elements.len++] = (const void *) subSrc;
    }
}
void ShowAggregate(RawValue *src) {
    auto &element = src->value.data.aggregate.elements;
    cout << "{";
    for(int i = 0; i < element.len;i++) {
        auto ptr = (RawValue *) element.buffer[i];
        auto tag = ptr->value.tag;
        if(tag == RVT_INTEGER) { cout << ptr->value.data.integer.value << ",";}
        else ShowAggregate(ptr);
    }
    cout << "}";
}

//填充value
void fillValue(RawValueP &raw,RawValue *target,int &index) {
    auto &targetElements = target->value.data.aggregate.elements;
    auto &rawElements = raw->value.data.aggregate.elements;
    for(int i = 0; i < targetElements.len;i++) {
        if(index >= rawElements.len) break;
        auto targetElement = (RawValue*)targetElements.buffer[i];
        auto targetTag = targetElement->value.tag;
        auto rawElement = (RawValueP)rawElements.buffer[index];
        auto rawTag = rawElement->value.tag;
        if(targetTag == RVT_AGGREGATE) {
            if(rawTag == RVT_INTEGER) {
            fillValue(raw,targetElement,index);
            } else {
                fillAggregate(rawElement,targetElement);index++;
            }
        }
        else {
            if(rawTag != RVT_INTEGER) {
                cerr << "error: rvalue can't be assign to the lvalue" << endl;
                assert(0);
            } else {
                targetElement->value.data.integer.value = rawElement->value.data.integer.value;
                index++;
            }
        }
    }
}
//填充aggregate
void fillAggregate(RawValueP &raw, RawValue *target){
    assert(raw->value.tag == RVT_AGGREGATE);
    assert(target->value.tag == RVT_AGGREGATE);
    auto &RawElements = raw->value.data.aggregate.elements;
    auto &TargetElements = target->value.data.aggregate.elements;
    int RawIndex = 0;
    for(int i = 0 ; i < TargetElements.len;i++) {
        if(RawIndex >= RawElements.len) break;
        auto rawElement = (RawValueP ) RawElements.buffer[RawIndex];
        auto rawElementTag = rawElement->value.tag;
        auto targetElement = (RawValue*) TargetElements.buffer[i];
        if(rawElementTag == RVT_INTEGER) fillValue(raw,target,RawIndex);
        else {
            fillAggregate(rawElement,targetElement); RawIndex++;
        }
    }
}

//这里直接与store绑定，无需出现在insts中
void fillZero(RawValueP &rawSrc,RawValueP &src,vector<int> &dimen) {
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    RawValue *Src = new RawValue();
    int index = 0;
    generateRawValue(Src,dimen,index);
    RawValue *raw = (RawValue *) rawSrc;
    fillAggregate(rawSrc,Src);
    src = (RawValueP) Src;
} 

//创建getelement对象
void generateElement(RawValueP &src,RawValueP &index,string &name) {
    auto bb = getTempBasicBlock();
    auto &insts = bb->insts;
    auto SrcTyTag = src->ty->tag;
    assert(SrcTyTag == RTT_ARRAY || SrcTyTag == RTT_POINTER);
    if(SrcTyTag == RTT_ARRAY) {
    RawValue *GetElement = new RawValue();
    GetElement->name = nullptr;
    GetElement->value.tag = RVT_GET_ELEMENT;   
    GetElement->value.data.getelement.index = index;
    GetElement->value.data.getelement.src = src;
    auto &ArrayBase = src->ty->data.array.base;
    GetElement->ty = ArrayBase;
    alloc_now++;name = "%"+to_string(alloc_now);
    insts.buffer[insts.len++] = (const void *) GetElement;
    signTable.insertMidVar(name,GetElement);
    } else if(SrcTyTag == RTT_POINTER) {
    RawValue *GetPtr = new RawValue();
    GetPtr->name = nullptr;
    GetPtr->value.tag = RVT_GET_PTR;   
    GetPtr->value.data.getelement.index = index;
    GetPtr->value.data.getelement.src = src;
    auto &PointerBase = src->ty->data.pointer.base;
    GetPtr->ty = PointerBase;
    alloc_now++;name = "%"+to_string(alloc_now);
    insts.buffer[insts.len++] = (const void *) GetPtr;
    signTable.insertMidVar(name,GetPtr);
    } else assert(0);
}//这里的这个类型我是直接定义成为数组
//当前还有三个任务没有完成：
/*
1、 函数参数
2、 数组左值赋值
3、 常量数组
*/

void PushFollowBasieBlock(RawBasicBlock *&fbb1,RawBasicBlock *&fbb2) {
    auto bb = getTempBasicBlock();
    auto &fbbs = bb->fbbs;
    fbbs.buffer = (const void **) malloc(sizeof(const void *) * 2);
    fbbs.len = 2;
    fbbs.kind = RSK_BASICBLOCK;
    fbbs.buffer[0] = fbb1;
    fbbs.buffer[1] = fbb2;
}

void PushFollowBasieBlock(RawBasicBlock *&fbb){
    auto bb = getTempBasicBlock();
    auto &fbbs = bb->fbbs;
    fbbs.buffer = (const void **) malloc(sizeof(const void *));
    fbbs.len = 1;
    fbbs.kind = RSK_BASICBLOCK;
    fbbs.buffer[0] = fbb;
}

void PushFollowBasieBlock(){
    auto bb = getTempBasicBlock();
    auto &fbbs = bb->fbbs;
    fbbs.len = 0;
    fbbs.kind = RSK_BASICBLOCK;
}