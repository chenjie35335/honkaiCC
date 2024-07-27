#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <cstring>
#include <string>
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
    auto &insts = bb->inst;
    RawValue * value = new RawValue();
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    value->name = nullptr;
    value->value.tag = RVT_RETURN;
    value->value.ret.value = src;
    value->ty = (RawTypeP) ty;
    insts.push_back(value);
}
/// @brief binary型value
/// @param sign 
/// @param lhs 
/// @param rhs 
/// @param op 
void generateRawValue(string &name, RawValueP lhs, RawValueP rhs, uint32_t op)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue * value = new RawValue();
    value->name = nullptr;
    value->value.tag = RVT_BINARY;
    value->value.binary.lhs = lhs;
    value->value.binary.rhs = rhs;
    value->value.binary.op = op;
    RawType *ty = new RawType();
    //我们尽量按照浮点的要求去处理，后面可以转化为整形
    if(lhs->ty->tag == RTT_FLOAT || rhs->ty->tag == RTT_FLOAT){
        if( op == RBO_FEQ       || 
            op == RBO_NOT_FEQ   || 
            op == RBO_FGE       || 
            op == RBO_FGT       ||
            op == RBO_FLE       || 
            op == RBO_FLT
            )
            ty->tag = RTT_INT32;
        else ty->tag = RTT_FLOAT;
    } else {
        ty->tag = RTT_INT32;
    }
    value->ty = (RawTypeP) ty;
    insts.push_back(value);
    signTable.insertMidVar(name,value);
}
/// @brief 
/// @param number 
/// @return 
RawValue * generateNumber(int32_t number) {
    RawValue *value = new RawValue();
    value->name = nullptr;
    value->value.tag = RVT_INTEGER;
    value->value.integer.value = number;
    RawType *ty = new RawType();
    ty->tag = RTT_INT32;
    value->ty = ty;
    return value;
}
/**
 * @brief add float
 * @param number 
 * @return RawValue* 
 */
RawValue * generateFloat(float number) {
    RawValue *value = new RawValue();
    value->name = nullptr;
    value->value.tag = RVT_FLOAT;
    value->value.floatNumber.value = number;
    RawType *ty = new RawType();
    ty->tag = RTT_FLOAT;
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
        auto &insts = bb->inst;
        insts.push_back(value);
        }
        signTable.insertNumber(number,value);
}

RawValueP generateZero() {
    RawValueP zero;
    int number = 0;
    generateRawValue(number);
    string ZeroSign = to_string(0);
    getMidVarValue(zero,ZeroSign);
    return zero;
}
/**
 * @brief float
 * @param number 
 */
void generateRawValue(float number)
{
        auto bb = getTempBasicBlock();
        RawValue * value = generateFloat(number);
        if(bb != nullptr) {
        auto &insts = bb->inst;
        insts.push_back(value);
        }
        //后面可能会产生很多bug，有些可以复用重载，有些得重新命名
        signTable.insertFnumber(number,value);
}

void generateRawValueArr(int32_t number) {
    RawValue * value = generateNumber(number);
    signTable.insertNumber(number,value);
}

/**
 * @brief add float
 * @param number 
 */
void generateRawValueArr(float number) {
    RawValue * value = generateFloat(number);
    signTable.insertFnumber(number,value);
}

/// @brief store型value
/// @param src 
/// @param dest 
void generateRawValue(RawValueP &src, RawValueP &dest)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *store = new RawValue();
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    store->ty = (RawTypeP)ty;
    store->name = nullptr;
    store->value.tag = RVT_STORE;
    store->value.store.value = src;
    store->value.store.dest = dest;
    RawValue *DestValue = (RawValue*)dest;
    insts.push_back(store);
}
/// @brief alloc型value
/// @param sign 
void generateRawValue(string& name, int32_t flag)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *alloc = new RawValue();
    RawType *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = flag;
    ty->pointer.base = pointerTy;
    char *ident = (char *) malloc(sizeof(char) * name.length());
    string NameScope = name + "_" + to_string(signTable.IdentTable->level);
    strcpy(ident,NameScope.c_str());
    alloc->name = ident;
    alloc->ty = (RawTypeP)ty;
    alloc->value.tag = RVT_ALLOC;
    alloc->identType = IDENT_VAR;
    insts.push_back(alloc);
    signTable.insertVar(name,alloc);
}
//这里建立的这个aggregate的type没有任何意义，因为无法判断！
//从当前来看，这个当前是要转换成getptr的，不过考虑全局变量的问题需要特殊对待。这里ty暂时不管
//这个部分属于初始部分，没有补0,所以没有ty没有用处
/**
 * @brief aggregate
 * @param sign 
 */
