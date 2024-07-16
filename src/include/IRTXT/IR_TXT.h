#ifndef FUTUROW_IR_TXT
#define FUTUROW_IR_TXT
#include <map>
#include <unordered_map>
#include <string>
#include "../midend/IR/IRGraph.h"
void GeneratorIRTxt(RawProgramme * &programme,bool isSSAmode);
void Visit_Fun(const RawFunctionP &func);
void Visit_BBS(const RawBasicBlockP &bb);
void Visit_Value(const RawValueP &value);
void Name_Fun(const RawFunctionP &func);
void Name_BBS(const RawBasicBlockP &bb);
void Name_Value(const RawValueP &value);

void Visit_Return(const RawValueP &value);
void Visit_Integer(const RawValueP &value);
void Visit_Float(const RawValueP &value);
void Visit_Binary(const RawValueP &value);
void Visit_Alloc(const RawValueP &value);
void Visit_Load(const RawValueP &value);
void Visit_Store(const RawValueP &value);
void Visit_Branch(const RawValueP &value);
void Visit_Jump(const RawValueP &value);
void Visit_Call(const RawValueP &value);
void Visit_Func_Args(const RawValueP &value);
void Visit_Global(const RawValueP &value);
void Visit_ZEROINIT(const RawValueP &value);
void visit_get_element(const RawValueP &value);
void visit_aggregate(const RawValueP &value);
void visit_get_ptr(const RawValueP &value);

static map<const RawValue *,string> Symbol_List;
static map<const RawValue *,string> Global_List;
static unordered_map<const RawValue *,int> var_id;

void init_symbol();
string GetValueType(const RawTypeP &ty);
void alloc_symbol(const RawValueP &value);
void alloc_ptr_symbol(const RawValueP &value);
static int symbol_num = 0;
static int ptr_idx = 0;
static bool SSAmode = false;


#endif