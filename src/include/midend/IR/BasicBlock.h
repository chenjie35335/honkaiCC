#ifndef STORMY_BASICBLOCK
#define STORMY_BASICBLOCK
#include "Type.h"
#include "Utility.h"
#include "Value.h"
#include <unordered_set>
using namespace std;
class RawBasicBlock{
    public:
/// type of basic block  
    RawType* ty;
/// name of bb
    const char * name;
/// parameter(not used until now)
    RawSlice params;
/// instructions
    RawSlice insts;
/// following basic blocks
    RawSlice fbbs;
/// use sites
    unordered_set<RawValue *> uses;
/// def sites
    unordered_set<RawValue *> defs;
/// pre domian node
    RawBasicBlockP preDomainNode;
/// follow domain nodes
    RawSlice domains;
/// DF
    unordered_set<const RawBasicBlock *> df;
/// A phi set 
    unordered_set<RawValueP> NessPhi;
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