void generateRawValue(vector<RawValueP>elements,string &sign) {
    auto bb = getTempBasicBlock();
    RawValue *aggregate = new RawValue();
    aggregate->value.tag = RVT_AGGREGATE;
    aggregate->name = nullptr;
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    aggregate->ty = ty;
    auto &elems = aggregate->value.aggregate.elements;
    for(auto &element : elements) {
        auto elem = (RawValue *)element;
        elems.push_back(elem);
    }
    alloc_now++;sign = "%"+to_string(alloc_now);
    signTable.insertMidVar(sign,aggregate);
}
/**
 * @brief ArrType
 * @param ty 
 * @param dimens 
 * @param index 
 * @param flag 
 */

void generateArrType(RawType *&ty,vector<int> &dimens,int index, int32_t flag) {
    assert(ty->tag == RTT_ARRAY);
    if(index >= dimens.size()) {return;}
    else {
        int dimen = dimens[index];
        RawType *subTy = new RawType();
        ty->array.base = subTy;
        ty->array.len = dimen;
        if(index == dimens.size()-1) {
            subTy->tag = flag; 
            return;
        } else {
            subTy->tag = RTT_ARRAY;
            generateArrType(subTy,dimens,index+1, flag);
        }
    }
}
/**
 * @brief global型分配数组
 * @param name 
 * @param dimen 
 * @param init 
 * @param flag 
 */
void generateRawValueArrGlobal(string &name,vector<int> &dimen,RawValue *&init, int32_t flag){
        auto programme = getTempProgramme();
        auto &values = programme->values;
        RawValue *global = new RawValue();
        RawType *ty = new RawType();
        ty->tag = RTT_POINTER;
        RawType *PointerBase = new RawType();
        PointerBase->tag = RTT_ARRAY;
        generateArrType(PointerBase,dimen,0,flag);
        ty->pointer.base = PointerBase;
        global->ty = ty;
        global->value.tag = RVT_GLOBAL;
        global->value.global.Init = (RawValueP) init;
        char *ident = (char *) malloc(sizeof(char) * name.length());
        string NameScope = name + "_" + to_string(signTable.IdentTable->level);
        strcpy(ident,NameScope.c_str());
        global->name = ident;
        global->identType = IDENT_ARR;
        values.push_back(global);
        signTable.insertVar(name,global);
}
/// @brief 
/// @param zeroinit 
void generateZeroInit(RawValue *&zeroinit) {
    zeroinit = new RawValue();
    zeroinit->value.tag = RVT_ZEROINIT;
}
/**
 * @brief alloc型分配数组
 * @param name 
 * @param dimen 
 * @param flag 
 */
