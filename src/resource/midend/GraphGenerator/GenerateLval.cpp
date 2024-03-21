#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern unordered_map <string,RawValueP> MidVarTable;

void LValRAST::generateGraph(RawSlice &IR, string &sign) const {
    int type;
    IdentTable->IdentSearch(ident,sign,type);
    RawValue *value;
    if(type == FIND_CONST) {
        generateRawValue(value,stoi(sign),IR);
    } else if(type == FIND_VAR) {
        RawValueP LoadSrc;
        //cout << sign << endl;
        generateRawValue(LoadSrc,sign);
        generateRawValue(value,LoadSrc,IR); 
        alloc_now++;
        sign = "%"+to_string(alloc_now);
        MidVarTable.insert(pair<string,RawValueP>(sign,value));
    } else assert(0);
}
//这里还需要这个load吗？：貌似没有必要（因为后端肯定会自己load到一个寄存器）
