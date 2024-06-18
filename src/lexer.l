%option noyywrap
%option yylineno

%{
#include "token.h"
%}

%%

[ \t\n]+                    { /* ignore whitespace */ }
\/\/(.*)                    { /* ignore comments */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* ignore multi-line comments */ }
[0-9]+                      { return TokenType::Number; }
\"([^\\\"]|\\.)*\"          { return TokenType::String;}
"+"                         { return TokenType::Plus; }
"-"                         { return TokenType::Minus; }
"*"                         { return TokenType::Multiply; }
"/"                         { return TokenType::Divide; }
"%"                         { return TokenType::Modulo; }
"="                         { return TokenType::Assign; }
"=="                        { return TokenType::Equal; }
"!="                        { return TokenType::NotEqual; }
"<"                         { return TokenType::Less; }
"<="                        { return TokenType::LessEqual; }
">"                         { return TokenType::Greater; }
">="                        { return TokenType::GreaterEqual; }
"&&"                        { return TokenType::And; }
"||"                        { return TokenType::Or; }
"!"                         { return TokenType::Not; }
"define"                    { return TokenType::Define; }
"if"                        { return TokenType::If; }
"else"                      { return TokenType::Else; }
"while"                     { return TokenType::While; }
"return"                    { return TokenType::Return; }
"u8"                        { return TokenType::U8; }
"u16"                       { return TokenType::U16; }
"u32"                       { return TokenType::U32; }
"u64"                       { return TokenType::U64; }
"i8"                        { return TokenType::I8; }
"i16"                       { return TokenType::I16; }
"i32"                       { return TokenType::I32; }
"i64"                       { return TokenType::I64; }
"f32"                       { return TokenType::F32; }
"f64"                       { return TokenType::F64; }
"bool"                      { return TokenType::Bool; }
"true"                      { return TokenType::True; }
"false"                     { return TokenType::False; }
[a-z_A-Z]+[a-z_A-Z0-9]*     { return TokenType::Identifier; }

%%