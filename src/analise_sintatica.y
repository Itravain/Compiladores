%{
// GLC to C- language

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"
#include <string.h>
#define HASH_SIZE 128

#define YYSTYPE No*

int yylex(void);
int yyerror(char *);
extern char *yytext;
extern int yylinenum;

No* raizArvore = NULL;
char heapNameLexeme[MAXLEXEME];
int heapLinenum;
int heapNumberLexeme;
%}
%debug

%start programa

%token T_INT T_VOID T_IF T_ELSE T_WHILE T_RETURN
%token T_ID T_NUM
%token T_SEMICOLON T_COMMA T_ASSIGN T_LPAREN T_RPAREN T_LBRACE T_RBRACE T_LBRACKET T_RBRACKET
%token T_LT T_LEQ T_GT T_GEQ T_EQ T_NEQ
%token T_PLUS T_MINUS T_TIMES T_OVER T_MOD

%left T_LT T_LEQ T_GT T_GEQ T_EQ T_NEQ
%left T_PLUS T_MINUS
%left T_TIMES T_OVER T_MOD
%right T_ASSIGN
%nonassoc LOWER_THAN_ELSE
%nonassoc T_ELSE

%%

programa:
    declaracao_lista { raizArvore = $1; }
;

declaracao_lista:
      declaracao_lista declaracao {
        if ($$ != NULL) {
            $$ = $1;
            add_irmao($$, $2);
        } else {
            $$ = $2;
        }
    }
    | declaracao { $$ = $1; }
;

declaracao:
      var_declaracao { $$ = $1; }
    | fun_declaracao { $$ = $1; }
;

id:
      T_ID {
        strcpy(heapNameLexeme, id_lexema);
        heapLinenum = yylinenum;
      }
;

num:
      T_NUM {
        heapNumberLexeme = atoi(yytext);
        heapLinenum = yylinenum;
      }

var_declaracao:
      tipo_especificador id T_SEMICOLON {
        $$ = $1;
        $$->kind_node = declaration_k;
        $$->kind_union.decl = (DeclarationKind)var_k;
        $$->linha = yylinenum;
        YYSTYPE aux = create_node(yylinenum, heapNameLexeme, declaration_k, var_k);
        add_filho($$, aux);        
      }
    | tipo_especificador id T_LBRACKET num T_RBRACKET T_SEMICOLON {
        $$ = $1;
        $$->kind_node = declaration_k;
        $$->kind_union.decl = (DeclarationKind)array_k;
        $$->linha = yylinenum;
        YYSTYPE aux1 = create_node(yylinenum, heapNameLexeme, declaration_k, array_k);
        add_filho($$, aux1);
        char heapNumberLexemeStr[10];
        sprintf(heapNumberLexemeStr, "%d", heapNumberLexeme);
        YYSTYPE aux2 = create_node(yylinenum, heapNumberLexemeStr, declaration_k, constant_k);
        aux2->max_index = heapNumberLexeme;
        add_filho($$, aux2);
    }
;

tipo_especificador:
      T_INT { 
        $$ = create_node(yylinenum, "int", declaration_k, var_k);
       }
    | T_VOID {
        $$ = create_node(yylinenum, "void", declaration_k, var_k);
    }
;

fun_declaracao:
      tipo_especificador f_id T_LPAREN params T_RPAREN composto_decl {
        $$ = $1;
        $$->kind_node = declaration_k;
        $$->kind_union.decl = (DeclarationKind)fun_k;
        $2->kind_node = declaration_k;
        $2->kind_union.decl = (DeclarationKind)fun_k;
        add_filho($2, $4);
        add_filho($$, $2);
        add_filho($2, $6);
      }
    | tipo_especificador f_id T_LPAREN T_RPAREN composto_decl{
        $$ = $1;
        $$->kind_node = declaration_k;
        $$->kind_union.decl = (DeclarationKind)fun_k;
        $2->kind_node = declaration_k;
        $2->kind_union.decl = (DeclarationKind)fun_k;
        add_filho($$, $2);
        add_filho($2, $5);
      }
;
f_id:
      T_ID {
        $$ = create_node(yylinenum, id_lexema, expression_k, id_k);
      }
;
params:
      param_lista { $$ = $1; }
    | T_VOID {
        $$ = create_node(yylinenum, "void", expression_k, type_k);
    }
;

param_lista:
      param { $$ = $1; }
    | param_lista T_COMMA param {
        if ($1 != NULL) {
            $$ = $1;
            add_irmao($$, $3);
        } else { $$ = $3; }
    }
;

param:
      tipo_especificador T_ID {
        $$ = $1;
        YYSTYPE aux = create_node(yylinenum, id_lexema, declaration_k, param_k);
        add_filho($$, aux);
      }
    | tipo_especificador T_ID T_LBRACKET T_RBRACKET {
        $$ = $1;
        YYSTYPE aux = create_node(yylinenum, id_lexema, declaration_k, array_k);
        add_filho($$, aux);
    }
;

composto_decl:
      T_LBRACE local_declaracoes statement_lista T_RBRACE {
        if ($2 != NULL) {
            $$ = $2;
            add_irmao($$, $3);
        } else { $$ = $3; }
      }
;

local_declaracoes:
      local_declaracoes var_declaracao {
        if ($1 != NULL) {
            $$ = $1;
            add_irmao($$, $2);
        } else { $$ = $2; }
      }
    | /* vazio */ { $$ = NULL; }
;

