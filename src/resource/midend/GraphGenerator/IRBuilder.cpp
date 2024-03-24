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

RawBasicBlock * getTempWhileEntry(){
    return irBuilder->WhileEntryRecord.top();
}

RawBasicBlock * getTempWhileEnd(){
    return irBuilder->WhileEndRecord.top();
}

void pushTempWhileEntry(RawBasicBlock *we){
    irBuilder->WhileEntryRecord.push(we); 
}

void popTempWhileEntry(){
    irBuilder->WhileEntryRecord.pop();
}

void pushTempWhileEnd(RawBasicBlock *we){
    irBuilder->WhileEndRecord.push(we);
}

void popTempWhileEnd(){
    irBuilder->WhileEndRecord.pop();
}

bool InWhile() {
    return !irBuilder->WhileEndRecord.empty();
}