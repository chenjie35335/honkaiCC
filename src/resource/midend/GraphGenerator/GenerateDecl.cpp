#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/GenerateIR.h"
#include "../../../include/midend/IR/ValueKind.h"
#include <cstdlib>
#include <unordered_map>
using namespace std;
extern int ValueNumber;
extern unordered_map <string,RawValueP> MidVarTable;

void DeclAST::generateGraph(RawSlice &IR) const {
    switch(type) {
        case DECLAST_CON : 
                ConstDecl->Dump(); break;
        case DECLAST_VAR : 
                //VarDecl->generateGraph(IR); break;
        default:assert(0);
    }
}