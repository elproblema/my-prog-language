%{
  #include <iostream>
  #include <ast.h>
  void yyerror(Node*, char const *);
  int yylex(void);
%}

%code requires {
  #include <iostream>
  #include <ast.h>
  void yyerror(char const *);
  int yylex(void);
}

%define api.value.type {Node*}
%parse-param {Node*& root}

%token INT
%token FLOAT
%token CHAR

%token LAMBDA // instead of lambda we will use @ for convinience

%token VAR

%token ADD
%token SUB
%token DIV
%token REM
%token MUL
%token CONS
%token HEAD
%token TAIL

%token PRINT
%token INPUT
%token GET_INT
%token GET_FLOAT
%token GET_CHAR

%nterm PROG
%nterm EXPR
%nterm NON_APP_EXPR

%start PROG

%% /* Grammar rules and actions follow. */

NON_APP_EXPR:
  INT 
| FLOAT
| CHAR 

| VAR

| ADD
| SUB
| DIV
| REM
| MUL
| CONS
| HEAD
| TAIL
| PRINT
| INPUT
| GET_INT
| GET_FLOAT
| GET_CHAR
| LAMBDA VAR '.' EXPR 
{
  $$ = new LambdaNode(std::move(dynamic_cast<VarNode&>(*$2)), $4);
  delete $2;
}
| '(' EXPR ')' { $$ = $2; }

EXPR:
  NON_APP_EXPR
| EXPR NON_APP_EXPR { $$ = new AppNode{$1, $2}; }

PROG:
  EXPR { root = $1; }

%%

void yyerror (Node*, char const *s)
{
  fprintf (stderr, "error");
}