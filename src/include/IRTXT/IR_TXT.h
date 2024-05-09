#pragma once
#include <map>
#include <string>
#include "../midend/IR/IRGraph.h"
void GeneratorIRTxt(RawProgramme * &programme);
void Visit_Fun(const RawFunctionP &func);
void Visit_BBS(const RawBasicBlockP &bb);
void Visit_Value(const RawValueP &value);
void Visit_Slice(const RawSlice &slice);

void Visit_Return(const RawValueP &value);
void Visit_Integer(const RawValueP &value);
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

static map<const RawValue *,string> Symbol_List;
static map<const RawValue *,string> Global_List;

void init_symbol();
void alloc_symbol(const RawValueP &value);
static int symbol_num = 0;
static int symbol_id = 0;
