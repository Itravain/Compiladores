#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../globals.h"
#define NUMMAXFILHOS 3
#define MAXLEXEME 25

const char *operacoes_nomes[] = {
    "FUN", "ARG", "LOAD", "EQUAL", "IFF", "RET", "GOTO", "LAB",
    "PARAM", "DIV", "MUL", "SUB", "CALL", "END", "STORE", "HALT", "SUM"
};
const int NUM_OPERACOES = sizeof(operacoes_nomes) / sizeof(operacoes_nomes[0]);

Tac *criarTac(Tac *estrutura_tac){
    estrutura_tac = malloc(sizeof(Tac));
    estrutura_tac->qtdNos = 0;
    estrutura_tac->fim = NULL;
    estrutura_tac->inicio = NULL;
    return estrutura_tac;
}

Tac *criarNoTac(Tac *estrutura_tac, int operacao,
                const char *op1,
                const char *op2,
                const char *resultado) {
    TacNo* novoNo = malloc(sizeof(TacNo));
    if (novoNo == NULL) {
        perror("Failed to allocate TacNo");
        return estrutura_tac;
    }
    //Preencher informações
    novoNo->operacao = operacao;

    strncpy(novoNo->op1, op1, sizeof(novoNo->op1) - 1);
    novoNo->op1[sizeof(novoNo->op1) - 1] = '\0';

    strncpy(novoNo->op2, op2, sizeof(novoNo->op2) - 1);
    novoNo->op2[sizeof(novoNo->op2) - 1] = '\0';

    strncpy(novoNo->resultado, resultado, sizeof(novoNo->resultado) - 1);
    novoNo->resultado[sizeof(novoNo->resultado) - 1] = '\0';

    novoNo->proximo = NULL;

    if (estrutura_tac == NULL) {
        estrutura_tac = criarTac(NULL);
        if (estrutura_tac == NULL) {
             free(novoNo);
             return NULL;
        }
    }

    if (estrutura_tac->fim == NULL)
    {
        estrutura_tac->inicio = novoNo;
        estrutura_tac->fim = novoNo;
    }
    else{
        estrutura_tac->fim->proximo = novoNo;
        estrutura_tac->fim = novoNo;
    }
    estrutura_tac->qtdNos++;
    return estrutura_tac;
}

Tac *liberarTac(Tac *estrutura_tac){
    if (estrutura_tac == NULL)
    {
        return NULL;
    }
    else if(estrutura_tac->qtdNos != 0){
        TacNo *atual = estrutura_tac->inicio;
        TacNo *proximo = NULL;
        while (atual->proximo != NULL)
        {
            proximo = atual->proximo;
            free(atual);
            atual = proximo;
        }
        free(atual);       
    }
    free(estrutura_tac);
    return NULL;    
}

void imprimirTac(FILE *arqCodInterm, Tac *tac){
    if (arqCodInterm == NULL) {
        fprintf(stderr, "Error: Invalid file pointer passed to imprimirTac.\n");
        return;
    }

    if (tac == NULL)
    {
        fprintf(arqCodInterm, "; TAC List: Not generated (NULL structure)\n");
        printf("imprimirTac: TAC structure is NULL.\n");
        return;
    }
    else if(tac->qtdNos == 0){
        fprintf(arqCodInterm, "; TAC List: Empty\n");
        printf("imprimirTac: TAC structure is empty (qtdNos = 0).\n");
    }
    else{
        fprintf(arqCodInterm, "; TAC List Generated (%d instructions)\n", tac->qtdNos);
        printf("imprimirTac: Printing %d TAC instructions to file.\n", tac->qtdNos);

        TacNo *percorre = tac->inicio;
        int contador = 1;

        while(percorre != NULL){
            const char *op_nome = "UNKNOWN"; 

            if (percorre->operacao >= 0 && percorre->operacao < NUM_OPERACOES) {
                op_nome = operacoes_nomes[percorre->operacao];
            } else {
                fprintf(stderr, "Warning: Unknown TAC operation enum value %d at instruction %d\n", percorre->operacao, contador);
            }

            fprintf(arqCodInterm, "(%d) (%s, %s, %s, %s)\n",
                    contador,
                    op_nome,
                    percorre->op1,
                    percorre->op2,
                    percorre->resultado);

            percorre = percorre->proximo;
            contador++;
        }
        fprintf(arqCodInterm, "; End of TAC List\n");
    }
    fflush(arqCodInterm);
}

char* gerar_temporario() {
    static int temp_count = 0;
    char* temp_name = malloc(12);
    if (temp_name) {
        sprintf(temp_name, "t%d", temp_count++);
    }
    return temp_name;
}

char* gerar_label() {
    static int label_count = 0;
    char* label_name = malloc(12);
    if (label_name) {
        sprintf(label_name, "L%d", label_count++);
    }
    return label_name;
}

