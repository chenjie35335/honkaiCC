#ifndef STORMY_DT
#define STORMY_DT
#include <unordered_set>
#include "../IR/common.h"
// typedef struct DTnode{
//     DTnode * pre;
//     int followLen;
//     DTnode ** follw;
//     RawBasicBlockP bbs;
// }DTnode;
// typedef struct DT
// {
//     const void ** buffer;
//     uint32_t len;
// }DT;
typedef std::unordered_set<RawBasicBlockP> nodeSet;

void GeneratorDT(const RawSlice &funcs);
void init_bbs(const RawFunctionP &func);
void clearBbsFlag();
bool check_bb_inOther(const RawBasicBlockP &bb);
void func_domain_nodes(const RawFunctionP &func);
void find_domain_nodes(const RawBasicBlockP &s_bbs,const RawBasicBlockP &delete_bbs);
void direct_domain_nodes();
// 生成CFG的文本描述(dot语言)
void traversalCFG(const RawFunctionP &func);
// 生成支配树的文本描述(dot语言)
void traversalDT(const RawFunctionP &func);
// 生成支配边界的文本
void traversalDF(const RawFunctionP &func);
//判断A是否支配B,或者B的必经结点是否是A
bool AisdomB(const RawBasicBlockP A, const RawBasicBlockP B);
// 计算支配边界
void computeDF(RawBasicBlock * & n);
#endif