void generateRawValueArr(string &name,vector<int> &dimen, int32_t flag) {
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *alloc = new RawValue();
    alloc->value.tag = RVT_ALLOC;
    RawType *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *PointerBase = new RawType();
    PointerBase->tag = RTT_ARRAY;
    generateArrType(PointerBase,dimen,0,flag);
    ty->pointer.base = PointerBase;
    alloc->ty = ty;
    char *ident = (char *) malloc(sizeof(char) * name.length());
    string NameScope = name + "_" + to_string(signTable.IdentTable->level);
    strcpy(ident,NameScope.c_str());
    alloc->name = ident;
    alloc->identType = IDENT_ARR;
    insts.push_back(alloc);
    signTable.insertVar(name,alloc);
}  
/// @brief load型value
/// @param sign 
/// @param src 
void generateRawValue(string &name, RawValueP &src)
{
    assert(src->ty->tag == RTT_POINTER);
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue * load = new RawValue();
    RawType *ty = new RawType();
    load->ty = (RawTypeP) src->ty->pointer.base;
    load->name = nullptr;
    load->value.tag = RVT_LOAD;
    load->value.load.src = src;
    insts.push_back(load);
    RawValue *SrcValue = (RawValue*) src;
    signTable.insertMidVar(name,load);
}
/// @brief branch型value
/// @param cond 
/// @param Truebb 
/// @param Falsebb 
void generateRawValue(RawValueP &cond, RawBasicBlock* &Truebb, RawBasicBlock* &Falsebb){
    RawValueP condValue = cond;
    if(cond->ty->tag == RTT_FLOAT) {
        string Convert;
        generateConvert(cond,Convert);
        condValue = signTable.getMidVar(Convert);
    }
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *br = new RawValue();
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    br->ty = (RawTypeP) ty;
    br->name = nullptr;
    br->value.tag = RVT_BRANCH;
    br->value.branch.cond = condValue;
    br->value.branch.true_bb = (RawBasicBlockP)Truebb;
    br->value.branch.false_bb = (RawBasicBlockP)Falsebb;
    insts.push_back(br);
}
/// @brief jump型value
/// @param TargetBB 
void generateRawValue(RawBasicBlock* &TargetBB){
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *jump = new RawValue();
    RawType *ty = new RawType();
    ty->tag = RTT_UNIT;
    jump->ty = (RawTypeP) ty;
    jump->name = nullptr;
    jump->value.tag = RVT_JUMP;
    jump->value.jump.target = (RawBasicBlockP)TargetBB;
    insts.push_back(jump);
}
/// @brief call型value
void generateRawValue(RawFunctionP callee,vector<RawValueP> paramsValue,string &sign){
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *call = new RawValue();
    call->name = nullptr;
    call->value.tag = RVT_CALL;
    call->value.call.callee = callee;
    auto &params = call->value.call.args;
    auto &calleeParams = callee->ty->function.params;
    if(calleeParams.size() != paramsValue.size()) {
        cerr << "wrong variables, has " <<  paramsValue.size() << ", expect " << calleeParams.size() << endl;
        assert(0);
    }
    for(int i = 0; i < paramsValue.size();i++) {
        auto funcParam = paramsValue[i];
        auto paramType = (RawType *) calleeParams[i];
        if(funcParam->ty->tag == paramType->tag){
            params.push_back((RawValue *)funcParam);
        } else if(paramType->tag == RTT_INT32 || paramType->tag == RTT_FLOAT){//这里就是如果这之中有个浮点但是其他是整型
            generateConvert(funcParam,sign);
            funcParam = signTable.getMidVar(sign);
            params.push_back((RawValue *)funcParam);
        } else {
            cerr << "wrong types, param " << i << " has " << funcParam->ty->tag << ", expect " << paramType->tag << endl;
            assert(0);
        }
    } 
    auto retType = callee->ty->function.ret->tag;
    //cout << "retType of "<< callee->name << " is " << retType << endl;
    RawType *ty = new RawType();
    ty->tag = retType;
    call->ty = ty;
    switch(retType) {
        //add float
        case RTT_FLOAT: case RTT_INT32:
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
    insts.push_back(call);
}//call的类型和function的返回值相同

/**
 * @brief args
 * @param ident 
 * @param index 
 * @param flag 
 */
void generateRawValueArgs(string &ident,int index, int32_t flag){
    auto function = getTempFunction();
    auto &params = function->params;
    RawValue *value = new RawValue();
    value->value.tag = RVT_FUNC_ARGS;
    value->value.funcArgs.index = index;
    value->name = nullptr;
    RawType *ty = new RawType();
    ty->tag = flag;
    value->ty = ty;
    params.push_back(value);
    auto &paramsTy = function->ty->function.params;
    paramsTy.push_back(ty);
    signTable.insertMidVar(ident,value);
}
/// @brief a[]类型的参数
/// @param ident 
/// @param index 
/// @param flag 
void generateRawValueSinArr(string &ident,int index,int flag) {
    auto function = getTempFunction();
    auto &params = function->params;
    RawValue *value = new RawValue();
    value->value.tag = RVT_FUNC_ARGS;
    value->value.funcArgs.index = index;
    value->name = nullptr;
    RawType *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = flag;
    ty->pointer.base = pointerTy;
    value->ty = ty;
    value->identType = IDENT_POINTER;
    params.push_back(value);
    auto &paramsTy = function->ty->function.params;
    paramsTy.push_back(ty);
    signTable.insertMidVar(ident,value);
}

/**
 * @brief Mularr
 * @param ident 
 * @param index 
 */
void generateRawValueMulArr(string &ident,int index,vector<int>dimens,int flag) {
    auto function = getTempFunction();
    auto &params = function->params;
    RawValue *value = new RawValue();
    value->value.tag = RVT_FUNC_ARGS;
    value->value.funcArgs.index = index;
    value->name = nullptr;
    RawType *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = RTT_ARRAY;
    generateArrType(pointerTy,dimens,0,flag);
    ty->pointer.base = pointerTy;
    value->ty = ty;
    value->identType = IDENT_POINTER;
    params.push_back(value);
    auto &paramsTy = function->ty->function.params;
    paramsTy.push_back(ty) ;
    signTable.insertMidVar(ident,value);
}
//alloc型变量，但是指针指向的是临外的类型
///ty为指向的类型
void generateRawValuePointer(string &name,RawType *ty)
{
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *alloc = new RawValue();
    RawType *pointer = new RawType();
    pointer->tag = RTT_POINTER;
    pointer->pointer.base = ty;
    char *ident = (char *) malloc(sizeof(char) * name.length());
    string NameScope = name + "_" + to_string(signTable.IdentTable->level);
    strcpy(ident,NameScope.c_str());
    alloc->name = ident;
    alloc->ty = (RawTypeP)pointer;
    alloc->value.tag = RVT_ALLOC;
    alloc->identType = IDENT_POINTER;
    insts.push_back(alloc);
    signTable.insertVar(name,alloc);
}//这个应该不会产生说是phi函数

void createRawProgramme(RawProgramme *&Programme) {
    Programme = new RawProgramme();
}


void generateRawBasicBlock(RawBasicBlock *&bb, const char * name){
    bb = new RawBasicBlock();
    bb->name = (char *) malloc(sizeof(char) * 100);
    char *sign = (char *)bb->name;
    strcpy(sign,name);
} 
//将basicBlock挂在Function下
void PushRawBasicBlock(RawBasicBlock *&bb) {
    auto function = getTempFunction();
    auto &bbs = function->basicblock;
    bbs.push_back(bb);
}

//将basicBlock挂在Function下
void PushRawFunction(RawFunction *&function) {}
//初始化时不进行对于类型的操作，等到后面一起修改
void generateRawFunction(RawFunction *&function, const char *name,int type) {
    auto programme = getTempProgramme();
    auto &funcs = programme->funcs;
    function = new RawFunction();
    auto &params = function->params;
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
        case FUNCTYPE_FLOAT:
            retTy->tag = RTT_FLOAT; break;
        default:
            assert(0);
    }
    ty->function.ret = retTy;
    auto &ParamTy = ty->function.params;
    function->ty = ty;
    funcs.push_back(function);
    setTempFunction(function);
}
//对于全局变量，只有初值是常量，其他如同其他变量一样，使用load或者store
//加浮点的重灾区
void generateRawValueGlobal(const char *name,int init) {
    auto programme = getTempProgramme();
    auto &values = programme->values;
    RawValue *global = new RawValue();
    RawType  *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = RTT_INT32;
    ty->pointer.base = pointerTy;
    global->ty = (RawTypeP)ty;
    global->value.tag = RVT_GLOBAL;
    RawValue *initValue = new RawValue();
    initValue->ty = pointerTy;
    initValue->value.tag = RVT_INTEGER;
    initValue->value.integer.value = init;
    global->value.global.Init = (RawValueP) initValue;
    global->name = (char *) malloc(sizeof(char)*50);
    global->identType = IDENT_VAR;
    char *sign = (char *)global->name;
    strcpy(sign,name);
    values.push_back(global);
    signTable.insertVar(name,global);
}

