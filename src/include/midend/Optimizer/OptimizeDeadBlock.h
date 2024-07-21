#ifndef DEADBLOCK_STORMY
#define DEADBLOCK_STORMY
#include <unordered_set>
#include "../IR/common.h"
//可以先尝试着先把这个算法实现，其他的再考虑
//先从alloc以及相应的copy和phi，将这些填入W
//对于W完成上述操作
//这个存在的一个问题是单独的临时变量的问题，但是这个在测例中很难出现，先不管
//难道说为了一个W我要遍历一边数据结构？
//貌似这个是可行的，但是感觉有些蠢，想想其他办法//
//这个算法的实施肯定以函数为单位进行
//全局变量作为的是额外的影响，不能一概而论。
//这样的话倒是有个比较好的想法，在插入的时候插入到alloc的RawValue当中，就没关系了
void OptimizeDCE(RawProgramme *&programme);
#endif





