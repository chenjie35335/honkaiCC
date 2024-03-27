#include "../../../include/midend/ValueTable/SignTable.h"

SignTable signTable;

void SignTable::insertFunction(const string &name, RawFunction *&function){
    this->FuncTable.insert(pair<string,RawFunction*>(name,function));
}

void SignTable::identForward(){
    if(this->IdentTable->child == nullptr) {
    auto child = new IdentTableNode(this->IdentTable,tempScope);
    this->IdentTable->child = child;
    }
    this->IdentTable = this->IdentTable->child;
}

void SignTable::identBackward(){
    this->IdentTable = this->IdentTable->father;
}

void SignTable::deleteBackward() {
    this->IdentTable = this->IdentTable->father;
    delete this->IdentTable->child;
    this->IdentTable->child = nullptr;
}

RawValue * SignTable::getMidVar(string &name) {
    assert(MidVarTable.find(name) != MidVarTable.end());
    return MidVarTable.at(name);
}

void SignTable::insertMidVar(string &name, RawValue* &value){
    MidVarTable.insert(pair<string,RawValue *>(name,value));
}

RawValue * SignTable::getVarL(string &name){
    return IdentTable->SearchVarL(name);
}

RawValue * IdentTableNode::SearchVarL(string &name){
    if(findValue(name)) {
        cerr << '"' << name << '"' << "is a constant and can't be altered" << endl;
        assert(0);
    } else if(findVariable(name)) {
        return this->VarTable.at(name);
    } else if(this->father == nullptr){
        cerr << "Error: " << '"' << name << '"' << " is not defined" << endl;
        assert(0);
    } else {
        return this->father->SearchVarL(name);
    }
}

RawValue *SignTable::getVarR(string &name) {
    return IdentTable->SearchVarR(name);
}

RawValue * IdentTableNode::SearchVarR(string &name){
    if(findVariable(name)) {
        return this->VarTable.at(name);
    } else if(this->father == nullptr){
        cerr << "Error: " << '"' << name << '"' << " is not defined" << endl;
        assert(0);
    } else {
        return this->father->SearchVarR(name);
    }
}

void SignTable::insertVar(string &name, RawValue *&value){
    IdentTable->insertVar(name,value);
}

void SignTable::insertNumber(int number,RawValue *&value){
    if(MidVarTable.find(to_string(number)) != MidVarTable.end()){
        MidVarTable[to_string(number)] = value;
    } else {
        MidVarTable.insert(pair<string,RawValue*>(to_string(number),value));
    }
}
