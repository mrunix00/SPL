%{
    #include <iostream>
    #include <vector>
    #include "ast.h"
    #include "lexer.h"
    extern int yylex(void);

    static void yyerror(const char* s) {
        fprintf(stderr, "%s\n", s);
    }

    AbstractSyntaxTree* root = nullptr;
    std::vector<Declaration*>* declarations;
    std::vector<AbstractSyntaxTree*>* Expressions;
%}

%union {
    void*   ast;
    char*   str;
}

%token String Plus Minus Multiply Divide Modulo Assign
%token Equal NotEqual Less Greater LessEqual GreaterEqual And Or Not
%token Define Function If Else While Return
%token U8 U16 U32 U64 I8 I16 I32 I64 F32 F64 Bool True False
%token Colon Comma Semicolon Arrow Newline
%token LParen RParen LBrace RBrace LBracket RBracket

%token <str> Number Identifier
%type <ast> Expression Expressions VarType ScopedBody TypeCast
%type <ast> ArgumentDeclaration ArgumentDeclarationsList FunctionDeclaration

%left Plus Minus
%left Multiply
%left Divide

%%

input:
    Expression Semicolon {
        root = (AbstractSyntaxTree*) $1;
    }
    ;

VarType:
    U8 { $$ = new Node({U8, "u8"}); }
    | U16 { $$ = new Node({U16, "u16"}); }
    | U32 { $$ = new Node({U32, "u32"}); }
    | U64 { $$ = new Node({U64, "u64"}); }
    | I8 { $$ = new Node({I8, "i8"}); }
    | I16 { $$ = new Node({I16, "i16"}); }
    | I32 { $$ = new Node({I32, "i32"}); }
    | I64 { $$ = new Node({I64, "i64"}); }
    | F32 { $$ = new Node({F32, "f32"}); }
    | F64 { $$ = new Node({F64, "f64"}); }
    ;

ArgumentDeclaration:
    Identifier Colon VarType {
        $$ = new Declaration((AbstractSyntaxTree*) $3, Node({Identifier, $1}));
    }
    ;

ArgumentDeclarationsList:
    ArgumentDeclaration {
        declarations = new std::vector<Declaration*>();
        declarations->push_back((Declaration*) $1);
        $$ = declarations;
    }
    | ArgumentDeclarationsList Comma ArgumentDeclaration {
        declarations->push_back((Declaration*) $3);
        $$ = declarations;
    }
    ;

FunctionDeclaration:
    Function LParen ArgumentDeclarationsList RParen Arrow VarType {
        $$ = new FunctionDeclaration((AbstractSyntaxTree*) $6, *(std::vector<Declaration*>*) $3);
        delete (std::vector<Declaration*>*) $3;
    }
    ;

TypeCast:
    LParen Expression RParen Expression  {
        $$ = new TypeCast((AbstractSyntaxTree*) $4, (AbstractSyntaxTree*) $2);
    }
    ;

Expressions:
    Expression {
        Expressions = new std::vector<AbstractSyntaxTree*>();
        Expressions->push_back((AbstractSyntaxTree*) $1);
        $$ = Expressions;
    }
    | Expressions Semicolon Expression {
        Expressions->push_back((AbstractSyntaxTree*) $3);
        $$ = Expressions;
    }
    ;

ScopedBody:
    LBrace Expressions RBrace {
        $$ = new ScopedBody(*(std::vector<AbstractSyntaxTree*>*) $2);
        delete $2;
    }
    ;

Expression:
    Identifier { $$ = new Node({Identifier, $1}); }
    | Number { $$ = new Node({Number, $1}); }
    | FunctionDeclaration { $$ = $1; }
    | VarType { $$ = $1; }
    | ScopedBody { $$ = $1; }
    | TypeCast { $$ = $1; }
    | Expression Plus Expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Plus, "+"});
    }
    | Expression Minus Expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Minus, "-"});
    }
    | Expression Multiply Expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Multiply, "*"});
    }
    | Expression Divide Expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Divide, "/"});
    }
    | Expression Modulo Expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Modulo, "%"});
    }
    | Return Expression {
        $$ = new ReturnStatement((AbstractSyntaxTree*) $2);
    }
    | Define Identifier Colon Expression {
        $$ = new Declaration((AbstractSyntaxTree*) $4, Node({Identifier, $2}));
    }
    | Define Identifier Colon Expression Assign Expression {
        $$ = new Declaration((AbstractSyntaxTree*) $4, Node({Identifier, $2}), (AbstractSyntaxTree*) $6);
    }
    | Define Identifier Assign Expression {
        $$ = new Declaration(Node({Identifier, $2}), (AbstractSyntaxTree*) $4);
    }
    ;
%%


AbstractSyntaxTree* parse(const char *input) {
    yy_scan_string(input);
    if (yyparse() != 0) throw std::runtime_error("Failed to parse input: " + std::string(input));
    return root;
}
