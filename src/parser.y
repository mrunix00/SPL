%{
    #include <iostream>
    #include "ast.h"
    #include "lexer.h"
    extern int yylex(void);

    static void yyerror(const char* s) {
        fprintf(stderr, "%s\n", s);
    }

    AbstractSyntaxTree* root = nullptr;
%}

%union {
    void* ast;
    char* str;
}

%token String Plus Minus Multiply Divide Modulo Assign
%token Equal NotEqual Less Greater LessEqual GreaterEqual And Or Not
%token Define If Else While Return U8 U16 U32 U64 I8 I16 I32 I64
%token F32 F64 Bool True False Identifier

%token <str> Number
%type <ast> expression

%left Plus Minus
%left Multiply
%left Divide

%%

input:
    expression {
        root = (AbstractSyntaxTree*) $1;
    }
    ;

expression:
    Number {
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
    ;
%%


AbstractSyntaxTree* parse(const char *input) {
    yy_scan_string(input);
    yyparse();
    return root;
}
