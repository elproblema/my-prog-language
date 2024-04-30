%{
  #include <iostream>
  #include <ast.h>

  void yyerror(std::shared_ptr<Node>, char const *);
  int yylex(void);  
%}

%code requires {
  #include <iostream>
  #include <ast.h>
  void yyerror(char const *);
  int yylex(void);
}

%define api.value.type {std::shared_ptr<Node>}
%parse-param {std::shared_ptr<Node>& root}

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
%right '.'

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
  $$ = std::make_shared<LambdaNode>(std::move(dynamic_cast<VarNode&>(*$2)), $4);
  $$->SetLoc($1);
}
| '(' EXPR ')' 
{ 
  $$ = $2;
  $$->SetLoc($1);
}

EXPR:
  NON_APP_EXPR 
| EXPR NON_APP_EXPR { 
    $$ = std::make_shared<AppNode>($1, $2);
    $$->SetLoc($1); 
}

PROG:
  EXPR { root = $1; }

%%

void yyerror (std::shared_ptr<Node>, char const *s)
{
  fprintf (stderr, "error");
}