char *percorrer_arvore(No *node_tree, Tac **tac_list_ptr) {
    if (node_tree == NULL) return NULL;

    char *result_str = NULL;

    switch (node_tree->kind_node) {
        case statement_k: {
            char *res_child = NULL;
            switch (node_tree->kind_union.stmt) {
                case if_k: {
                    char *label_else = gerar_label();
                    char *label_end = gerar_label();
                    if (!label_else || !label_end) { return NULL; }

                    char *cond_res = percorrer_arvore(node_tree->filho[0], tac_list_ptr);
                    if (cond_res) {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, IFF, cond_res, "", label_else);
                        free(cond_res);
                    } else {
                        fprintf(stderr, "Error: If condition did not produce a result.\n");
                    }

                    res_child = percorrer_arvore(node_tree->filho[1], tac_list_ptr);
                    free(res_child);
                    *tac_list_ptr = criarNoTac(*tac_list_ptr, GOTO, "", "", label_end);

                    *tac_list_ptr = criarNoTac(*tac_list_ptr, LAB, label_else, "", "");
                    if (node_tree->filho[2]) {
                        res_child = percorrer_arvore(node_tree->filho[2], tac_list_ptr);
                        free(res_child);
                    }

                    *tac_list_ptr = criarNoTac(*tac_list_ptr, LAB, label_end, "", "");
                    free(label_else);
                    free(label_end);
                    result_str = NULL;
                    break;
                }
                case while_k: {
                    char *label_start = gerar_label();
                    char *label_end = gerar_label();
                    if (!label_start || !label_end) { return NULL; }

                    *tac_list_ptr = criarNoTac(*tac_list_ptr, LAB, label_start, "", "");

                    char *cond_res = percorrer_arvore(node_tree->filho[0], tac_list_ptr);
                    if (cond_res) {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, IFF, cond_res, "", label_end);
                        free(cond_res);
                    } else {
                        fprintf(stderr, "Error: While condition did not produce a result.\n");
                    }

                    res_child = percorrer_arvore(node_tree->filho[1], tac_list_ptr);
                    free(res_child);
                    *tac_list_ptr = criarNoTac(*tac_list_ptr, GOTO, "", "", label_start);

                    *tac_list_ptr = criarNoTac(*tac_list_ptr, LAB, label_end, "", "");
                    free(label_start);
                    free(label_end);
                    result_str = NULL;
                    break;
                }
                case return_k:
                    if (node_tree->filho[0]) {
                        char *ret_val = percorrer_arvore(node_tree->filho[0], tac_list_ptr);
                        if (ret_val) {
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, RET, ret_val, "", "");
                            free(ret_val);
                        } else {
                            fprintf(stderr, "Error: Return expression did not produce a result.\n");
                        }
                    } else {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, RET, "", "", "");
                    }
                    result_str = NULL;
                    break;
                default:
                    res_child = percorrer_arvore(node_tree->filho[0], tac_list_ptr);
                    free(res_child);
                    result_str = NULL;
                    break;
            }
            break;
        }

        case expression_k: {
            switch (node_tree->kind_union.expr) {
                case op_k: {
                    char *res1 = percorrer_arvore(node_tree->filho[0], tac_list_ptr);
                    char *res2 = percorrer_arvore(node_tree->filho[1], tac_list_ptr);

                    if (res1 && res2) {
                        result_str = gerar_temporario();
                        if (!result_str) { free(res1); free(res2); return NULL; }

                        enum operacoes op;
                        if (strcmp(node_tree->lexmema, "+") == 0) op = SUM;
                        else if (strcmp(node_tree->lexmema, "-") == 0) op = SUB;
                        else if (strcmp(node_tree->lexmema, "*") == 0) op = MUL;
                        else if (strcmp(node_tree->lexmema, "/") == 0) op = DIV;
                        else {
                            fprintf(stderr, "Error: Unrecognized operator '%s'\n", node_tree->lexmema);
                            op = -1;
                        }

                        if (op != -1) {
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, op, res1, res2, result_str);
                        }
                    } else {
                        fprintf(stderr, "Error: Operands for '%s' did not produce results.\n", node_tree->lexmema);
                        result_str = NULL;
                    }
                    free(res1);
                    free(res2);
                    break;
                }
                case constant_k:
                case id_k:
                    result_str = strdup(node_tree->lexmema);
                    break;

                case assign_k: {
                    char *rhs_res = percorrer_arvore(node_tree->filho[1], tac_list_ptr);
                    char *lhs_name = NULL;

                    if (node_tree->filho[0]->kind_node == expression_k && node_tree->filho[0]->kind_union.expr == id_k) {
                        lhs_name = node_tree->filho[0]->lexmema;
                    }

                    if (rhs_res && lhs_name) {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, EQUAL, rhs_res, "", lhs_name);
                        result_str = strdup(lhs_name);
                    } else {
                        fprintf(stderr, "Error: Invalid assignment structure or RHS failed.\n");
                        result_str = NULL;
                    }
                    free(rhs_res);
                    break;
                }
                default:
                    result_str = NULL;
                    break;
            }
            break;
        }

        case declaration_k: {
            char *res_child = NULL;
            switch (node_tree->kind_union.decl) {
                case fun_k:
                    if(strcmp(node_tree->lexmema, "int") !=0 && strcmp(node_tree->lexmema, "void") != 0){
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, FUN, node_tree->pai->lexmema, node_tree->lexmema, "");
                    }
                    for (int i = 0; i < NUMMAXFILHOS; i++) {
                        if (node_tree->filho[i]) {
                            res_child = percorrer_arvore(node_tree->filho[i], tac_list_ptr);
                            free(res_child);
                        }
                    }
                    if(strcmp(node_tree->lexmema, "int") !=0 && strcmp(node_tree->lexmema, "void") != 0){
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, END, node_tree->lexmema, "", "");
                    }
                    result_str = NULL;

                    break;
                case param_k:
                    *tac_list_ptr = criarNoTac(*tac_list_ptr, PARAM, node_tree->lexmema, "", "");
                    
                    result_str = NULL;
                    break;

                case var_k:
                    result_str = NULL;
                    break;

                default:
                    result_str = NULL;
                    break;
            }
            break;
        }

        default:
            result_str = NULL;
            break;
    }

    if (node_tree->irmao) {
        char *sibling_res = percorrer_arvore(node_tree->irmao, tac_list_ptr);
        free(sibling_res);
    }

    return result_str;
}
