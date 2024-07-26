#ifndef STORMY_DT
#define STORMY_DT
#include <list>
#include <unordered_set>
#include <unordered_map>
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

void GeneratorDT(RawProgramme *&programme,int genDot);
void init_bbs(const RawFunctionP &func);
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
//判断回边
bool BackEdge(const RawBasicBlockP &start,const RawBasicBlockP &end);
//计算循环集合
void cal_cfgloop(const RawBasicBlockP &start,const RawBasicBlockP &end);
//计算根据idom支配树
void cal_DT(RawFunction* func,unordered_map<RawBasicBlock*,RawBasicBlock*>idom);
//计算逆后序遍历顺序
void cal_RPO(RawBasicBlock* nowbb,list<RawBasicBlock*> &RPO);
//计算idom集合
void cal_IDOM(list<RawBasicBlock*> RPO,unordered_map<RawBasicBlock*,RawBasicBlock*>&idom);
//计算公共祖先
RawBasicBlock* intersect(RawBasicBlock*b1,RawBasicBlock*b2,unordered_map<RawBasicBlock*,RawBasicBlock*>DOMS,unordered_map<RawBasicBlock*,int>idx);
#endif