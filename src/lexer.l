%option noyywrap
%option yylineno

%{
#include "token.h"
%}

%%

[ \t\n]+                    { /* ignore whitespace */ }
[a-z_A-Z]+[a-z_A-Z0-9]*     { return TokenType::Identifier; }
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
\/\/(.*)                    { /* ignore comments */ }
"/*"([^*]|\*+[^*/])*\*+"/"  { /* ignore multi-line comments */ }
%%