/**
 * @brief add float
 * @param name 
 * @param init 
 */
void generateRawValueGlobal(const char *name,float init) {
    auto programme = getTempProgramme();
    auto &values = programme->values;
    RawValue *global = new RawValue();
    RawType  *ty = new RawType();
    ty->tag = RTT_POINTER;
    RawType *pointerTy = new RawType();
    pointerTy->tag = RTT_FLOAT;
    ty->pointer.base = pointerTy;
    global->ty = (RawTypeP)ty;
    global->value.tag = RVT_GLOBAL;
    global->identType = IDENT_VAR;
    RawValue *initValue = new RawValue();
    initValue->ty = pointerTy;
    initValue->value.tag = RVT_FLOAT;
    initValue->value.floatNumber.value = init;
    global->value.global.Init = (RawValueP) initValue;
    global->name = (char *) malloc(sizeof(char)*50);
    char *sign = (char *)global->name;
    strcpy(sign,name);
    values.push_back(global);
    signTable.insertVar(name,global);
}

/**
 * @brief float version
 * @param src 
 * @param dimen 
 * @param index 
 * @param flag 
 */
void generateRawValue(RawValue *&src,vector<int> &dimen,int index, int flag) {
    src->name = nullptr;
    src->value.tag = RVT_AGGREGATE;
    int elemLen = dimen[index];
    //cout << "elemLen: " << elemLen << endl;
    auto &elements = src->value.aggregate.elements;
    bool isEnd = index+1 >= dimen.size();
    for(int i = 0; i < elemLen;i++) {
        RawValue* subSrc = new RawValue();
        if(isEnd) {
            RawType *ty = new RawType();
            ty->tag = flag;
            subSrc->ty = ty;
            if(flag == RTT_INT32) {
                subSrc->value.tag = RVT_INTEGER;
                subSrc->value.integer.value = 0;
            } else if(flag == RTT_FLOAT) {
                subSrc->value.tag = RVT_FLOAT;
                subSrc->value.floatNumber.value = 0.0;
            } else assert(0);
        }
        else generateRawValue(subSrc,dimen,index+1,flag);
        elements.push_back(subSrc) ;
    }
}
//这里规定一个事情就是：Convert只能从float向int或者int向float
//而且这个是临时变量，因此不存在其他问题
//不过这里注意一个问题就是，int转成float是很正常的，但是float转成int貌似只有
//赋值的时候才会出现
void generateConvert(RawValueP &src,string &name)
{
    RawValue *Convert = new RawValue();
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    Convert->name = nullptr;
    Convert->value.tag = RVT_CONVERT;
    Convert->value.Convert.src = src;
    auto ty = new RawType();
    auto SrcTag = src->ty->tag;
    if(SrcTag == RTT_INT32) ty->tag = RTT_FLOAT;
    else ty->tag = RTT_INT32;
    Convert->ty = ty;
    alloc_now++;name = "%"+to_string(alloc_now);
    insts.push_back(Convert);
    signTable.insertMidVar(name,Convert);
}

