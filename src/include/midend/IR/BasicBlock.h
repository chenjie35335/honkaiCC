#ifndef STORMY_BASICBLOCK
#define STORMY_BASICBLOCK
#include "Type.h"
#include "Utility.h"
#include "Value.h"
#include <list>
#include <cstring>
#include <unordered_set>
class AsmInst;
using namespace std;
class RawBasicBlock{
    public:
/// type of basic block  
    RawType* ty;
/// name of bb
    const char * name;
/// instruction list
    list<RawValue *> inst;
/// asm instruction list
    list<AsmInst *> asmInstList;
/// cfg pre basic blocks
    list<RawBasicBlock *> pbbs;
/// cfg following basic blocks
    list<RawBasicBlock *> fbbs;
/// use sites
    unordered_set<RawValue *> uses;
/// def sites
    unordered_set<RawValue *> defs;
/// pre domian node
    RawBasicBlockP preDomainNode;
/// follow domain nodes
    list<RawBasicBlock *> domains;
/// DF
    unordered_set<const RawBasicBlock *> df;
/// A phi set 
    unordered_set<RawValueP> NessPhi;
/// function phi for this basicblock
    list<RawValue *> phi;
/// whether the bb is to remove
    bool isDeleted;
/// whether the bb is executed
    bool isExec;
/// used when DFS
    bool isVisited;

    RawBasicBlock() {
        this->isDeleted = false;
        this->isExec = false;
        this->isVisited = false;
    }
    RawBasicBlock(const char * name,int idx){
        this->isDeleted = false;
        this->isExec = false;
        this->isVisited = false;
        size_t bufSize = strlen(name) + 12;
        this->name = new char[bufSize];
        snprintf((char *)this->name, bufSize, "%s_%d", name, idx);
        
    }
    RawBasicBlock(const char * funcname,const char * bbname,int idx){
        this->isDeleted = false;
        this->isExec = false;
        this->isVisited = false;
        size_t bufSize = strlen(funcname)+strlen(bbname) + 13;
        this->name = new char[bufSize];
        snprintf((char *)this->name, bufSize, "%s_%s_%d", funcname,bbname, idx);
        
    }

    void AddAsmInst(AsmInst *inst) {
        this->asmInstList.push_back(inst);
    }
};  
typedef const RawBasicBlock * RawBasicBlockP;
/// @brief generate RawBasicBlock data structure
/// @param bb 
/// @param name 
void generateRawBasicBlock(RawBasicBlock *&bb, const char *name);
/// @brief insert basicblock into function
/// @param bb 
void PushRawBasicBlock(RawBasicBlock *&bb);
/// @brief create cfg when following basicblock is 2
/// @param fbb1 
/// @param fbb2 
void PushFollowBasieBlock(RawBasicBlock *&fbb1,RawBasicBlock *&fbb2);
/// @brief create cfg when following basicblock is 1
/// @param fbb 
void PushFollowBasieBlock(RawBasicBlock *&fbb);
/// @brief create cfg when following basicblock is 0
void PushFollowBasieBlock();
#endif