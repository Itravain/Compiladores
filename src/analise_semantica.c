#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../globals.h"

// Keep track of whether main was declared
int main_declared = 0;
char scope[MAXLEXEME + 1] = "GLOBAL";

void semantic_analysis(No* root, HashTable* symbol_table) {
    if (!root) return;

    char current_scope[MAXLEXEME + 1];
    strcpy(current_scope, scope); // Salva o escopo atual

    // Rule 6: Ensure main function exists and is valid
    if (root->kind_node == declaration_k && root->kind_union.decl == fun_k) {
        if (strcmp(root->lexmema, "main") == 0) {
            main_declared = 1;
        }
        // Atualiza o escopo global para o nome da função atual
        strcpy(scope, root->lexmema);
    }

    // Rule 1: Assign to undeclared variable
    if (root->kind_node == expression_k && root->kind_union.expr == assign_k) {
        char* var_name = root->filho[0]->lexmema;
        // Procura no escopo da função atual E no escopo global
        Symbol* var_local = find_symbol(symbol_table, var_name, scope);
        Symbol* var_global = find_symbol(symbol_table, var_name, "GLOBAL");

        if (!var_local && !var_global) {
            fprintf(stderr, "Error: Variable '%s' assigned before declaration at line %d.\n", var_name, root->linha);
        }
    }
    
    // Rule 2: Invalid assign type (assigning void function result)
    if (root->kind_node == expression_k && strcmp(root->lexmema, "=") == 0  && root->filho[1]->kind_union.expr == ativ_k) {
        Symbol* func = find_symbol(symbol_table, root->filho[1]->lexmema, "GLOBAL");
        if (func && strcmp(func->type, "void") == 0) {
            fprintf(stderr, "Semantic Error: Cannot assign return value of void function '%s' at line %d.\n", root->filho[1]->lexmema, root->linha);
        }
    }

    // Recurse for children
    for (int i = 0; i < NUMMAXFILHOS; ++i) {
        if (root->filho[i] != NULL) {
            semantic_analysis(root->filho[i], symbol_table);
        }
    }
    if (root->irmao != NULL) {
        semantic_analysis(root->irmao, symbol_table);
    }

    strcpy(scope, current_scope); // Restaura o escopo anterior ao sair do nó
}

void check_main_function() {
    if (!main_declared) {
        fprintf(stderr, "Semantic Error: No declaration of 'main' function.\n");
    }
}
