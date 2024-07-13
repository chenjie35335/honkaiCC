#ifndef FUTUROW_Inline
#define FUTUROW_Inline
#include "../IR/Programme.h"
#include <unordered_map>
//内联函数中条件判断的数量限制
#define BRANCH_SIZE 3
//内联函数中value条数限制
#define VALUE_SIZE 50
static int inlinefunc_idx = 0;
static unordered_map<RawFunction*,RawValue*> RetValue;
static unordered_map<RawValue*,RawValue*>InlineShareVar;
static unordered_map<RawBasicBlock*,RawBasicBlock*> sliceBlocks;
bool judgementOutFunc(string fun_name);
//使用函数内联
void OptimizeFuncInline(RawProgramme *IR);
//判断函数列表中可以内联的函数集合
void MarkInlineFunc(list<RawFunction *> funcs,unordered_set<RawFunction *>&inline_funcs);
//判断函数中是否存在循环
bool HasLoopInFunc(RawFunction * func);
//判断函数是否递归
bool HasCallSelfFunc(RawFunction * func);
//计算函数中条件判断的数量
int  CalCondNum(RawFunction * func);
//计算函数中value条数
int  CalValueNum(RawFunction * func);
//分配内联函数共用的内部变量，参数变量，返回值变量
void InlineAllocShareVar(RawFunction * func);
//对函数进行内联
void InlineFunc(RawFunction *func,unordered_set<RawFunction *>inline_funcs);
//对基本块进行内联
void InlineBasicBlock(RawBasicBlock* bb,RawFunction *func,unordered_set<RawFunction *>inline_funcs);
//将函数内联到插入点
void InsertInlineFunc(RawFunction *func,deque<RawBasicBlock *> &slicebbs,int inlinefunc_idx,RawBasicBlock* fbb);
//对内联block的value进行复制
void InlineValue(unordered_map<RawBasicBlock *,RawBasicBlock *> copybbs,RawBasicBlock* fbb,RawFunction *func);
#endif