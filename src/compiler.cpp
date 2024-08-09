#include "include/common.h"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
extern void backend_advanced(RawProgramme *& programme);
void OptimizeMem2Reg(RawProgramme *&programme);
extern void OptimizeDCE(RawProgramme *&programme);
extern void OptimizeConstCombine(RawProgramme *&prgramme);
extern void OptimizeSCCP(RawProgramme *&programme);
extern void MarkUseDef(RawProgramme *&programme);
extern void OptimizeLCSE(RawProgramme *programme);
extern void OptimizeGCSE(RawProgramme *programme);
extern void OptimizeFuncInline(RawProgramme *IR);
extern void OptimizeLoop(RawProgramme *IR);
extern void OptimizeLoopUnroll(RawProgramme *IR);
int main(int argc, const char *argv[]) {
  // 解析命令行参数. 测试脚本/评测平台要求你的编译器能接收如下参数:
  // compiler0 -S1 -o2 输出文件3 输入文件4
  // compiler0 -riscv1 hello.c2 -o3 hello.S4
  assert(argc == 5 || argc == 6);
  auto mode = argv[1];
  auto input = argv[4];
  auto OutMode = argv[2];
  auto output = argv[3];
  auto optMode = (argc == 6) ? argv[5]:nullptr;
  assert(strcmp(OutMode,"-o") == 0);

  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input, "r");
  assert(yyin);

  // 调用 parser 函数, parser 函数会进一步调用 lexer 解析输入文件的
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);
  freopen(output,"w",stdout);
  RawProgramme *irGraph;
  ast->generateGraph(irGraph);
  // if(optMode != nullptr && strcmp(optMode,"-O1") == 0) {
     // GeneratorIRTxt(irGraph,false);
      MarkUseDef(irGraph);
      //OptimizeFuncInline(irGraph);
      GeneratorDT(irGraph,0);
      //  循环优化需要基于支配树
    //  OptimizeLoop(irGraph);
      OptimizeMem2Reg(irGraph);
      OptimizeLoopUnroll(irGraph);
      GeneratorIRTxt(irGraph,true);
      //mem2regTop(irGraph);
      //GeneratorIRTxt(irGraph,true);
      //DCE(irGraph);
      //GeneratorIRTxt(irGraph,true);
      //  DCE(irGraph);
      //GeneratorIRTxt(irGraph,true);
      //ConstCombine(irGraph);
      // DCE(irGraph);
      // OptimizeSCCP(irGraph);
      //GeneratorIRTxt(irGraph,true);
      // GeneratorDT(irGraph,0);
      // OptimizeGCSE(irGraph);
      // OptimizeLCSE(irGraph);
      //GeneratorIRTxt(irGraph,true);
      // exitSSA(irGraph);
  // }
  if(strcmp(mode,"-riscv") == 0 || strcmp(mode,"-S") == 0) {
    // OptimizeFuncInline(irGraph);
    // exitSSA(irGraph);
    backend_advanced(irGraph);
  } 
  else if(strcmp(mode,"-koopa") == 0) {
    // OptimizeFuncInline(irGraph);
    // GeneratorIRTxt(irGraph,false);
  }
  else if(strcmp(mode,"-cfg") == 0){
    // OptimizeFuncInline(irGraph);
    GeneratorDT(irGraph,3);//控制流图
    // GeneratorDT(irGraph,2);//支配树
  }
  else if(strcmp(mode,"-astT") == 0){
    Generator_ast(ast,1);
  }
  else if(strcmp(mode,"-astG") == 0){
    Generator_ast(ast,2);
  }
  return 0;
}