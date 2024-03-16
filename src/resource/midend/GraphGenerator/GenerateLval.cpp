#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern unordered_map <string,RawValueP> MidVarTable;

void LValRAST::generateGraph(RawSlice &IR, string &sign) const {
    auto p = IdentTable;
    int type;
    p->IdentSearch(ident,sign,type);
    RawValue *value;
    if(type == FIND_CONST) {
        generateRawValue(value,stoi(sign),IR);
    }
}
