#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/AST/ast.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern unordered_map <string,RawValueP> MidVarTable;

void LValRAST::generateGraph(string &sign) const {
    // int type;
    // IdentTable->IdentSearch(ident,sign,type);
    // if(type == FIND_CONST) {
        // generateRawValue(stoi(sign));
    // } else if(type == FIND_VAR) {
        // RawValueP LoadSrc;
        // generateRawValue(LoadSrc,sign);
        // alloc_now++;
        // sign = "%"+to_string(alloc_now);
        // generateRawValue(sign,LoadSrc); 
    // } else assert(0);
}

