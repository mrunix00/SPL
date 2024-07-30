%{
    #include <iostream>
    #include <vector>
    #include <stdexcept>
    #include "ast.h"
    #include "lexer.h"
    extern int yylex(void);

    static void yyerror(const char* s) {
        throw std::runtime_error(s);
    }

    std::vector<AbstractSyntaxTree*> root;
    std::vector<Declaration*>* declarations;
    std::vector<AbstractSyntaxTree*>* expressions;
    std::vector<AbstractSyntaxTree*>* elements;
%}

%union {
    void* ast;
    char* str;
}

%token Plus Minus Multiply Divide Modulo Assign
%token Increment Decrement IncrementAssign DecrementAssign
%token Equal NotEqual Less Greater LessEqual GreaterEqual And Or Not
%token Define Function If Else While For Return
%token Int UInt F64 Bool True False Str
%token Colon Comma Semicolon Arrow Newline
%token LParen RParen LBrace RBrace LBracket RBracket

%token <str> Number String Identifier
%type <ast> Expression Expressions VarType ScopedBody TypeCast FunctionCall IfStatement WhileStatement ForLoop
%type <ast> ArgumentDeclaration ArgumentDeclarationsList Arguments FunctionDeclaration UnaryExpression
%type <ast> List Elements

%left Plus Minus
%left Multiply Divide

%%

input: | Statements;
Statements: | Statements Statement;
Statement:
    Expression Semicolon {
        root.push_back(static_cast<AbstractSyntaxTree*>($1));
    }
;

List:
    LBracket Elements RBracket {
        $$ = new List(*static_cast<std::vector<AbstractSyntaxTree*>*>($2));
    }
    ;

Elements:
    | Elements Comma Expression {
        elements->push_back(static_cast<AbstractSyntaxTree*>($3));
        $$ = elements;
    }
    | Expression {
        elements = new std::vector<AbstractSyntaxTree*>();
        elements->push_back(static_cast<AbstractSyntaxTree*>($1));
        $$ = elements;
    }

IfStatement:
    If Expression ScopedBody {
        $$ = new IfStatement(static_cast<AbstractSyntaxTree*>($2), static_cast<AbstractSyntaxTree*>($3));
    }
    | If Expression ScopedBody Else ScopedBody {
        $$ = new IfStatement(static_cast<AbstractSyntaxTree*>($2), static_cast<AbstractSyntaxTree*>($3), static_cast<AbstractSyntaxTree*>($5));
    }
;

WhileStatement:
    While Expression ScopedBody {
        $$ = new WhileStatement(static_cast<AbstractSyntaxTree*>($2), static_cast<AbstractSyntaxTree*>($3));
    }
;

ForLoop:
    For Expression Semicolon Expression Semicolon Expression ScopedBody {
        $$ = new ForLoop(static_cast<AbstractSyntaxTree*>($2), static_cast<AbstractSyntaxTree*>($4), static_cast<AbstractSyntaxTree*>($6), static_cast<AbstractSyntaxTree*>($7));
    }
;

VarType:
    Int  { $$ = new Node({Int, "int"}); }
    | UInt { $$ = new Node({UInt, "uint"}); }
    | F64 { $$ = new Node({F64, "f64"}); }
    | Bool { $$ = new Node({Bool, "bool"}); }
    | Str { $$ = new Node({Str, "str"}); }
;

ArgumentDeclaration:
    Identifier Colon VarType {
        $$ = new Declaration(static_cast<AbstractSyntaxTree*>($3), Node({Identifier, $1}));
    }
;

ArgumentDeclarationsList:
    ArgumentDeclaration {
        declarations = new std::vector<Declaration*>();
        declarations->push_back(static_cast<Declaration*>($1));
        $$ = declarations;
    }
    | ArgumentDeclarationsList Comma ArgumentDeclaration {
        declarations->push_back(static_cast<Declaration*>($3));
        $$ = declarations;
    }
;

FunctionDeclaration:
    Function LParen ArgumentDeclarationsList RParen Arrow VarType {
        $$ = new FunctionDeclaration(static_cast<AbstractSyntaxTree*>($6), *static_cast<std::vector<Declaration*>*>($3));
        delete static_cast<std::vector<Declaration*>*>($3);
    }
;

TypeCast:
    LParen Expression RParen Expression {
        $$ = new TypeCast(static_cast<AbstractSyntaxTree*>($4), static_cast<AbstractSyntaxTree*>($2));
    }
;

