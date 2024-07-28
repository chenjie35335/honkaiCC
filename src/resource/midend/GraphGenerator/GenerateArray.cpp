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
        int dimen = sinDimen->calc();
        dimens.push_back(dimen);
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
    switch(type) {
        case ARELEM_AI: 
            constArrayInit->generateGraph(sign, retType);
            break;
        case ARELEM_EX:{
            //constExp->generateGraph(sign);
            generateRawValueArr(constExp->calc());
            sign = to_string(constExp->calc());
            //push to table
            RawValue *r1 = new RawValue();
            r1->value.integer.value = stoi(sign);
            signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
            break;
        }
        case FARELEM_EX:{
            //constExp->generateGraph(sign, retType);
            generateRawValueArr(constExp->fcalc());
            sign = to_string(constExp->fcalc());
            //push to table
            RawValue *r1 = new RawValue();
            r1->value.floatNumber.value = stoi(sign);
            signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
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
            int value = constExp->calc();
            generateRawValueArr(value);
            sign = to_string(value); 
            RawValue *r1 = new RawValue();
            r1->value.integer.value = stoi(sign);
            signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
            break;
        }
        case FARELEM_EX:{
            float value = constExp->fcalc();
            generateRawValueArr(value);
            sign = to_string(value); 
            RawValue *r1 = new RawValue();
            r1->value.floatNumber.value = stoi(sign);
            signTable.IdentTable->ArrayTable.at(sign)->arrValue.elements.push_back(r1);
            break;
        }
        default:  assert(0);
    }
}