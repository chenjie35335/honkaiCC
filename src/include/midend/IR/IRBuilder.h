#include "IRGraph.h"
#ifndef IRBUILDER_STORMY
#define IRBUILDER_STORMY
//函数只需要连接就行，每次更换都返回函数进行
//这三者之间本身是带有关系的，但是之后需要手动维持这种关系
typedef struct{
    /// 当前程序
    RawProgramme *tempProgramme;
    /// 当前函数
    RawFunction *tempFunction;
    /// 当前基本块
    RawBasicBlock *tempBasicBlock;
    /// 当前基本块是否结束（貌似函数的和程序的这个域没有意义）
    bool BBFinished;
}IRBuilder;
/// @brief 获取当前程序
/// @return 
RawProgramme *getTempProgramme();
/// @brief 获取当前函数
/// @return 
RawFunction *getTempFunction();
/// @brief 获取当前的基本块
/// @return 
RawBasicBlock *getTempBasicBlock();
/// @brief 获取当前基本块的状态
/// @return 
bool getFinished();
/// @brief 修改基本块状态
/// @param finished 
void setFinished(bool finished);
/// @brief 修改当前的tempProgramme
/// @param tempProgramme 
void setTempProgramme(RawProgramme *tempProgramme);
/// @brief 修改当前的tempProgramme
/// @param tempFunction
void setTempFunction(RawFunction *tempFunction);
/// @brief 修改当前的BasicBlock
/// @param tempBasicBlock 
void setTempBasicBlock(RawBasicBlock *tempBasicBlock);
#endif