#include "include/common.h"
#include <chrono>
using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
extern void backend(RawProgramme *& programme);
extern void DCE(RawProgramme *&programme);
extern void ConstCombine(RawProgramme *&prgramme);
extern void CondCCP(RawProgramme *&programme);
void BlockEliminate(RawProgramme *&programme);
void InstMerge(RawProgramme *&programmer);

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
  auto start_time = std::chrono::high_resolution_clock::now();
  cerr << "start" << endl;
  ast->generateGraph(irGraph);
  BlockEliminate(irGraph);
  auto end_time = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
  std::cerr << "程序运行时间: " << duration.count() << " 微秒" << std::endl;
  //InstMerge(irGraph);
  //if(optMode != nullptr && strcmp(optMode,"-O1") == 0) {
      //GeneratorIRTxt(irGraph,false);
      //OptimizeFuncInline(irGraph);
      // GeneratorDT(irGraph,0);
      // GeneratorDT(irGraph,3);
      // AddPhi(irGraph);
      // renameValue(irGraph);
      //  循环优化需要基于支配树
    //  OptimizeLoop(irGraph);
      // OptimizeMem2Reg(irGraph);
      //GeneratorIRTxt(irGraph,true);
      //mem2regTop(irGraph);
      //GeneratorIRTxt(irGraph,true);
      //DCE(irGraph);
      //GeneratorIRTxt(irGraph,true);
      //  DCE(irGraph);
      //GeneratorIRTxt(irGraph,true);
      // ConstCombine(irGraph);
      // DCE(irGraph);
      //  CondCCP(irGraph);
      // exitSSA(irGraph);
  //}
  start_time = std::chrono::high_resolution_clock::now();
  cerr << "start" << endl;
  if(strcmp(mode,"-riscv") == 0 || strcmp(mode,"-S") == 0) {
    GeneratorDT(irGraph,0);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cerr << "程序运行时间: " << duration.count() << " 微秒" << std::endl;
    backend(irGraph);
  } 
  else if(strcmp(mode,"-koopa") == 0) {
     GeneratorIRTxt(irGraph,false);
  }
  else if(strcmp(mode,"-cfg") == 0){
    // OptimizeFuncInline(irGraph);
    // GeneratorDT(irGraph,3);控制流图
    GeneratorDT(irGraph,2);//支配树
  }
  else if(strcmp(mode,"-astT") == 0){
    Generator_ast(ast,1);
  }
  else if(strcmp(mode,"-astG") == 0){
    Generator_ast(ast,2);
  }
  return 0;
}