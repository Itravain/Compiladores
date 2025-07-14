#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../globals.h"

Escopo escopo_atual;

//Função para mapear registradores do código intermediário para registradores físicos
char* get_reg(const char *temp_name) {
    char* reg_name = (char*) malloc(4 * sizeof(char));
    if (!temp_name || temp_name[0] != 't') {
        // Se não for um temporário, retorna uma string vazia para evitar falhas
        strcpy(reg_name, "");
        return reg_name;
    }
    
    int temp_num;
    sscanf(temp_name, "t%d", &temp_num);
    sprintf(reg_name, "R%d", temp_num);
    return reg_name;
}
int primeira_funcao = 0;

//Verifica se uma string contém apenas dígitos.
int is_numeric(const char *str) {
    if (str == NULL || *str == '\0') {
        return 0; // String nula ou vazia não é numérica.
    }
    while (*str) {
        if (!isdigit(*str)) {
            return 0; // Encontrou um caractere que não é dígito.
        }
        str++;
    }
    return 1; // Todos os caracteres são dígitos.
}


void traduzir_tac_para_assembly(FILE *arquivoSaida, TacNo *tac, HashTable *tabela_simbolos) {
    const char *op_nomes[] = {
        "FUN", "ARG", "LOAD", "EQUAL", "GREATER", "LESS", "IFF", "RET", "GOTO", "LAB",
        "PARAM", "DIV", "MUL", "SUB", "CALL", "END", "STORE", "HALT", "SUM", "ALLOC", "ASSIGN"
    };
    
    char *reg_res = get_reg(tac->resultado);
    char *reg_op1 = get_reg(tac->op1);
    char *reg_op2 = get_reg(tac->op2);

    fprintf(arquivoSaida, ";   TAC: (%s, %s, %s, %s)\n", op_nomes[tac->operacao], tac->resultado, tac->op1, tac->op2);

    switch (tac->operacao) {
        case SUM:
            // Ex: (SUM, t2, t0, t1) -> ADD R2, R0, R1
            fprintf(arquivoSaida, "    ADD %s, %s, %s\n", reg_op1, reg_res, reg_op2);
            break;
        case SUB:
            // Ex: (SUB, t2, t0, t1) -> SUB R2, R0, R1
            fprintf(arquivoSaida, "    SUB %s, %s, %s\n",  reg_op1, reg_op2, reg_res);
            break;
        case MUL:
            // Ex: (MUL, t1, t0, t2) -> MUL R1, R0, R2
            fprintf(arquivoSaida, "    MUL %s, %s, %s\n",  reg_op1,  reg_op2, reg_res);
            break;
        case DIV:
            // Ex: (DIV, t1, t0, t2) -> UDIV R1, R0, R2
            fprintf(arquivoSaida, "    UDIV %s, %s, %s\n",  reg_op1, reg_op2, reg_res);
            break;
        case LAB:
            // Ex: (LAB, L1, , ) -> L1:
            fprintf(arquivoSaida, "%s:\n", tac->op1);
            break;
        case GOTO:
            // Ex: (GOTO, L1, , ) -> BI #L1
            fprintf(arquivoSaida, "    B %s\n", tac->op1); //
            break;
        case HALT:
            // (HALT, , , ) -> FINISH
            fprintf(arquivoSaida, "    FINISH\n"); //
            break;
        case LOAD: {
            // Verifica se a variável (tac->op1) é global
            Symbol* simbolo = find_symbol(tabela_simbolos, tac->op2, "GLOBAL");
            if (simbolo != NULL) { // É global
                if (simbolo->id_type == var_k) {
                    fprintf(arquivoSaida, "    ; Acessando variável global '%s'\n", tac->op2);
                    fprintf(arquivoSaida, "    LDR %s, [R0, #%d]\n", reg_op1, simbolo->offset);
                }
                else if (simbolo->id_type == array_k) {
                    if(strcmp(reg_res, "") == 0) {
                        fprintf(arquivoSaida, "    ; Acessando endereço de array global '%s'\n", tac->op2);
                        //Acessando enderço base
                        fprintf(arquivoSaida, "    MOVI Rad, #%d\n", simbolo->offset);
                        fprintf(arquivoSaida, "    MOV %s, Rad\n", reg_op1);
                    }
                    else {
                        fprintf(arquivoSaida, "    ; Acessando array global '%s'\n", tac->op2);
                        // 1. Carrega o endereço base do array global em Rad
                        fprintf(arquivoSaida, "    MOVI Rad, #%d\n", simbolo->offset);
                        // 2. Adiciona o deslocamento (índice, que está em reg_res) ao endereço base
                        fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_res);
                        // 3. Carrega o valor do endereço final no registrador de destino (reg_op1)
                        fprintf(arquivoSaida, "    LDR %s, [Rad, #0]\n", reg_op1);
                    }
                }
            } else { // Não é global, procura no escopo local
                if ( (atoi(tac->op2) != 0) || (strcmp(tac->op2, "0") == 0) ) {
                    fprintf(arquivoSaida, "    MOVI %s, #%s\n", reg_op1, tac->op2);
                }
                else {
                    simbolo = find_symbol(tabela_simbolos, tac->op2, escopo_atual.escopo);
                    if (simbolo != NULL) {
                        if (simbolo->id_type == var_k){
                            fprintf(arquivoSaida, "    ; Acessando variavel local '%s'\n", tac->op2);
                            fprintf(arquivoSaida, "    LDR %s, [FP, #%d]\n", reg_op1, simbolo->offset);
                        }   
                        else if (simbolo->id_type == array_k){
                            //Acessando endereço do array
                            if(strcmp(reg_res, "") == 0) {
                                fprintf(arquivoSaida, "    ; Acessando endereço de array local '%s'\n", tac->op2);
                                fprintf(arquivoSaida, "    MOV Rad, FP\n");
                                fprintf(arquivoSaida, "    ADDI Rad, Rad, #%d\n", simbolo->offset);
                                fprintf(arquivoSaida, "    MOV %s, Rad\n", reg_op1);

                            }
                            else{
                                fprintf(arquivoSaida, "    ; Acessando array local '%s'\n", tac->op2);
                                // 1. Carrega o endereço base do array local em Rad
                                fprintf(arquivoSaida, "    MOV Rad, FP\n");
                                fprintf(arquivoSaida, "    ADDI Rad, Rad, #%d\n", simbolo->offset);
                                // 2. Adiciona o deslocamento (índice, que está em reg_res) ao endereço base
                                fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_res);
                                // 3. Carrega o valor do endereço final no registrador de destino (reg_op1)
                                fprintf(arquivoSaida, "    LDR %s, [Rad, #0]\n", reg_op1);
                            }


                        }
                        else if (simbolo->id_type == param_k){
                            fprintf(arquivoSaida, "    ; Acessando param'%s'\n", tac->op2);
                            fprintf(arquivoSaida, "    MOV Rad, FP\n", reg_op1, simbolo->offset);
                            fprintf(arquivoSaida, "    SUBI Rad, Rad, #%d\n", atoi(escopo_atual.qtd_param) - simbolo->offset);
                            fprintf(arquivoSaida, "    LDR %s, [Rad, #0]\n", reg_op1);
                        }
                        else if (simbolo->id_type == param_array_k){
                            if(strcmp(reg_res, "") == 0) {
                                fprintf(arquivoSaida, "    ; Acessando endereço de param array: '%s'\n", tac->op2);
                                fprintf(arquivoSaida, "    MOV Rad, FP\n");
                                fprintf(arquivoSaida, "    SUBI Rad, Rad, #%d\n", atoi(escopo_atual.qtd_param) - simbolo->offset);
                                fprintf(arquivoSaida, "    LDR %s, [Rad, #0]\n", reg_op1);
                            }
                            else {
                                fprintf(arquivoSaida, "    ; Acessando param array: '%s'\n", tac->op2);
                                // 1. Carrega o endereço do ponteiro para Rad
                                fprintf(arquivoSaida, "    MOV Rad, FP\n");
                                fprintf(arquivoSaida, "    SUBI Rad, Rad, #%d\n", atoi(escopo_atual.qtd_param) - simbolo->offset);
                                // 2. Carrega o valor do ponteiro (o endereço base do array original) para Rad
                                fprintf(arquivoSaida, "    LDR Rad, [Rad, #0]\n");
                                // 3. Adiciona o deslocamento (índice, que está em reg_res) ao endereço base
                                fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_res);
                                // 4. Carrega o valor do endereço final no registrador de destino (reg_op1)
                                fprintf(arquivoSaida, "    LDR %s, [Rad, #0]\n", reg_op1);
                            }
                        }
                    }
                    else {
                        fprintf(stderr, "AVISO: Símbolo '%s' não encontrado no escopo '%s' durante a geração de código.\n", tac->op2, escopo_atual);
                    }
                }
            }
            break;
        }

        case STORE: {
            Symbol* simbolo = find_symbol(tabela_simbolos, tac->op1, "GLOBAL");
            if (simbolo != NULL) { // É global
                if (simbolo->id_type == var_k){
                    fprintf(arquivoSaida, "    ; Armazenando em variável global '%s'\n", tac->op1);
                    fprintf(arquivoSaida, "    STR %s, [R0, #%d]\n", reg_res, simbolo->offset);
                }   
                else if (simbolo->id_type == array_k){
                    fprintf(arquivoSaida, "    ;Armazenando em array global '%s'\n", tac->op1);
                    // 1. Carrega o endereço base do array global em Rad
                    fprintf(arquivoSaida, "    MOVI Rad, #%d\n", simbolo->offset);
                    // 2. Adiciona o deslocamento (índice, que está em reg_op2) ao endereço base
                    fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_op2);
                    // 3. Armazena o valor (que está em reg_res) no endereço final
                    fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_res);
                }
            } else { // Não é global, procura no escopo local
                simbolo = find_symbol(tabela_simbolos, tac->op1, escopo_atual.escopo);
                if (simbolo != NULL) {
                    if (simbolo->id_type == var_k) {
                        fprintf(arquivoSaida, "    ; Armazenando em variavel local '%s'\n", tac->op1);
                        fprintf(arquivoSaida, "    STR %s, [FP, #%d]\n", reg_res, simbolo->offset);
                    } 
                    else if (simbolo->id_type == array_k){
                        fprintf(arquivoSaida, "    ; Armazenando em array local '%s'\n", tac->op1);
                        // 1. Carrega o endereço base do array local em Rad
                        fprintf(arquivoSaida, "    MOV Rad, FP\n");
                        fprintf(arquivoSaida, "    ADDI Rad, Rad, #%d\n", simbolo->offset);
                        
                        // 2. Adiciona o deslocamento (índice, que está em reg_op2) ao endereço base
                        fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_op2);

                        // 3. Armazena o valor (que está em reg_res) no endereço final
                        fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_res);
                    }
                    else if (simbolo->id_type == param_k){
                        fprintf(arquivoSaida, "    ; Armazenando param '%s'\n", tac->op1);
                        fprintf(arquivoSaida, "    MOV Rad, FP\n", reg_op1, simbolo->offset);
                        fprintf(arquivoSaida, "    SUBI Rad, Rad, #%d\n", atoi(escopo_atual.qtd_param) - simbolo->offset);
                        fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_op2);
                    }
                    else if (simbolo->id_type == param_array_k){
                        fprintf(arquivoSaida, "    ; Armazenando em param array '%s'\n", tac->op1);
                        
                        // 1. Carrega o endereço do ponteiro (parâmetro) para Rad.
                        fprintf(arquivoSaida, "    MOV Rad, FP\n");
                        fprintf(arquivoSaida, "    SUBI Rad, Rad, #%d\n", atoi(escopo_atual.qtd_param) - simbolo->offset);
                        
                        // 2. Carrega o valor do ponteiro (o endereço base do array original) para Rad.
                        fprintf(arquivoSaida, "    LDR Rad, [Rad, #0]\n");

                        // 3. Adiciona o deslocamento (índice, que está em reg_op2) ao endereço base.
                        fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_op2);

                        // 4. Armazena o valor (que está em reg_res) no endereço final calculado.
                        fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_res);
                    }
                }
                else {
                    fprintf(stderr, "AVISO: Símbolo '%s' não encontrado no escopo '%s' durante a geração de código.\n", tac->op2, escopo_atual);
                }
            }
            break;
        }
        
        case FUN: {
            if (primeira_funcao == 0) {
                fprintf(arquivoSaida, "    B main\n");
                primeira_funcao = 1;
            }
            
            fprintf(arquivoSaida, ".%s\n", tac->op2);


            if (strcmp(tac->op2, "main") == 0) {
                fprintf(arquivoSaida, "    MOV FP, SP\n");
            }
            else {
                fprintf(arquivoSaida, "    ADDI Rlink, Rlink, #1\n");
                fprintf(arquivoSaida, "    STR Rlink [SP #0]\n");
                fprintf(arquivoSaida, "    ADDI SP, SP, #1\n");
            }
            
            strcpy(escopo_atual.escopo, tac->op2);
            strcpy(escopo_atual.qtd_param, tac->resultado);
            break;
        }
        case END:
            if (strcmp(tac->op1, "main") != 0){
                Symbol* simbolo = find_symbol(tabela_simbolos, tac->op1, "GLOBAL");
                //Retorno apenas em funções void
                if(strcmp(simbolo->type, "void") == 0){
                    fprintf(arquivoSaida,"    LDR Rlink [FP #1]\n", get_reg(tac->op1), get_reg(tac->op2));
                    fprintf(arquivoSaida,"    MOV SP, FP\n", get_reg(tac->op1), get_reg(tac->op2));
                    fprintf(arquivoSaida,"    LDR FP [FP #0]\n", get_reg(tac->op1), get_reg(tac->op2));
                    fprintf(arquivoSaida,"    B Rlink\n", get_reg(tac->op1), get_reg(tac->op2));
                }
            }
            
            break;
        case IFF:{
            break;
        }
        case CALL: {
            if (strcmp(tac->op2, "input") == 0)  {
                fprintf(arquivoSaida, "    IN\n");
                fprintf(arquivoSaida, "    MOV %s, Rin\n", get_reg(tac->op1));

            }
            else if (strcmp(tac->op2, "output") == 0)  {
                fprintf(arquivoSaida, "    MOV Rout, %s\n", get_reg(tac->anterior->op1));
                fprintf(arquivoSaida, "    OUT\n");
            }
            else {
                fprintf(arquivoSaida, "    STR FP [SP, #0]\n");
                fprintf(arquivoSaida, "    MOV FP, SP\n");
                fprintf(arquivoSaida, "    ADDI SP, SP, #1\n");
                fprintf(arquivoSaida, "    BL %s\n", tac->op2);
                fprintf(arquivoSaida, "    MOV %s, Rret\n", reg_op1);
            }
            
            break;
        } 
        case ARG:
            
            break;
        case PARAM:{
            // Em chamadas de output não é necessário usar o param
            if(strcmp(tac->proximo->op2, "output") == 0) {
            }
            else {
                fprintf(arquivoSaida, "    STR %s [SP, #0]\n", get_reg(tac->op1));
                fprintf(arquivoSaida, "    ADDI SP, SP, #1\n");
            }

            break;
        }
        case ALLOC: {
            if (strcmp(tac->resultado, "") == 0) {
                fprintf(arquivoSaida, "    ADDI SP, SP, #1\n");
            }
            else {
                fprintf(arquivoSaida, "    ADDI SP, SP, #%s\n", tac->resultado);

            }
            
            break;
        }
        case ASSIGN: {
            fprintf(arquivoSaida,"    MOV %s, %s\n", get_reg(tac->op1), get_reg(tac->op2));
            break;
        }
        case RET: {
            fprintf(arquivoSaida,"    MOV Rret, %s\n", reg_op1);
            fprintf(arquivoSaida,"    LDR Rlink [FP #1]\n", get_reg(tac->op1), get_reg(tac->op2));
            fprintf(arquivoSaida,"    MOV SP, FP\n", get_reg(tac->op1), get_reg(tac->op2));
            fprintf(arquivoSaida,"    LDR FP [FP #0]\n", get_reg(tac->op1), get_reg(tac->op2));
            fprintf(arquivoSaida,"    B Rlink\n", get_reg(tac->op1), get_reg(tac->op2));
            break;
        }
        case GREATER:{
            fprintf(arquivoSaida, "    CMP %s, %s\n", reg_op2, reg_res);
            fprintf(arquivoSaida, "    BLE %s\n", tac->proximo->op2);
            break;
        }   
        case EQUAL:{
            fprintf(arquivoSaida, "    CMP %s, %s\n", reg_op2, reg_res);
            fprintf(arquivoSaida, "    BNE %s\n", tac->proximo->op2);
            break;
        }
        case LESS:{
            fprintf(arquivoSaida, "    CMP %s, %s\n", reg_op2, reg_res);
            fprintf(arquivoSaida, "    BGE %s\n", tac->proximo->op2);
            break;
        }      
        default:
            fprintf(arquivoSaida, "    ; AVISO: Operação TAC %d não reconhecida.\n", tac->operacao);
            break;
    }

    free(reg_res);
    free(reg_op1);
    free(reg_op2);
}



