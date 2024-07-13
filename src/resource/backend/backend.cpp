#include"../../include/backend/Generator/generator.h"
#include "../../include/midend/IR/IRGraph.h"

void backend_advanced(RawProgramme *& programme) {
    generateASM(programme);
}