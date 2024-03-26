#include"../../include/backend/Generator/generator.h"
#include"../../include/backend/hardware/hardwareManager.h"
#include "../../include/midend/IR/IRGraph.h"

void backend(RawProgramme *& programme) {
    generateASM(programme);
}