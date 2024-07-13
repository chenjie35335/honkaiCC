%option noyywrap
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>
#include <iostream>

// 因为 Flex 会用到 Bison 中关于 token 的定义
// 所以需要 include Bison 生成的头文件
#include "sysy.tab.hpp"

using namespace std;

%}

/* 空白符和注释 */
WhiteSpace    [ \t\n\r]*
LineComment   "//".*
Comments      "/*"([^\*]|(\*)*[^\*/])*(\*)*"*/"

/* 标识符 */
Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/* 整数字面量 */

Decimal       (0|[1-9][0-9]*)
Octal         0[0-7]*
Hexadecimal   (0x|0X)[0-9a-fA-F]+

/*float*/

FloatNumber      ([0-9]*\.?[0-9]+|[0-9]+\.)((e|E)[-+]?[0-9]+)?
HexFloatNumber   (0x|0X)([0-9a-fA-F]*\.?[0-9a-fA-F]+|[0-9a-fA-F]+\.)((p|P)[-+]?[0-9a-fA-F]+)?

/*运算符*/
%%

{WhiteSpace}    { /* 忽略, 不做任何操作 */ }
{LineComment}   { /* 忽略, 不做任何操作 */ }
{Comments}      { /* 忽略, 不做任何操作 */ }

"int"           { return INT; }
"return"        { return RETURN; }
"=="            { return EQ; }
"<="            { return LE; }
">="            { return GE; }
"!="            { return NE; }
"&&"            { return AND; }
"||"            { return OR; }
"if"            { return IF; }
"else"          { return ELSE; }
"const"         { return CONST; }
"while"         { return WHILE; }
"break"         { return BREAK; }
"continue"      { return CONTINUE; }
"void"          { return VOID; }
"float"         { return FLOAT;}

{Decimal}       { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Octal}         { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{Hexadecimal}   { yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; }
{FloatNumber}   { yylval.float_val = strtof(yytext, nullptr);return FLOAT_CONST; }
{HexFloatNumber}   { yylval.float_val = strtof(yytext, nullptr);return FLOAT_CONST; }


{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }
.               { return yytext[0]; }
        

%%