void ShowAggregate(RawValue *src) {
    auto &element = src->value.aggregate.elements;
    cout << "{";
    for(int i = 0; i < element.size();i++) {
        auto ptr = (RawValue *) element[i];
        auto tag = ptr->value.tag;
        if(tag == RVT_INTEGER) { cout << ptr->value.integer.value << ",";}
        else if(tag == RVT_FLOAT) { cout<< ptr->value.floatNumber.value << ","; }
        else ShowAggregate(ptr);
    }
    cout << "},";
}

//填充value
void fillValue(RawValueP &raw,RawValue *target,int &RawIndex,int targetIndex) {
    auto &targetElements = target->value.aggregate.elements;
    auto &rawElements = raw->value.aggregate.elements;
    for(int i = targetIndex; i < targetElements.size();i++) {
        //cout << "index" << RawIndex << endl;
        if(RawIndex >= rawElements.size()) break;
        auto targetElement = (RawValue *)targetElements[i];
        auto targetTag = targetElement->value.tag;
        auto rawElement = (RawValueP)rawElements[RawIndex];
        auto rawTag = rawElement->value.tag;
        if(targetTag == RVT_AGGREGATE) {
            //cout << "aggregate type" << endl;
            if(rawTag == RVT_AGGREGATE){
                fillAggregate(rawElement,targetElement);RawIndex++;
            } else {
                fillValue(raw,targetElement,RawIndex,0);
            }
        }
        else {
                targetElements[i] = (RawValue *)rawElement;
                RawIndex++;
        }
}
}
//填充aggregate
void fillAggregate(RawValueP &raw, RawValue *target){
    assert(raw->value.tag == RVT_AGGREGATE);
    //cout << endl << "raw:" << endl;
    assert(target->value.tag == RVT_AGGREGATE);
    //ShowAggregate((RawValue *)raw);
    //cout << endl << "target:" << endl;
    //ShowAggregate(target);
    auto &RawElements = raw->value.aggregate.elements;
    auto &TargetElements = target->value.aggregate.elements;
    int RawIndex = 0;
    for(int i = 0 ; i < TargetElements.size();i++) {
        //cout << "RawIndex" << RawIndex << endl;
        if(RawIndex >= RawElements.size()) break;
        auto rawElement = (RawValueP ) RawElements[RawIndex];
        auto rawElementTag = rawElement->value.tag;
        auto targetElement = (RawValue*) TargetElements[i];
        if(rawElementTag == RVT_AGGREGATE){
            //cout << "fill aggregate" << endl;
            fillAggregate(rawElement,targetElement); RawIndex++;
        } else {
            //cout << "fill value" << endl;
            fillValue(raw,target,RawIndex,i);
        }
    }
}