statement_lista:
      statement { $$ = $1; }
    | statement_lista statement {
        if ($1 != NULL) {
            $$ = $1;
            add_irmao($$, $2);
        } else { $$ = $2; }
    }
;

statement:
      expressao_decl     { $$ = $1; }
    | composto_decl      { $$ = $1; }
    | selecao_decl       { $$ = $1; }
    | iteracao_decl      { $$ = $1; }
    | retorno_decl       { $$ = $1; }
;

expressao_decl:
      expressao T_SEMICOLON { $$ = $1; }
;

selecao_decl:
      T_IF T_LPAREN expressao T_RPAREN statement %prec LOWER_THAN_ELSE {
          $$ = create_node(yylinenum, "if", statement_k, if_k);
          /* If-statement: $3 e "then-statement": $5 */
          add_filho($$, $3);
          add_filho($$, $5);
      }
    | T_IF T_LPAREN expressao T_RPAREN statement T_ELSE statement {
        /* Cria no if-else */
        $$ = create_node(yylinenum, "if-else", statement_k, if_k); /* A Gabi não criou if_else_k */
        /* If-statement: $3, "then-statement": $5 e "else-statement": $7 */
        add_filho($$, $3);
        add_filho($$, $5);
        add_filho($$, $7);
    }
;

iteracao_decl:
      T_WHILE T_LPAREN expressao T_RPAREN statement {
        $$ = create_node(yylinenum, "while", statement_k, while_k);
        add_filho($$, $3);
        add_filho($$, $5);
      }
;

retorno_decl:
      T_RETURN T_SEMICOLON {
        $$ = create_node(yylinenum, "return", statement_k, return_k);
      }
    | T_RETURN expressao T_SEMICOLON {
        $$ = create_node(yylinenum, "return", statement_k, return_k);
        add_filho($$, $2);
    }
;

expressao:
      var T_ASSIGN expressao {
        $$ = create_node(yylinenum, "=", expression_k, assign_k);
        add_filho($$, $1);
        add_filho($$, $3);
      }
    | simples_expressao { $$ = $1; }
;

var:
      f_id { $$ = $1; }
    | f_id T_LBRACKET expressao T_RBRACKET { 
        $$ = $1;
        $$->kind_node = expression_k;
        $$->kind_union.expr = (ExpressionKind)arr_k;
        $3->kind_node = expression_k;
        $3->kind_union.expr = (ExpressionKind)var_k;
        add_filho($$, $3);
        }
;

simples_expressao:
      soma_expressao { $$ = $1; }
    | soma_expressao relacional soma_expressao {
        $$ = $2;
        $$->kind_node = expression_k;
        $$->kind_union.expr = (ExpressionKind)op_k;
        $$->linha = yylinenum;
        add_filho($$, $1);
        add_filho($$, $3);
    }
;

relacional:
      T_LT  { $$ = create_node(yylinenum, "<", expression_k, op_k); }
    | T_LEQ { $$ = create_node(yylinenum, "<=", expression_k, op_k); }
    | T_GT  { $$ = create_node(yylinenum, ">", expression_k, op_k); }
    | T_GEQ { $$ = create_node(yylinenum, ">=", expression_k, op_k); }
    | T_EQ  { $$ = create_node(yylinenum, "==", expression_k, op_k); }
    | T_NEQ { $$ = create_node(yylinenum, "!=", expression_k, op_k); }
;

soma_expressao:
      termo { $$ = $1; }
    | soma_expressao T_PLUS termo {
        $$ = create_node(yylinenum, "+", expression_k, op_k);
        add_filho($$, $1);
        add_filho($$, $3);
    }
    | soma_expressao T_MINUS termo {
        $$ = create_node(yylinenum, "-", expression_k, op_k);
        add_filho($$, $1);
        add_filho($$, $3);
    }
;

termo:
      fator { $$ = $1; }
    | termo T_TIMES fator {
        $$ = create_node(yylinenum, "*", expression_k, op_k);
        add_filho($$, $1);
        add_filho($$, $3);
    }
    | termo T_OVER fator {
        $$ = create_node(yylinenum, "/", expression_k, op_k);
        add_filho($$, $1);
        add_filho($$, $3);
    }
    | termo T_MOD fator {
        $$ = create_node(yylinenum, "%", expression_k, op_k);
        add_filho($$, $1);
        add_filho($$, $3);
    }
;

fator:
      T_LPAREN expressao T_RPAREN { $$ = $2; }
    | var { $$ = $1; }
    | ativacao { $$ = $1; }
    | T_NUM {
        $$ = create_node(yylinenum, yytext, expression_k, constant_k);
    }
;

ativacao:
      f_id T_LPAREN args T_RPAREN { 
        $$ = $1;
        $$->kind_node = expression_k;
        $$->kind_union.expr = (ExpressionKind)ativ_k;
        add_filho($$, $3);
        }
    | f_id T_LPAREN T_RPAREN { $$ = create_node(yylinenum, id_lexema, expression_k, ativ_k); }
;

args:
    args_lista { $$ = $1; }
;

args_lista:
      expressao { $$ = $1; }
    | args_lista T_COMMA expressao {
        if ($1 != NULL) {
            $$ = $1;
            add_irmao($$, $3);
        } else { $$ = $3; }
    }
;

%%

int yyerror(char *msg) {
    fprintf(stderr, "Erro de sintaxe na linha %d: Token inesperado: '%s'\n", yylinenum, yytext);
}
