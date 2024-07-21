%option noyywrap
%option yylineno

%top {
#include "parser.h"
}

%%

[ \t\n]+                    { /* ignore whitespace */ }
\/\/(.*)                    { /* ignore comments */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* ignore multi-line comments */ }
[0-9]+                      { yylval.str = strdup(yytext); return Number; }
\"([^\\"]|\\.)*\"           {
    size_t len = yyleng - 2;
    char* str = (char*)malloc(len + 1);
    if (!str) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    strncpy(str, yytext + 1, len);
    str[len] = '\0';
    yylval.str = str;
    yytext = str;
    return String;
}
"++"                        { return Increment; }
"--"                        { return Decrement; }
"+="                        { return IncrementAssign; }
"-="                        { return DecrementAssign; }
"+"                         { return Plus; }
"-"                         { return Minus; }
"*"                         { return Multiply; }
"/"                         { return Divide; }
"%"                         { return Modulo; }
"="                         { return Assign; }
"=="                        { return Equal; }
"!="                        { return NotEqual; }
"<"                         { return Less; }
"<="                        { return LessEqual; }
">"                         { return Greater; }
">="                        { return GreaterEqual; }
"&&"                        { return And; }
"||"                        { return Or; }
"!"                         { return Not; }
"define"                    { return Define; }
"function"                  { return Function; }
"if"                        { return If; }
"else"                      { return Else; }
"while"                     { return While; }
"for"                       { return For; }
"return"                    { return Return; }
"u8"                        { return U8; }
"u16"                       { return U16; }
"u32"                       { return U32; }
"u64"                       { return U64; }
"i8"                        { return I8; }
"i16"                       { return I16; }
"i32"                       { return I32; }
"i64"                       { return I64; }
"f32"                       { return F32; }
"f64"                       { return F64; }
"bool"                      { return Bool; }
"str"                       { return Str; }
"true"                      { return True; }
"false"                     { return False; }
":"                         { return Colon; }
";"                         { return Semicolon; }
"("                         { return LParen; }
")"                         { return RParen; }
"{"                         { return LBrace; }
"}"                         { return RBrace; }
","                         { return Comma; }
"->"                        { return Arrow; }
"["                         { return LBracket; }
"]"                         { return RBracket; }
[a-z_A-Z]+[a-z_A-Z0-9]*     { yylval.str = strdup(yytext); return Identifier; }

%%
