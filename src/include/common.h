#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <cstdlib>

#include "koopa.h"

#include "midend/AST/BaseAST.h"
#include "midend/AST/BlockAST.h"
#include "midend/AST/DeclAST.h"
#include "midend/AST/ExpAST.h"
#include "midend/AST/FuncAST.h"
#include "midend/AST/LValAST.h"
#include "midend/AST/OpAST.h"
#include "midend/AST/StmtAST.h"

#include "ValueTable.h"
using namespace std;
#pragma once

