#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern unordered_map <string,RawValueP> MidVarTable;

void LValRAST::generateGraph(RawSlice &IR, string &sign) const {
    auto p = IdentTable;
    int type;int dep = p->level;
    p->IdentSearch(ident,sign,type);
    RawValue *value;
    if(type == FIND_CONST) {
        generateRawValue(value,stoi(sign),IR);
    } else if(type == FIND_VAR) {
        RawValueP LoadSrc;
        string SrcSign = "@" + ident + "_" + to_string(dep);
        generateRawValue(LoadSrc,SrcSign);
        value = (RawValue *) malloc(sizeof(RawValue));      
        value->name = nullptr;
        value->value.tag = RVT_LOAD;
        value->value.data.load.src = LoadSrc;
        IR.buffer[IR.len++] = (const void *) value;
        MidVarTable.insert(pair<string,RawValueP>(sign,value));
    } else assert(0);
}
//这里还需要这个load吗？：貌似没有必要（因为后端肯定会自己load到一个寄存器）