void gerar_codigo_final(FILE *arquivoSaida, Tac *listaTac, HashTable *tabela_simbolos) {
    strcpy(escopo_atual.qtd_param, "0");
    strcpy(escopo_atual.escopo, "GLOBAL");
    
    if (!arquivoSaida) {
        fprintf(stderr, "Erro: Arquivo de saída para assembly é nulo.\n");
        return;
    }
    if (!listaTac || !listaTac->inicio) {
        fprintf(stderr, "Aviso: Lista de código intermediário está vazia. Nenhum código assembly gerado.\n");
        return;
    }

    // Escreve as diretivas iniciais do assembly.
    fprintf(arquivoSaida, "; Arquivo Assembly gerado pelo compilador C-\n");
    
    fprintf(arquivoSaida, "\n.data\n");
    
    
    // Itera por toda a tabela de símbolos
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol *simbolo = tabela_simbolos->table[i];
        while (simbolo != NULL) {
            // Se o símbolo é uma variável no escopo GLOBAL...
            if (strcmp(simbolo->scope, "GLOBAL") == 0) {
                if (simbolo->id_type == var_k) {
                    fprintf(arquivoSaida, "    %s: .word 0\n", simbolo->name);
                } else if (simbolo->id_type == array_k) {
                    //Declaração de array;
                    int tamanho_bytes = simbolo->size;
                    fprintf(arquivoSaida, "    %s: .space %d\n", simbolo->name, tamanho_bytes);
                }
            }
            simbolo = simbolo->next;
        }
    }
    fprintf(arquivoSaida, "\n.text\n");
    fprintf(arquivoSaida, "    NOP\n");
    fprintf(arquivoSaida, "    MOVI SP, #0\n");
    fprintf(arquivoSaida, "    MOVI FP, #0\n");
    
    // Itera por toda a lista de instruções TAC
    TacNo *percorre = listaTac->inicio;
    while (percorre != NULL) {
        traduzir_tac_para_assembly(arquivoSaida, percorre, tabela_simbolos);
        percorre = percorre->proximo;
    }

    fprintf(arquivoSaida, "\n; Fim do programa\n");
}

