#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../globals.h"

Escopo escopo_atual;

void emit_movi(FILE *arquivoSaida, const char* reg_destino, int valor) {
    if (valor <= MAX_IMMEDIATE) {
        // O valor cabe em uma única instrução, gera o MOVI diretamente.
        fprintf(arquivoSaida, "    MOVI %s, #%d\n", reg_destino, valor);
    } else {
        // O valor é grande, precisa ser expandido usando a lógica de multiplicação.
        int quociente = valor / 1024;
        int resto = valor % 1024;

        fprintf(arquivoSaida, "    ; Carregando valor grande %d em %s (expansao via MUL)\n", valor, reg_destino);
        
        // 1. Carrega o quociente (multiplicador) em um registrador temporário (Rad).
        fprintf(arquivoSaida, "    MOVI Rad, #%d\n", quociente);
       
        // 2. Multiplica o quociente pelo valor em RBase (que foi inicializado com 1024).
        fprintf(arquivoSaida, "    MUL Rad, Rad, RBase\n");
        
        // 3. Adiciona o resto para compor o valor final no registrador de destino.
        fprintf(arquivoSaida, "    ADDI %s, Rad, #%d\n", reg_destino, resto);
    }
}

void emit_addi(FILE *arquivoSaida, const char* reg_destino, const char* reg_origem, int valor) {
    if (valor <= MAX_IMMEDIATE) {
        fprintf(arquivoSaida, "    ADDI %s, %s, #%d\n", reg_destino, reg_origem, valor);
    } else {
        fprintf(arquivoSaida, "    ; ADDI grande: %s = %s + %d\n", reg_destino, reg_origem, valor);
        // Rad será clobber. RBase deve estar como 1024.
        emit_movi(arquivoSaida, "Rad", valor);   // Rad <- valor grande
        fprintf(arquivoSaida, "    ADD %s, %s, Rad\n", reg_destino, reg_origem);
    }
}

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
        "FUN", "ARG", "LOAD", "EQUAL", "GREATER", "LESS", "LEQ", "IFF", "RET", "GOTO", "LAB",
        "PARAM", "DIV", "MUL", "SUB", "CALL", "END", "STORE", "HALT", "SUM", "ALLOC", "ASSIGN",
        "BRANCH", "SINT", "SBLR"
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
            if (strcmp(tac->op2, "VIDEO_MEMORY") == 0) {
                fprintf(stderr, "\nErro de Compilação: Tentativa de leitura da 'VIDEO_MEMORY'.\n");
                exit(1);
            }
            else if (strcmp(tac->op2, "INSTR_MEMORY") == 0) {
                fprintf(stderr, "\nErro de Compilação: Tentativa de leitura da 'INSTR_MEMORY' (write-only).\n");
                exit(1);
            }
            else if (strcmp(tac->op2, "HD_MEMORY") == 0) {
                fprintf(arquivoSaida, "    ; Lendo do HD mapeado em memória\n");
                emit_movi(arquivoSaida, "Rad", HD_BASE);
                if (strcmp(get_reg(tac->resultado), "") != 0) {
                    char* reg_idx = get_reg(tac->resultado);
                    fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_idx);
                    free(reg_idx);
                }
                fprintf(arquivoSaida, "    LDR %s, [Rad, #0]\n", get_reg(tac->op1));
            }
            else {
                // Verifica se a variável (tac->op1) é global
                Symbol* simbolo = find_symbol(tabela_simbolos, tac->op2, "GLOBAL");
                if (simbolo != NULL) { // É global
                    if (simbolo->id_type == var_k) {
                        fprintf(arquivoSaida, "    ; Acessando variável global '%s'\n", tac->op2);
                        emit_ldr(arquivoSaida, reg_op1, "R0", simbolo->offset);
                    }
                    else if (simbolo->id_type == array_k) {
                        if(strcmp(reg_res, "") == 0) {
                            fprintf(arquivoSaida, "    ; Acessando endereço de array global '%s'\n", tac->op2);
                            //Acessando enderço base
                            emit_movi(arquivoSaida, "Rad", simbolo->offset);
                            fprintf(arquivoSaida, "    MOV %s, Rad\n", reg_op1);
                        }
                        else {
                            fprintf(arquivoSaida, "    ; Acessando array global '%s'\n", tac->op2);
                            // 1. Carrega o endereço base do array global em Rad
                            emit_movi(arquivoSaida, "Rad", simbolo->offset);
                            // 2. Adiciona o deslocamento (índice, que está em reg_res) ao endereço base
                            fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_res);
                            // 3. Carrega o valor do endereço final no registrador de destino (reg_op1)
                            fprintf(arquivoSaida, "    LDR %s, [Rad, #0]\n", reg_op1);
                        }
                    }
                } else { // Não é global, procura no escopo local
                    if ( (atoi(tac->op2) != 0) || (strcmp(tac->op2, "0") == 0) ) {
                        int valor_imediato = atoi(tac->op2);
                        emit_movi(arquivoSaida, reg_op1, valor_imediato);
                    }
                    else {
                        simbolo = find_symbol(tabela_simbolos, tac->op2, escopo_atual.escopo);
                        if (simbolo != NULL) {
                            if (simbolo->id_type == var_k){
                                fprintf(arquivoSaida, "    ; Acessando variavel local '%s'\n", tac->op2);
                                emit_ldr(arquivoSaida, reg_op1, "FP", simbolo->offset);
                            }   
                            else if (simbolo->id_type == array_k){
                                //Acessando endereço do array
                                if(strcmp(reg_res, "") == 0) {
                                    fprintf(arquivoSaida, "    ; Acessando endereço de array local '%s'\n", tac->op2);
                                    fprintf(arquivoSaida, "    MOV Rad, FP\n");
                                    emit_addi(arquivoSaida, "Rad", "Rad", simbolo->offset);
                                    fprintf(arquivoSaida, "    MOV %s, Rad\n", reg_op1);
    
                                }
                                else{
                                    fprintf(arquivoSaida, "    ; Acessando array local '%s'\n", tac->op2);
                                    // 1. Carrega o endereço base do array local em Rad
                                    fprintf(arquivoSaida, "    MOV Rad, FP\n");
                                    emit_addi(arquivoSaida, "Rad", "Rad", simbolo->offset);
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
            }
            break;
        }

        case STORE: {
            if (strcmp(tac->op1, "VIDEO_MEMORY") == 0) {
                fprintf(arquivoSaida, "    ; Armazenando em VIDEO_MEMORY\n");
                emit_movi(arquivoSaida, "Rad", VIDEO_BASE);
                fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_op2);
                fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_res);
            }
            else if (strcmp(tac->op1, "HD_MEMORY") == 0) {
                fprintf(arquivoSaida, "    ; Escrevendo no HD mapeado\n");
                emit_movi(arquivoSaida, "Rad", HD_BASE);
                fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_op2);
                fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_res);
            }
            else if (strcmp(tac->op1, "INSTR_MEMORY") == 0) {
                fprintf(arquivoSaida, "    ; Escrevendo na memória de instruções (write-only)\n");
                emit_movi(arquivoSaida, "Rad", INSTR_BASE);
                fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_op2);
                fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_res);
            }
            else {
                Symbol* simbolo = find_symbol(tabela_simbolos, tac->op1, "GLOBAL");
                if (simbolo != NULL) { // É global
                    if (simbolo->id_type == var_k){
                        fprintf(arquivoSaida, "    ; Armazenando em variável global '%s'\n", tac->op1);
                        emit_str(arquivoSaida, reg_res, "R0", simbolo->offset);
                    }   
                    else if (simbolo->id_type == array_k){
                        fprintf(arquivoSaida, "    ;Armazenando em array global '%s'\n", tac->op1);
                        // 1. Carrega o endereço base do array global em Rad
                        emit_movi(arquivoSaida, "Rad", simbolo->offset);
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
                            emit_str(arquivoSaida, reg_res, "FP", simbolo->offset);
                        } 
                        else if (simbolo->id_type == array_k){
                            fprintf(arquivoSaida, "    ; Armazenando em array local '%s'\n", tac->op1);
                            // 1. Carrega o endereço base do array local em Rad
                            fprintf(arquivoSaida, "    MOV Rad, FP\n");
                            emit_addi(arquivoSaida, "Rad", "Rad", simbolo->offset);
                            
                            // 2. Adiciona o deslocamento (índice, que está em reg_op2) ao endereço base
                            fprintf(arquivoSaida, "    ADD Rad, Rad, %s\n", reg_op2);
    
                            // 3. Armazena o valor (que está em reg_res) no endereço final
                            fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_res);
                        }
                        else if (simbolo->id_type == param_k){
                            fprintf(arquivoSaida, "    ; Armazenando em param '%s'\n", tac->op1);
                            fprintf(arquivoSaida, "    MOV Rad, FP\n");
                            fprintf(arquivoSaida, "    SUBI Rad, Rad, #%d\n", atoi(escopo_atual.qtd_param) - simbolo->offset);
                            fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_res); // Usa reg_res, que contém o valor
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
            else if(strcmp(tac->op2, "FinishInterrupt") == 0){
                //Não faz nada
            }
            else {
                emit_addi(arquivoSaida, "Rlink", "Rlink", 1);
                fprintf(arquivoSaida, "    STR Rlink [SP #0]\n");
                emit_addi(arquivoSaida, "SP", "SP", 1);
            }
            
            strcpy(escopo_atual.escopo, tac->op2);
            strcpy(escopo_atual.qtd_param, tac->resultado);
            break;
        }
        case END:
            if (strcmp(tac->op1, "main") != 0){
                Symbol* simbolo = find_symbol(tabela_simbolos, tac->op1, "GLOBAL");
                if(strcmp(tac->op1, "FinishInterrupt") == 0) {
                    //Retornar os valores dos registradores base
                    fprintf(arquivoSaida,"    SBL R0, R0\n");
                    fprintf(arquivoSaida,"    MOVI Rad, #%d\n", 2);
                    fprintf(arquivoSaida,"    SBL Rad, R0\n");
                    //Retornar para o início do programa
                    fprintf(arquivoSaida,"    BI #0\n");
                }
                else if(strcmp(tac->op1, "PrintInterrupt") == 0){

                }
                else if(strcmp(tac->op1, "ClockInterrupt") == 0){

                }
                //Retorno apenas em funções void
                else if(strcmp(simbolo->type, "void") == 0){
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
                emit_addi(arquivoSaida, "SP", "SP", 1);
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
                emit_addi(arquivoSaida, "SP", "SP", 1);
            }

            break;
        }
        case ALLOC: {
            if (strcmp(tac->op1, "VIDEO_MEMORY") == 0 ||
                strcmp(tac->op1, "HD_MEMORY") == 0 ||
                strcmp(tac->op1, "INSTR_MEMORY") == 0) {
                // memória mapeada – nada a fazer
            }
            else if (strcmp(tac->resultado, "") == 0) {
                emit_addi(arquivoSaida, "SP", "SP", 1);
            }
            else {
                if (is_numeric(tac->resultado)) {
                    emit_addi(arquivoSaida, "SP", "SP", atoi(tac->resultado));
                } else {
                    fprintf(arquivoSaida, "    ; ADD com valor não imediato literal\n");
                    fprintf(arquivoSaida, "    MOV Rad, %s\n", get_reg(tac->resultado));
                    fprintf(arquivoSaida, "    ADD SP, SP, Rad\n");
                }
            }
            break;
        }
        case ASSIGN: {
            fprintf(arquivoSaida,"    MOV %s, %s\n", get_reg(tac->op1), get_reg(tac->op2));
            break;
        }
        case RET: {
            if (tac->op1[0] != '\0') {
                fprintf(arquivoSaida,"    MOV Rret, %s\n", reg_op1);
            }
            // fprintf(arquivoSaida,"    SUBI SP, SP, #1\n");
            fprintf(arquivoSaida,"    LDR Rlink [FP #1]\n", get_reg(tac->op1), get_reg(tac->op2));
            fprintf(arquivoSaida,"    MOV SP, FP\n", get_reg(tac->op1), get_reg(tac->op2));
            fprintf(arquivoSaida,"    LDR FP [FP #0]\n", get_reg(tac->op1), get_reg(tac->op2));
            fprintf(arquivoSaida,"    B Rlink\n", get_reg(tac->op1), get_reg(tac->op2));
            break;
        }
        case BRANCH: {
            fprintf(arquivoSaida, "    BI #0\n");
            break;
        }
        case SINT: {
            fprintf(arquivoSaida, "    SIR %s, %s\n", reg_op1, reg_op2);
            break;
        }
        case SBLR: {
            fprintf(arquivoSaida, "    SBL %s, %s\n", reg_op1, reg_op2);
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
        case LEQ:{
            fprintf(arquivoSaida, "    CMP %s, %s\n",reg_op2, reg_res);
            fprintf(arquivoSaida, "    BGT %s\n", tac->proximo->op2); // Salta para fora do laço se op1 > op2
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

// Lê de [base + offset] em reg_destino
void emit_ldr(FILE *arquivoSaida, const char* reg_destino, const char* reg_base, int offset) {
    if (offset <= MAX_IMMEDIATE) {
        fprintf(arquivoSaida, "    LDR %s, [%s, #%d]\n", reg_destino, reg_base, offset);
    } else {
        fprintf(arquivoSaida, "    ; LDR grande: %s = [%s + %d]\n", reg_destino, reg_base, offset);
        fprintf(arquivoSaida, "    MOV Rad, %s\n", reg_base);
        emit_addi(arquivoSaida, "Rad", "Rad", offset);
        fprintf(arquivoSaida, "    LDR %s, [Rad, #0]\n", reg_destino);
    }
}

// Escreve src em [base + offset]
void emit_str(FILE *arquivoSaida, const char* reg_src, const char* reg_base, int offset) {
    if (offset <= MAX_IMMEDIATE) {
        fprintf(arquivoSaida, "    STR %s, [%s, #%d]\n", reg_src, reg_base, offset);
    } else {
        fprintf(arquivoSaida, "    ; STR grande: [%s + %d] = %s\n", reg_base, offset, reg_src);
        fprintf(arquivoSaida, "    MOV Rad, %s\n", reg_base);
        emit_addi(arquivoSaida, "Rad", "Rad", offset);
        fprintf(arquivoSaida, "    STR %s, [Rad, #0]\n", reg_src);
    }
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
    emit_movi(arquivoSaida, "SP", 0);
    emit_movi(arquivoSaida, "FP", 0);
    fprintf(arquivoSaida, "    MOVI RBase, #1023\n");
    fprintf(arquivoSaida, "    ADDI RBase, RBase, #1\n");
    
    // Itera por toda a lista de instruções TAC
    TacNo *percorre = listaTac->inicio;
    while (percorre != NULL) {
        traduzir_tac_para_assembly(arquivoSaida, percorre, tabela_simbolos);
        percorre = percorre->proximo;
    }

    fprintf(arquivoSaida, "\n; Fim do programa\n");
}

