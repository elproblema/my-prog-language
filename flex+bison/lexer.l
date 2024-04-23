%pointer
%option noyywrap
%option header-file="scanner.h"
%{
    #include <ast.h>
    #include <parser.hpp>   
    #include <iostream>
    #include <cstdio>
%}

INT     -?([0-9])|([1-9][0-9]*)
NEWLINE \'\\n\'
BACKSLASH \\\\
HOR_TAB \\t
SINGLE_QUOTE \\\'

/* no need to add others */

%%

\t

\n

({INT}?\.[0-9]+)|({INT}\.[0-9]*) { // float constant

}

{INT} { // integer constant
    yylval = new ConstNode(atoll(yytext));
}

\'{NEWLINE}\' {
    yylval = new ConstNode('\n');
    return CHAR;
}

\'{BACKSLASH}\' {
    yylval = new ConstNode('\\');
    return CHAR;
}

\'{SINGLE_QUOTE}\' {
    yylval = new ConstNode('\'');
    return CHAR;
}

\'{HOR_TAB}\' {
    yylval = new ConstNode('\t');
    return CHAR;
}

\'.\' { // letter char
    yylval = new ConstNode(yytext[1]);
    return CHAR;
}

"+" { 
    yylval = new AddNode();
    return ADD;
}

- {
    yylval = new SubNode();
    return SUB;
}

"/" {
    yylval = new DivNode();
    return DIV;
}

% {
    yylval = new RemNode();
    return REM;
}

"*" {
    yylval = new MulNode();
    return MUL;
}

CONS {
    yylval = new ConsNode();
    return CONS;
}

HEAD {
    yylval = new HeadNode();
    return HEAD;
}

TAIL {
    yylval = new TailNode();
    return TAIL;
}

PRINT {
    yylval = new PrintNode();
    return PRINT;
}

GET_INT {
    yylval = new GetIntNode();
    return GET_INT;
}

GET_FLOAT {
    yylval = new GetFloatNode();
    return GET_FLOAT;
}

GET_CHAR {
    yylval = new GetCharNode();
    return GET_CHAR;
}

@ { return LAMBDA; }

"." { return '.'; }

"(" { return '('; }

")" { return ')'; }

[a-zA-Z_]+[0-9]* { 
    yylval = new VarNode(yytext);
    return VAR;
}

. { std::cerr << "Unrecognized charachter\n"; }

%%