%pointer
%option noyywrap
%option yylineno

%{
    #include <ast.h>
    #include <parser.hpp>   
    #include <iostream>
    #include <cstring>
    #include <cstdlib>
    #include <memory>

    static int yy_next_column = 1;
    int yycolumn = 1;

    #define HANDLE_COLUMN yycolumn = yy_next_column; yy_next_column += strlen(yytext)
%}

INT     -?([0-9])|([1-9][0-9]*)
NEWLINE \'\\n\'
BACKSLASH \\\\
HOR_TAB \\t
SINGLE_QUOTE \\\'

/* no need to add others */

%%

\t {HANDLE_COLUMN;}

\n {HANDLE_COLUMN; yy_next_column = 1;}

" " {HANDLE_COLUMN;}

({INT}?"."[0-9]+)|({INT}"."[0-9]*) {
    HANDLE_COLUMN;
    yylval = std::make_shared<ConstNode>(.0L); // not implemented yet
    return FLOAT;
}

{INT} { // integer constant
    HANDLE_COLUMN;
    yylval = std::make_shared<ConstNode>(atoll(yytext));
    return INT;
}

\'{NEWLINE}\' {
    HANDLE_COLUMN;
    yylval = std::make_shared<ConstNode>('\n');
    return CHAR;
}

\'{BACKSLASH}\' {
    HANDLE_COLUMN;
    yylval = std::make_shared<ConstNode>('\\');
    return CHAR;
}

\'{SINGLE_QUOTE}\' {
    HANDLE_COLUMN;
    yylval = std::make_shared<ConstNode>('\'');
    return CHAR;
}

\'{HOR_TAB}\' {
    HANDLE_COLUMN;
    yylval = std::make_shared<ConstNode>('\t');
    return CHAR;
}

\'.\' { // letter char
    HANDLE_COLUMN;
    yylval = std::make_shared<ConstNode>(yytext[1]);
    return CHAR;
}

"+" { 
    HANDLE_COLUMN;
    yylval = std::make_shared<AddNode>();
    return ADD;
}

"-" {
    HANDLE_COLUMN;
    yylval = std::make_shared<SubNode>();
    return SUB;
}

"/" {
    HANDLE_COLUMN;
    yylval = std::make_shared<DivNode>();
    return DIV;
}

"%" {
    HANDLE_COLUMN;
    yylval = std::make_shared<RemNode>();
    return REM;
}

"*" {
    HANDLE_COLUMN;
    yylval = std::make_shared<MulNode>();
    return MUL;
}

CONS {
    HANDLE_COLUMN;
    yylval = std::make_shared<ConsNode>();
    return CONS;
}

HEAD {
    HANDLE_COLUMN;
    yylval = std::make_shared<HeadNode>();
    return HEAD;
}

TAIL {
    HANDLE_COLUMN;
    yylval = std::make_shared<TailNode>();
    return TAIL;
}

PRINT {
    HANDLE_COLUMN;
    yylval = std::make_shared<PrintNode>();
    return PRINT;
}

GET_INT {
    HANDLE_COLUMN;
    yylval = std::make_shared<GetIntNode>();
    return GET_INT;
}

GET_FLOAT {
    HANDLE_COLUMN;
    yylval = std::make_shared<GetFloatNode>();
    return GET_FLOAT;
}

GET_CHAR {
    HANDLE_COLUMN;
    yylval = std::make_shared<GetCharNode>();
    return GET_CHAR;
}

@ {
    HANDLE_COLUMN;
    yylval = std::make_shared<Node>();
    return LAMBDA; 
}

"." { 
    HANDLE_COLUMN; 
    yylval = std::make_shared<Node>();
    return '.'; 
}

"(" { 
    HANDLE_COLUMN; 
    yylval = std::make_shared<Node>();
    return '('; 
}

")" { 
    HANDLE_COLUMN; 
    yylval = std::make_shared<Node>();
    return ')'; 
}

[a-zA-Z_]+[0-9]* { 
    HANDLE_COLUMN;
    yylval = std::make_shared<VarNode>(yytext);
    return VAR;
}

. { 
    HANDLE_COLUMN;
    std::cerr << "Unrecognized token\n"; 
}

%%