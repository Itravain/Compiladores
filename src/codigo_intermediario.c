#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "../globals.h"
#define NUMMAXFILHOS 3
#define MAXLEXEME 25
#define MAX_TEMP 24

const char *operacoes_nomes[] = {
    "FUN", "ARG", "LOAD", "EQUAL", "GREATER", "LESS", "IFF", "RET", "GOTO", "LAB",
    "PARAM", "DIV", "MUL", "SUB", "CALL", "END", "STORE", "HALT", "SUM", "ALLOC", "ASSIGN"
};
const int NUM_OPERACOES = sizeof(operacoes_nomes) / sizeof(operacoes_nomes[0]);

char escopo[MAXLEXEME+1] = "global\0"; 

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
        fprintf(stderr, "Erro [criarNoTac]: Falha na alocação de memória para novo nó TAC (operação: %s). Possível falta de memória.\n", 
                operacoes_nomes[operacao]);
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
    novoNo->anterior = NULL;

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
        novoNo->anterior = estrutura_tac->fim;
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
        fprintf(stderr, "Erro [imprimirTac]: Arquivo de saída inválido. Verifique se o arquivo foi aberto corretamente.\n");
        return;
    }

    if (tac == NULL)
    {
        fprintf(stderr, "Erro [imprimirTac]: Estrutura TAC nula. A geração de código intermediário falhou anteriormente.\n");
        fprintf(arqCodInterm, "; Lista TAC: Não gerada (estrutura NULL)\n");
        return;
    }
    else if(tac->qtdNos == 0){
        fprintf(arqCodInterm, "; Lista TAC: Vazia\n");
        printf("imprimirTac: TAC structure is empty (qtdNos = 0).\n");
    }
    else{
        fprintf(arqCodInterm, "; Lista TAC Gerada (%d instruções)\n", tac->qtdNos);
        printf("imprimirTac: Printing %d TAC instructions to file.\n", tac->qtdNos);

        TacNo *percorre = tac->inicio;
        int contador = 1;

        while(percorre != NULL){
            const char *op_nome = "UNKNOWN"; 

            if (percorre->operacao >= 0 && percorre->operacao < NUM_OPERACOES) {
                op_nome = operacoes_nomes[percorre->operacao];
            } else {
                fprintf(stderr, "Erro [imprimirTac]: Operação TAC desconhecida (%d) na instrução %d. A tabela de operações pode estar incorreta.\n", 
                        percorre->operacao, contador);
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
        fprintf(arqCodInterm, "; Fim da Lista TAC\n");
    }
    fflush(arqCodInterm);
}


char* gerar_temporario() {
    char* temp_name = malloc(12);
    static int temp_count = 1;
    if (temp_name) {
        sprintf(temp_name, "t%d", temp_count++);
    }
    if(temp_count >= MAX_TEMP){
        temp_count = 1;
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

char *percorrer_arvore(No *node_tree, Tac **tac_list_ptr, int expression_parametro, int esq_assign) {
    
    if (node_tree == NULL) return NULL;
    char *result_str = NULL;

    switch (node_tree->kind_node) {
        case statement_k: {
            char *res_child = NULL;
            switch (node_tree->kind_union.stmt) {
                case if_k: {
                    char *label_else = gerar_label(); 
                    char *label_end = gerar_label();  

                    //aviso de erro caso falhe a geração de rótulos
                    if (!label_else || !label_end) { 
                        fprintf(stderr, "Erro [if_k]: Falha na geração de rótulos para o comando IF. Possível falta de memória.\n");
                        free(label_else); 
                        free(label_end);  
                        return NULL; 
                    }

                    char *cond_res = percorrer_arvore(node_tree->filho[0], tac_list_ptr, 0, 0);
                    //aviso de erro caso falhe a criação do nó TAC
                    if (cond_res) {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, IFF, cond_res, label_else, "");
                        free(cond_res);
                    } else {
                        fprintf(stderr, "Erro [if_k]: A condição do IF na linha %d não produziu um resultado válido. Verifique a expressão.\n", node_tree->linha);
                    }

                    res_child = percorrer_arvore(node_tree->filho[1], tac_list_ptr, 0, 0);
                    free(res_child); 

                    *tac_list_ptr = criarNoTac(*tac_list_ptr, GOTO, label_end, "", "");

                    *tac_list_ptr = criarNoTac(*tac_list_ptr, LAB, label_else, "", "");
                    
                    if (node_tree->filho[2]) {
                        res_child = percorrer_arvore(node_tree->filho[2], tac_list_ptr, 0, 0);
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
                    //erro caso falhe a geração de rótulos
                    if (!label_start || !label_end) { 
                        fprintf(stderr, "Erro [while_k]: Falha na geração de rótulos para o comando WHILE. Possível falta de memória.\n");
                        return NULL; 
                    }

                    *tac_list_ptr = criarNoTac(*tac_list_ptr, LAB, label_start, "", "");

                    char *cond_res = percorrer_arvore(node_tree->filho[0], tac_list_ptr,0, 0);
                    if (cond_res) {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, IFF, cond_res, label_end, "");
                        free(cond_res);
                    } else {
                        fprintf(stderr, "Erro [while_k]: A condição do WHILE na linha %d não produziu um resultado válido. Verifique a expressão.\n", node_tree->linha);
                    }

                    res_child = percorrer_arvore(node_tree->filho[1], tac_list_ptr,0, 0);
                    free(res_child);
                    *tac_list_ptr = criarNoTac(*tac_list_ptr, GOTO, label_start, "", "");

                    *tac_list_ptr = criarNoTac(*tac_list_ptr, LAB, label_end, "", "");
                    free(label_start);
                    free(label_end);
                    result_str = NULL;
                    break;
                }
                case return_k:
                    if (node_tree->filho[0]) {
                        char *ret_val = percorrer_arvore(node_tree->filho[0], tac_list_ptr,0, 0);
                        if (ret_val) {
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, RET, ret_val, "", "");
                            free(ret_val);
                        } else {
                            fprintf(stderr, "Erro [return_k]: A expressão de retorno na linha %d não produziu um resultado válido. Verifique a expressão.\n", node_tree->linha);
                        }
                    } else {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, RET, "", "", "");
                    }
                    result_str = NULL;
                    break;
                default:
                    res_child = percorrer_arvore(node_tree->filho[0], tac_list_ptr,0, 0);
                    free(res_child);
                    result_str = NULL;
                    break;
            }
            break;
        }

        case expression_k: {
            switch (node_tree->kind_union.expr) {
                case op_k: {
                    char *res1 = percorrer_arvore(node_tree->filho[0], tac_list_ptr,0, 0);
                    char *res2 = percorrer_arvore(node_tree->filho[1], tac_list_ptr,0, 0);
                    
                    if (res1 && res2) {
                        result_str = gerar_temporario();
                        if (!result_str) { 
                            fprintf(stderr, "Erro [op_k]: Falha ao gerar variável temporária para operação '%s'. Possível falta de memória.\n", node_tree->lexmema);
                            free(res1); 
                            free(res2); 
                            return NULL; 
                        }

                        enum operacoes op;
                        if (strcmp(node_tree->lexmema, "+") == 0) op = SUM;
                        else if (strcmp(node_tree->lexmema, "-") == 0) op = SUB;
                        else if (strcmp(node_tree->lexmema, "*") == 0) op = MUL;
                        else if (strcmp(node_tree->lexmema, "/") == 0) op = DIV;
                        else if (strcmp(node_tree->lexmema, "==") == 0) op = EQUAL;
                        else if (strcmp(node_tree->lexmema, ">") == 0) op = GREATER;
                        else if (strcmp(node_tree->lexmema, "<") == 0) op = LESS;
                        else {
                            fprintf(stderr, "Erro: Operador não reconhecido '%s'\n", node_tree->lexmema);
                            op = -1;
                        }

                        if (op != -1) {
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, op, result_str, res1, res2);
                        }
                        if (expression_parametro){
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, PARAM, result_str, "", "");
                        }
                    } else {
                        fprintf(stderr, "Erro [op_k]: Os operandos para operação '%s' na linha %d não produziram resultados válidos. Verifique a expressão.\n", 
                                node_tree->lexmema, node_tree->linha);
                        result_str = NULL;
                    }
                    free(res1);
                    free(res2);
                    break;
                }
                // Ambos possuem a mesma lógica, por isso estão juntos.
                case constant_k:
                case id_k: {
                    char *tmp = gerar_temporario();
                    if (tmp != NULL) {
                        if (!esq_assign) {
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, LOAD, tmp, node_tree->lexmema, "");
                        }
                        
                        if (expression_parametro){
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, PARAM, tmp, "", "");
                        }
                        
                        result_str = strdup(tmp);  
                        free(tmp); 
                    } else {
                        fprintf(stderr, "Erro [id_k/constant_k]: Falha ao gerar variável temporária para '%s' na linha %d. Possível falta de memória.\n", 
                                node_tree->lexmema, node_tree->linha);
                        result_str = NULL;
                    }
                    break;
                }
                case assign_k: {
                    char *lhs_name = percorrer_arvore(node_tree->filho[0], tac_list_ptr, 0, 1);
                    char *rhs_res = percorrer_arvore(node_tree->filho[1], tac_list_ptr, 0, 0);

                    if (rhs_res && lhs_name) {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, ASSIGN, lhs_name, rhs_res, "");
                        if (node_tree->filho[0]->kind_union.expr == arr_k) {
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, STORE, node_tree->filho[0]->lexmema, lhs_name, node_tree->filho[0]->filho[0]->lexmema);
                        }
                        else {
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, STORE, node_tree->filho[0]->lexmema, lhs_name, "");
                        }
                        
                        result_str = strdup(lhs_name);
                        
                        
                    } else {
                        fprintf(stderr, "Erro [assign_k]: Atribuição inválida na linha %d. Lado esquerdo ou direito produziu resultado nulo.\n", 
                                node_tree->linha);
                        result_str = NULL;
                    }
                    free(rhs_res);
                    break;
                }
                case ativ_k:{
                    int num_params = 0;
                    No* param_node = node_tree->filho[0];
                    
                    char *tmp = percorrer_arvore(node_tree->filho[0], tac_list_ptr, 1, 0);
                    if (expression_parametro){
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, PARAM, tmp, "", "");
                        }
            
                    while(param_node != NULL) {
                        num_params++;
                        param_node = param_node->irmao;
                    }
                    
                    char params_str[10];
                    sprintf(params_str, "%d", num_params);
                    

                    tmp = gerar_temporario();
                    *tac_list_ptr = criarNoTac(*tac_list_ptr, CALL, tmp, node_tree->lexmema, params_str);
                    
                    if(node_tree->irmao == NULL){
                        return tmp;
                    }
                    free(tmp);
                    result_str = NULL;
                    break;
                }
                case arr_k: {
                    char *index_res = percorrer_arvore(node_tree->filho[0], tac_list_ptr, 0, 0);
                    
                    if (index_res) {
                        result_str = gerar_temporario();
                        if (!result_str) { 
                            fprintf(stderr, "Erro [array_k]: Falha ao gerar variável temporária para o array '%s'. Possível falta de memória.\n", node_tree->lexmema);
                            free(index_res); 
                            return NULL; 
                        }
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, LOAD, result_str, node_tree->lexmema, index_res);
                        if (expression_parametro){
                            *tac_list_ptr = criarNoTac(*tac_list_ptr, PARAM, result_str, "", "");
                        }
                        free(index_res);
                    } else {
                        fprintf(stderr, "Erro [array_k]: O índice do array na linha %d não produziu um resultado válido. Verifique a expressão.\n", node_tree->linha);
                        result_str = NULL;
                    }
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
                        int contador_filhos = 0;
                        No *auxiliar = node_tree->filho[0];
                        while (auxiliar != NULL)
                        {
                            contador_filhos++;
                            auxiliar = auxiliar->irmao;
                        }
                        //transformar contador_filhos em string
                        char contador_filhos_str[10];
                        sprintf(contador_filhos_str, "%d", contador_filhos);
                        
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, FUN, node_tree->pai->lexmema, node_tree->lexmema, contador_filhos_str);
                        strncpy(escopo, node_tree->lexmema, MAXLEXEME);
                    }
                    for (int i = 0; i < NUMMAXFILHOS; i++) {
                        if (node_tree->filho[i]) {
                            res_child = percorrer_arvore(node_tree->filho[i], tac_list_ptr, 0, 0);
                            free(res_child);
                        }
                    }
                    if(strcmp(node_tree->lexmema, "int") !=0 && strcmp(node_tree->lexmema, "void") != 0){
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, END, node_tree->lexmema, "", "");
                    }
                    
                    result_str = NULL;

                    break;
                case param_k:              
                    *tac_list_ptr = criarNoTac(*tac_list_ptr, ARG, node_tree->pai->lexmema, node_tree->lexmema, escopo);                   
                    result_str = NULL;
                    break;

                case var_k:
                    res_child = percorrer_arvore(node_tree->filho[0], tac_list_ptr, 0, 0);
                    if (strcmp(node_tree->lexmema, "int") !=0 && strcmp(node_tree->lexmema, "void") != 0)
                    {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, ALLOC, node_tree->lexmema, escopo, "");
                        result_str = NULL;
                    }
                    
                    break;
                
                case array_k:
                if (strcmp(node_tree->lexmema, "int") != 0 && strcmp(node_tree->lexmema, "void") != 0) {
                    if (node_tree->filho[0] != NULL) {
                        *tac_list_ptr = criarNoTac(*tac_list_ptr, ALLOC, node_tree->lexmema, escopo, node_tree->filho[0]->lexmema);
                        result_str = NULL;
                    } else {
                        fprintf(stderr, "Erro [array_k]: Nó do tamanho do array (filho[0]) é NULL para o array '%s' na linha %d.\n", node_tree->lexmema, node_tree->linha);
                        result_str = NULL;
                    }
                }
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

    if(node_tree->irmao) {
        char *sibling_res = percorrer_arvore(node_tree->irmao, tac_list_ptr, expression_parametro, 0);
        free(sibling_res);
    }

    return result_str;
}