Expressions:
    Expression Semicolon {
        expressions = new std::vector<AbstractSyntaxTree*>();
        expressions->push_back(static_cast<AbstractSyntaxTree*>($1));
        $$ = expressions;
    }
    | Expressions Expression Semicolon {
        expressions->push_back(static_cast<AbstractSyntaxTree*>($2));
        $$ = expressions;
    }
;

ScopedBody:
    LBrace Expressions RBrace {
        $$ = new ScopedBody(*static_cast<std::vector<AbstractSyntaxTree*>*>($2));
    }
;

Arguments:
    Expression {
        expressions = new std::vector<AbstractSyntaxTree*>();
        expressions->push_back(static_cast<AbstractSyntaxTree*>($1));
        $$ = expressions;
    }
    | Arguments Comma Expression {
        expressions->push_back(static_cast<AbstractSyntaxTree*>($3));
        $$ = expressions;
    }
;

FunctionCall:
    Identifier LParen RParen {
        $$ = new FunctionCall(Node({Identifier, $1}), {});
    }
    | Identifier LParen Arguments RParen {
        $$ = new FunctionCall(Node({Identifier, $1}), *static_cast<std::vector<AbstractSyntaxTree*>*>($3));
    }
;

UnaryExpression:
    Expression Increment {
        $$ = new UnaryExpression(static_cast<AbstractSyntaxTree*>($1), {Increment, "++"}, UnaryExpression::Side::RIGHT);
    }
    | Increment Expression {
        $$ = new UnaryExpression(static_cast<AbstractSyntaxTree*>($2), {Increment, "++"}, UnaryExpression::Side::LEFT);
    }
    | Expression Decrement {
        $$ = new UnaryExpression(static_cast<AbstractSyntaxTree*>($1), {Decrement, "--"}, UnaryExpression::Side::RIGHT);
    }
    | Decrement Expression {
        $$ = new UnaryExpression(static_cast<AbstractSyntaxTree*>($2), {Decrement, "--"}, UnaryExpression::Side::LEFT);
    }
;

Expression:
    Identifier { $$ = new Node({Identifier, $1}); }
    | False { $$ = new Node({False, "false"}); }
    | True { $$ = new Node({True, "true"}); }
    | Number { $$ = new Node({Number, $1}); }
    | String { $$ = new Node({String, $1}); }
    | FunctionDeclaration { $$ = $1; }
    | VarType { $$ = $1; }
    | ScopedBody { $$ = $1; }
    | TypeCast { $$ = $1; }
    | FunctionCall { $$ = $1; }
    | IfStatement { $$ = $1; }
    | WhileStatement { $$ = $1; }
    | ForLoop { $$ = $1; }
    | UnaryExpression { $$ = $1; }
    | List { $$ = $1; }
    | Return Expression {
        $$ = new ReturnStatement(static_cast<AbstractSyntaxTree*>($2));
    }
    | Define Identifier Colon Expression {
        $$ = new Declaration(static_cast<AbstractSyntaxTree*>($4), Node({Identifier, $2}));
    }
    | Define Identifier Colon Expression Assign Expression {
        $$ = new Declaration(static_cast<AbstractSyntaxTree*>($4), Node({Identifier, $2}), static_cast<AbstractSyntaxTree*>($6));
    }
    | Define Identifier Assign Expression {
        $$ = new Declaration(Node({Identifier, $2}), static_cast<AbstractSyntaxTree*>($4));
    }
    | Expression Plus Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Plus, "+"});
    }
    | Expression Minus Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Minus, "-"});
    }
    | Expression Multiply Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Multiply, "*"});
    }
    | Expression Divide Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Divide, "/"});
    }
    | Expression Modulo Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Modulo, "%"});
    }
    | Expression Assign Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Assign, "="});
    }
    | Expression Equal Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Equal, "=="});
    }
    | Expression NotEqual Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {NotEqual, "!="});
    }
    | Expression Less Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Less, "<"});
    }
    | Expression Greater Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {Greater, ">"});
    }
    | Expression LessEqual Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {LessEqual, "<="});
    }
    | Expression GreaterEqual Expression {
        $$ = new BinaryExpression(static_cast<AbstractSyntaxTree*>($1), static_cast<AbstractSyntaxTree*>($3), {GreaterEqual, ">="});
    }
;
%%

std::vector<AbstractSyntaxTree*> parse(const char *input) {
    root.clear();
    yy_scan_string(input);
    try {
        yyparse();
    } catch (std::runtime_error&) {
        throw std::runtime_error("Failed to parse input: " + std::string(input));
    }
    return root;
}
