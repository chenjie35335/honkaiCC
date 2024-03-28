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
        default:
            assert(0);
    }
}
//对于sign就从返回的RawValueP判断就行

