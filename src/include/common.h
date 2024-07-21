#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <cstdlib>


#include "midend/AST/BaseAST.h"
#include "midend/AST/BlockAST.h"
#include "midend/AST/DeclAST.h"
#include "midend/AST/ExpAST.h"
#include "midend/AST/FuncAST.h"
#include "midend/AST/LValAST.h"
#include "midend/AST/OpAST.h"
#include "midend/AST/StmtAST.h"
#include "midend/AST/ArrayAST.h"

#include "midend/IR/IRGraph.h"
#include "IRTXT/IR_TXT.h"
#include "IRTXT/AstPrint.h"
#include "midend/SSA/DT.h"
#include "midend/SSA/PHI.h"
#include "midend/SSA/rename.h"
#include "midend/SSA/ExitSSA.h"


using namespace std;
#pragma once

