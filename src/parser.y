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
    std::vector<Declaration*> declarations;
%}

%union {
    void*   ast;
    char*   str;
}

%token String Plus Minus Multiply Divide Modulo Assign
%token Equal NotEqual Less Greater LessEqual GreaterEqual And Or Not
%token Define Function If Else While Return
%token U8 U16 U32 U64 I8 I16 I32 I64 F32 F64 Bool True False
%token Colon Comma Semicolon Arrow
%token LParen RParen LBrace RBrace LBracket RBracket

%token <str> Number
%token <str> Identifier
%type <ast> expression
%type <ast> VarType
%type <ast> ArgumentDeclaration
%type <ast> ArgumentList
%type <ast> FunctionDeclaration

%left Plus Minus
%left Multiply
%left Divide

%%

input:
    expression {
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

ArgumentList:
    ArgumentDeclaration {
        declarations.clear();
        declarations.push_back((Declaration*) $1);
        $$ = &declarations;
    }
    | ArgumentList Comma ArgumentDeclaration {
        declarations.push_back((Declaration*) $3);
        $$ = &declarations;
    }
    ;

FunctionDeclaration:
    Function LParen ArgumentList RParen Arrow VarType {
        $$ = new FunctionDeclaration((AbstractSyntaxTree*) $6, *(std::vector<Declaration*>*) $3);
    }
    ;

expression:
    Identifier {
        $$ = new Node({Identifier, $1});
    }
    | Number {
        $$ = new Node({Number, $1});
    }
    | expression Plus expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Plus, "+"});
    }
    | expression Minus expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Minus, "-"});
    }
    | expression Multiply expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Multiply, "*"});
    }
    | expression Divide expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Divide, "/"});
    }
    | expression Modulo expression {
        $$ = new BinaryExpression((AbstractSyntaxTree*) $1, (AbstractSyntaxTree*) $3, {Modulo, "%"});
    }
    | Define Identifier Colon VarType {
        $$ = new Declaration((AbstractSyntaxTree*) $4, Node({Identifier, $2}));
    }
    | Define Identifier Colon VarType Assign expression {
        $$ = new Declaration((AbstractSyntaxTree*) $4, Node({Identifier, $2}), (AbstractSyntaxTree*) $6);
    }
    | Define Identifier Colon FunctionDeclaration {
        $$ = new Declaration((AbstractSyntaxTree*) $4, Node({Identifier, $2}));
    }
    ;
%%


AbstractSyntaxTree* parse(const char *input) {
    yy_scan_string(input);
    if (yyparse() != 0) throw std::runtime_error("Failed to parse input: " + std::string(input));
    return root;
}
