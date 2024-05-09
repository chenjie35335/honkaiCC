#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern SignTable signTable;

void LValRAST::generateGraph(string &sign) const {
    if(type == IDENT) {
    RawValueP IdentSrc = signTable.getVarR(ident);
    auto &tag = IdentSrc->value.tag;
    switch(tag) {
        case RVT_INTEGER:
            sign = to_string(IdentSrc->value.data.integer.value);
            break;
        case RVT_ALLOC:case RVT_GLOBAL:{
            alloc_now++;sign = "%"+to_string(alloc_now);
            auto TyTag = IdentSrc->ty->tag;
            if(TyTag == RTT_POINTER) {
            auto PointerTag = IdentSrc->ty->data.pointer.base->tag;
            if(PointerTag == RTT_INT32) {
            generateRawValue(sign,IdentSrc);
            } else if(PointerTag == RTT_ARRAY){
                RawValueP zero;
                int number = 0;
                generateRawValue(number);
                string ZeroSign = to_string(0);
                getMidVarValue(zero,ZeroSign);
                generateElement(IdentSrc,zero,sign);
            } else assert(0);
            } else if(TyTag == RTT_ARRAY) {
                RawValueP zero;
                int number = 0;
                generateRawValue(number);
                string ZeroSign = to_string(0);
                getMidVarValue(zero,ZeroSign);
                generateElement(IdentSrc,zero,sign);
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
        RawValueP IdentSrc = signTable.getVarR(ident);
        auto tag = IdentSrc->value.tag;
        assert(tag == RVT_ALLOC || tag == RVT_GLOBAL || tag == RVT_FUNC_ARGS);//如果是aggregate类型，需要
        vector<RawValueP> dimens;
        array->generateGraph(dimens);
        //则返回的类型就是
        for(auto &dimen : dimens) {
            generateElement(IdentSrc,dimen,sign);
            IdentSrc = signTable.getMidVar(sign);
        }
            auto IdentSrcTy = IdentSrc->ty->tag;
            if(IdentSrcTy == RTT_INT32) {
            alloc_now++;sign = "%"+to_string(alloc_now);
            generateRawValue(sign,IdentSrc);
            }
    } else assert(0);
}//这次需要getelementptr这个类型，首先分析一下这个类型
// ty来说，可以通过遍历原来的type，对于类型判断的话，这里暂时不处理
//对于sign就从返回的RawValueP判断就行
//反正这里就是getelementptr,然后load

