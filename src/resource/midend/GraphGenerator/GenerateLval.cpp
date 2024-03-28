#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern SignTable signTable;

void LValRAST::generateGraph(string &sign) const {
    RawValueP IdentSrc = signTable.getVarR(ident);
    auto &tag = IdentSrc->value.tag;
    switch(tag) {
        case RVT_INTEGER:
            sign = to_string(IdentSrc->value.data.integer.value);
            break;
        case RVT_ALLOC:
            alloc_now++;
            sign = "%"+to_string(alloc_now);
            generateRawValue(sign,IdentSrc);
            break;
        case RVT_FUNC_ARGS://一个比较好的解决办法是将这个存入一个临时变量中
        //为了方便管理，要在
        {
            RawValue *param = (RawValue *)IdentSrc;
            sign = "%"+ident;
            signTable.insertMidVar(sign,param);
            break;
        }
        default:
            assert(0);
    }
}
//对于sign就从返回的RawValueP判断就行

