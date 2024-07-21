#include "../../../include/midend/IR/IRGraph.h"
#include "../../../include/midend/IR/IRBuilder.h"
#include "../../../include/midend/IR/ValueKind.h"
#include "../../../include/midend/IR/LibFunction.h"
#include "../../../include/midend/AST/AST.h"
#include "../../../include/midend/ValueTable/SignTable.h"
#include "../../../include/midend/Optimizer/OptimizeMem2reg.h"
#include "../../../include/midend/SSA/PHI.h"
#include "../../../include/midend/SSA/rename.h"

void OptimizeMem2Reg(RawProgramme *&programme){
    AddPhi(programme);
    renameValue(programme);
}