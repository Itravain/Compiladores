#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../globals.h"
#define NUMMAXFILHOS 3
#define MAXLEXEME 25

No * create_node(int linha, const char *lexmema, NodeKind kind, int kind_union){
    No *node = malloc(sizeof(No));
    node->linha = linha;
    strncpy(node->lexmema, lexmema, MAXLEXEME);
    node->max_index = -1;
    node->kind_node = kind;
    node->pai = NULL;
    node->irmao = NULL;
    node->prev_irmao = NULL;
    for (int i = 0; i < NUMMAXFILHOS; i++) {
        node->filho[i] = NULL;
    }

    switch (kind){
        case statement_k:
            node->kind_union.stmt = (StatementKind)kind_union;
            break;
        case expression_k:
            node->kind_union.expr = (ExpressionKind)kind_union;
            break;
        case declaration_k:
            node->kind_union.decl = (DeclarationKind)kind_union;
            break;
    }

    return node;
}

No * add_filho(No *pai, No *filho){
    for (int i = 0; i < NUMMAXFILHOS; i++){
        if (pai->filho[i] == NULL) {
            pai->filho[i] = filho;
            filho->pai = pai;
            return pai;
        }
    }
    printf("Erro: Número máximo de filhos excedido.\n");
    free(filho);
    return pai;
}

No * add_irmao(No *irmao, No *novo){
    while (irmao->irmao != NULL){
        irmao = irmao->irmao;
    }
    irmao->irmao = novo;
    novo->prev_irmao = irmao;
    return novo;
}

void free_tree(No *tree){
    if (tree == NULL){return;}
    for (int i = 0; i < NUMMAXFILHOS; i++) {
        free_tree(tree->filho[i]);
    }
    free_tree(tree->irmao);
    free(tree);
}

void print_node(FILE *file, No *node){
    if (node == NULL){return;}
    char *kind_node;
    char *kind_union_str = "";

    switch (node->kind_node){
        case statement_k:
            kind_node = "statement";
            switch(node->kind_union.stmt) {
                case if_k: kind_union_str = "if"; break;
                case while_k: kind_union_str = "while"; break;
                case return_k: kind_union_str = "return"; break;
                case break_k: kind_union_str = "break"; break;
                case continue_k: kind_union_str = "continue"; break;
                case expression_statement_k: kind_union_str = "expression_statement"; break;
                default: kind_union_str = "unknown_statement"; break;
            }
            break;
        case expression_k:
            kind_node = "expression";
            switch(node->kind_union.expr) {
                case op_k: kind_union_str = "op"; break;
                case constant_k: kind_union_str = "constant"; break;
                case id_k: kind_union_str = "id"; break;
                case type_k: kind_union_str = "type"; break;
                case arr_k: kind_union_str = "array"; break;
                case ativ_k: kind_union_str = "activation"; break;
                case assign_k: kind_union_str = "assign"; break;
                case parametro_exp_t: kind_union_str = "parameter_expression"; break;
                default: kind_union_str = "unknown_expression"; break;
            }
            break;
        case declaration_k:
            kind_node = "declaration";
            switch(node->kind_union.decl) {
                case var_k: kind_union_str = "variable"; break;
                case fun_k: kind_union_str = "function"; break;
                case param_k: kind_union_str = "parameter"; break;
                case array_k: kind_union_str = "array"; break;
                case unknown_k: kind_union_str = "unknown"; break;
                default: kind_union_str = "unknown_declaration"; break;
            }
            break;
        default:
            kind_node = "unknown";
            kind_union_str = "unknown";
            break;
    }

    fprintf(file, "Linha: %d, Lexema: %s, Tipo: %s, Tipo_Union: %s", 
            node->linha, node->lexmema, kind_node, kind_union_str);
            
    if (node->pai != NULL){
        fprintf(file, ", Pai: %s", node->pai->lexmema);
    }
    if (node->prev_irmao != NULL){
        fprintf(file, ", Irmao Anterior: %s", node->prev_irmao->lexmema);
    }
    fprintf(file, "\n");
}

void print_tree(FILE *file, No *tree, int depth, int is_irmao){
    if (tree == NULL){return;}
    for (int i = 0; i < depth; i++) {
        fprintf(file, "  ");
    }
    if (is_irmao){
        fprintf(file, "-");
    }
    print_node(file, tree);
    for (int i = 0; i < NUMMAXFILHOS; i++) {
        print_tree(file, tree->filho[i], depth + 1, 0);
    }
    print_tree(file, tree->irmao, depth, 1);
}
