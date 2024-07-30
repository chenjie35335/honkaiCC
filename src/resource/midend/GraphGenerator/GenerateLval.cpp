#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern SignTable signTable;
/*
需不需要将这里修改一下呢，其实getelement和getptr本意差别较大
* */
/*
load去掉一层*，getelement去掉一层括号，getptr对于类型没有影响
所以根据这个来说，如果要让中端正确，后端就得修改
*/
// 右值这里可以使用的场合非常多
void LValRAST::generateGraph(string &sign) const {
    RawValueP zero = generateZero();
    if(type == IDENT) {
    RawValueP IdentSrc = signTable.getVarR(ident);
    auto &tag = IdentSrc->value.tag;
    switch(tag) {
        //整数浮点
        case RVT_INTEGER:
            // cout << "get ident "<< ident <<" value " << IdentSrc->value.integer.value << endl;
            sign = to_string(IdentSrc->value.integer.value);
            break;
        case RVT_FLOAT:
            // cout << "get ident "<< ident <<" value " << IdentSrc->value.floatNumber.value << endl;
            sign = to_string(IdentSrc->value.floatNumber.value);
            break;
        //考虑alloc类型和global类型
        case RVT_ALLOC:case RVT_GLOBAL:{
            alloc_now++;sign = "%"+to_string(alloc_now);
            auto PointerTag = IdentSrc->ty->pointer.base->tag;
            if(PointerTag == RTT_INT32 || PointerTag == RTT_FLOAT) {
                generateRawValue(sign,IdentSrc);
            } else if(PointerTag == RTT_ARRAY){//这里确实，如果是单的就单纯一个getelemptr就行。
                generateElement(IdentSrc,zero,sign);
                //IdentSrc = signTable.getMidVar(sign);
                //generateElement(IdentSrc,zero,sign);
            } else assert(0);
            break;
        }//这里如果是ptr还得用getptr或者getelementptr
        case RVT_FUNC_ARGS://一个比较好的解决办法是将这个存入一个临时变量中，这个可能之后是个隐患，但是这里先不管
        {
            RawValue *param = (RawValue *)IdentSrc;
            sign = "%"+ident;
            signTable.insertMidVar(sign,param);
            break;
        }
        default:
            assert(0);
        }
    } else if(type == ARRAY) {//对于右值来说，const和非const是没有区别的
        //ident = arr[i][j];
        RawValueP IdentSrc = signTable.getVarR(ident);
        assert(IdentSrc->ty->tag == RTT_POINTER);
        vector<RawValueP> dimens;
        array->generateGraph(dimens);
        auto IdentType = IdentSrc->identType;
        auto &IdentSrcBase = IdentSrc->ty->pointer.base;
        auto SrcBaseTag = IdentSrcBase->tag;
         if(IdentType == IDENT_ARR) {
            for(auto &dimen : dimens) {
                generateElement(IdentSrc,dimen,sign);
                IdentSrc = signTable.getMidVar(sign);
                SrcBaseTag = IdentSrc->ty->pointer.base->tag;
                //push table
                //signTable.IdentTable->ArrayTable.at(ident)->arrValue.elements.push_back((RawValue*)dimen);
                //这里maybe是调用，不能push arrTable元素
            }
        } else if(IdentType == IDENT_POINTER) {
            auto it = dimens.begin();
            generatePtr(IdentSrc, *it, sign);
            IdentSrc = signTable.getMidVar(sign);
            for(advance(it,1);it != dimens.end();it++) {
                generateElement(IdentSrc, *it, sign);
                IdentSrc = signTable.getMidVar(sign);
            }
            SrcBaseTag = IdentSrc->ty->pointer.base->tag;
        } else {
            cerr << "unknown type:" << IdentType << " ident = " << ident << endl;
            assert(0);
        }
        if(SrcBaseTag == RTT_INT32 || SrcBaseTag == RTT_FLOAT) {
            alloc_now++;sign = "%"+to_string(alloc_now);
            generateRawValue(sign,IdentSrc);
        } else if(SrcBaseTag == RTT_ARRAY) {
            generateElement(IdentSrc,zero,sign);
        } else if(SrcBaseTag == RTT_POINTER) {
            //TODO:
            alloc_now++;
            sign = "%" + to_string(alloc_now);
            //cout << "666" << endl;
            generatePtr(IdentSrc, zero, sign);
        }
        else {
            cout<< SrcBaseTag << "SRCBaseTag" <<endl;
            assert(0);
        } 
    }
}