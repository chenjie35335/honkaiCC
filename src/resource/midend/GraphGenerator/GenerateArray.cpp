#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
#define EXP 0.00000001
using namespace std;
extern IRBuilder* irBuilder;
extern SignTable signTable;
extern int alloc_now;
extern SignTable signTable;
//这里是只返回大小
void ArrayDimenAST::generateGraph(vector<int> &dimens) const {
    for(auto &sinDimen : sinArrayDimen) {
        auto dimen = sinDimen->Calculate();
        int dimenValue = 0;
        if(dimen->type == ExpResult::FLOAT) {
            dimenValue = dimen->FloatResult;
        } else {
            dimenValue = dimen->IntResult;
        }
        dimens.push_back(dimenValue);
    }
}
//这里需要返回RawValueP
void ArrayDimenAST::generateGraph(vector<RawValueP> &dimens) const {
    for(auto &sinDimen : sinArrayDimen) {
        string dimenName;
        sinDimen->generateGraph(dimenName);
        RawValueP dimen = signTable.getMidVar(dimenName);
        //cout << "dimenName= " << dimenName << endl;
        //cout << "dimenType= " << dimen->value.tag << endl;
        dimens.push_back(dimen);
    }
}

void SinArrayDimenAST::generateGraph(string &name) const {
    //cout << "enter SinArrayDimen" << endl;
    exp->generateGraph(name);
}

//这个sign返回回去可以由前面通过查表获取这个aggregate对象
//返回之后再调用进行转换
void ConstArrayInitAST::generateGraph(string &sign, int &retType) const {
    vector<RawValueP> elements;
    if(type == INIT_MUL) {
        multiArrayElement->generateGraph(elements, retType);
    }
    generateRawValue(elements,sign);
}

void ConstArrayInitAST::generateGraphGlobal(string &sign, int &retType) const {
    vector<RawValueP> elements;
    if(type == INIT_MUL) {
        multiArrayElement->generateGraphGlobal(elements, retType);
    }
    generateRawValue(elements,sign);
}
//对于每一个ConstArrayInit型的节点都需要建立一个aggregate对象，一个aggregate对象还是需要弄一个sign
//因为对于init来说必定都是临时变量
void MultiArrayElementAST::generateGraph(vector<RawValueP> &elem, int &retType) const {
    for(auto &sinArrayElem : sinArrayElement) {
        string name;
        sinArrayElem->generateGraph(name,retType);
        RawValueP element;
        getMidVarValue(element,name); 
        elem.push_back(element);
    }
}

void MultiArrayElementAST::generateGraphGlobal(vector<RawValueP> &elem, int &retType) const {
    for(auto &sinArrayElem : sinArrayElement) {
        string name;
        sinArrayElem->generateGraphGlobal(name,retType);
        RawValueP element;
        getMidVarValue(element,name); 
        elem.push_back(element);
    }
}

//这里就直接传字符串，然后返回查表获取了
void SinArrayElementAST::generateGraph(string &sign, int &retType) const {
    //constexp = rettype
    switch(type) {
        case ARELEM_AI: 
            constArrayInit->generateGraph(sign, retType);
            break;
        case ARELEM_EX:{
            constExp->generateGraph(sign);
            auto value = constExp->Calculate();
            if(value->type == ExpResult::FLOAT){
                float res = value->FloatResult;
                if(retType == RVT_FLOAT){
                    sign = to_string(res);
                    RawValue *r1 = new RawValue();
                    r1->value.floatNumber.value = res;
                    signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
                }else {
                    int res_cvt = res;
                    sign = to_string(res_cvt);
                    RawValue *r1 = new RawValue();
                    r1->value.integer.value = res_cvt;
                    signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
                }
            } else {
                int res = value->IntResult;
                if(retType == RVT_FLOAT){
                    float res_cvt = res;
                    sign = to_string(res);
                    RawValue *r1 = new RawValue();
                    r1->value.floatNumber.value = res_cvt;
                    signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
                }else {
                    sign = to_string(res);
                    RawValue *r1 = new RawValue();
                    r1->value.integer.value = res;
                    signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
                }
            }
            break;
            }
          default:  assert(0);
        }   
}


void SinArrayElementAST::generateGraphGlobal(string &sign, int &retType) const {
    switch(type) {
        case ARELEM_AI: 
            constArrayInit->generateGraphGlobal(sign, retType);
            break;
        case ARELEM_EX:{
            auto value = constExp->Calculate();
            if(value->type == ExpResult::FLOAT) {
                float CalValue = value->FloatResult;
                if(retType == RTT_FLOAT){
                    generateRawValueArr(CalValue);
                    sign = to_string(CalValue); 
                    RawValue *r1 = new RawValue();
                    r1->value.floatNumber.value = CalValue;
                    signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
                } else {
                    int IntValue = CalValue;
                    generateRawValueArr(IntValue);
                    sign = to_string(IntValue);
                    RawValue *r1 = new RawValue();
                    r1->value.integer.value = IntValue;
                    signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
                }
            } else {
                int CalValue = value->IntResult;
                if(retType == RTT_FLOAT){
                    float FloatValue = CalValue;
                    generateRawValueArr(FloatValue);
                    sign = to_string(FloatValue);
                    RawValue *r1 = new RawValue();
                    r1->value.floatNumber.value = FloatValue;
                    signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
                } else {
                    generateRawValueArr(CalValue);
                    sign = to_string(CalValue); 
                    RawValue *r1 = new RawValue();
                    r1->value.integer.value = CalValue;
                    signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
                } 
            }
            break;
        }
        default:  assert(0);
    }
}