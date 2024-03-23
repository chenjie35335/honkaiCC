#include "../../../include/midend/IR/IRBuilder.h"
IRBuilder* irBuilder;

RawProgramme *getTempProgramme(){
    return irBuilder->tempProgramme;
}

RawFunction *getTempFunction(){
    return irBuilder->tempFunction;
}

RawBasicBlock *getTempBasicBlock(){
    return irBuilder->tempBasicBlock;
}

bool getFinished(){
    return irBuilder->BBFinished;
}

void setFinished(bool finished){
    irBuilder->BBFinished = finished;
}

void setTempProgramme(RawProgramme *tempProgramme){
    irBuilder->tempProgramme = tempProgramme;
}

void setTempFunction(RawFunction *tempFunction){
    irBuilder->tempFunction = tempFunction;
}

void setTempBasicBlock(RawBasicBlock *tempBasicBlock){
    irBuilder->tempBasicBlock = tempBasicBlock;
}