//这里直接与store绑定，无需出现在insts中
void fillZero(RawValueP &rawSrc,RawValueP &src,vector<int> &dimen) {
    RawValue *Src = new RawValue();
    int index = 0;
    generateRawValue(Src,dimen,index,RTT_INT32);
    fillAggregate(rawSrc,Src);
    src = (RawValueP) Src;
} 

/*
    这里是我在koopa IR上的一个小小的变化：对于IR来说，就是单纯的去掉了一层[]和一层*
*/

//创建getelement对象
void generateElement(RawValueP &src,RawValueP &index,string &name) {
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *GetElement = new RawValue();
    GetElement->name = nullptr;
    GetElement->value.tag = RVT_GET_ELEMENT;   
    GetElement->value.getelement.index = index;
    GetElement->value.getelement.src = src;
    auto SrcTag = src->ty->tag;
    assert(src->ty->tag == RTT_POINTER);
    auto &PointerBase = src->ty->pointer.base;
    auto PointerBaseTag = PointerBase->tag;
    RawType *NewPointer = new RawType();
    NewPointer->tag = RTT_POINTER;
    if(PointerBaseTag == RTT_ARRAY) {
    auto &ArrayBase = PointerBase->array.base;
    NewPointer->pointer.base = ArrayBase;
    }
    GetElement->ty = NewPointer;
    alloc_now++;name = "%"+to_string(alloc_now);
    insts.push_back(GetElement);
    signTable.insertMidVar(name,GetElement);
}//这里的这个类型我是直接定义成为数组
//当前还有三个任务没有完成：
/*
1、 函数参数
2、 数组左值赋值
3、 常量数组
*/

void generatePtr(RawValueP &src, RawValueP &index, string &name){
    auto bb = getTempBasicBlock();
    auto &insts = bb->inst;
    RawValue *GetPtr = new RawValue();
    GetPtr->name = nullptr;
    GetPtr->value.tag = RVT_GET_PTR;   
    GetPtr->value.getptr.index = index;
    GetPtr->value.getptr.src = src;
    GetPtr->ty = src->ty;
    alloc_now++;name = "%"+to_string(alloc_now);
    insts.push_back(GetPtr);
    signTable.insertMidVar(name,GetPtr);
}

void PushFollowBasieBlock(RawBasicBlock *&fbb1,RawBasicBlock *&fbb2) {
    auto bb = getTempBasicBlock();
    auto &fbbs = bb->fbbs;
    fbbs.push_back(fbb1) ;
    fbbs.push_back(fbb2) ;
    auto &fpbb1 = fbb1->pbbs;
    auto &fpbb2 = fbb2->pbbs;
    fpbb1.push_back(bb) ;
    fpbb2.push_back(bb) ;
}

void PushFollowBasieBlock(RawBasicBlock *&fbb){
    auto bb = getTempBasicBlock();
    auto &fbbs = bb->fbbs;
    fbbs.push_back(fbb) ;
    auto &fpbb = fbb->pbbs;
    fpbb.push_back(bb) ;
}

void PushFollowBasieBlock(){}
void MarkUse(RawValue *src,RawValue *target)
{
    if(src != nullptr)
    src->usePoints.push_back(target);
}

void MarkDef(RawValue *src,RawValue *target)
{
    if(src != nullptr)
    src->defPoints.push_back(target);
}



void ArrInit(RawValueP src, RawValueP target) {
    assert(src->value.tag == RVT_AGGREGATE); 
    auto &elements = src->value.aggregate.elements;
    for(int i = 0; i < elements.size(); ++i) {
        auto element = (RawValueP)elements[i];
        auto tag = element->value.tag;
        string sign;
        generateRawValue(i);
        string iNumber = to_string(i);
        RawValueP Idx = signTable.getMidVar(iNumber);
        generateElement(target,Idx,sign);
        RawValueP Addr = signTable.getMidVar(sign);
        if(tag == RVT_AGGREGATE) {
            ArrInit(element,Addr);
        } else {
            auto AddrTag = Addr->ty->pointer.base->tag;
            auto SrcTag = element->ty->tag;
            if(AddrTag != SrcTag) {
                cerr << "Addr = " << AddrTag << ", ElementTag = " << SrcTag << endl; 
                generateConvert(element,sign);
                RawValueP ElemCov = signTable.getMidVar(sign);
                generateRawValue(ElemCov,Addr);
            } else generateRawValue(element,Addr);
        }
    }
}

//哪些地方需要用到这个convert,我认为有以下几个地方：
//1、decl的地方，这里先解决
//2、接下来就是恐怖的表达式