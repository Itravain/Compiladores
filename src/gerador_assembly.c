#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../globals.h"

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

void traduzir_tac_para_assembly(FILE *arquivoSaida, TacNo *tac, HashTable *tabela_simbolos) {
    const char *op_nomes[] = {
        "FUN", "ARG", "LOAD", "EQUAL", "GREATER", "LESS", "IFF", "RET", "GOTO", "LAB",
        "PARAM", "DIV", "MUL", "SUB", "CALL", "END", "STORE", "HALT", "SUM", "ALLOC", "ASSIGN"
    };
    
    char *reg_res = get_reg(tac->resultado);
    char *reg_op1 = get_reg(tac->op1);
    char *reg_op2 = get_reg(tac->op2);

    fprintf(arquivoSaida, "; TAC: (%s, %s, %s, %s)\n", op_nomes[tac->operacao], tac->resultado, tac->op1, tac->op2);

    switch (tac->operacao) {
        case SUM:
            // Ex: (SUM, t2, t0, t1) -> ADD R2, R0, R1
            fprintf(arquivoSaida, "    ADD %s, %s, %s\n", reg_res, reg_op1, reg_op2);
            break;
        case SUB:
            // Ex: (SUB, t2, t0, t1) -> SUB R2, R0, R1
            fprintf(arquivoSaida, "    SUB %s, %s, %s\n", reg_res, reg_op1, reg_op2);
            break;
        case MUL:
            // Ex: (MUL, t1, t0, t2) -> MUL R1, R0, R2
            fprintf(arquivoSaida, "    MUL %s, %s, %s\n", reg_res, reg_op1, reg_op2);
            break;
        case DIV:
            // Ex: (DIV, t1, t0, t2) -> UDIV R1, R0, R2
            fprintf(arquivoSaida, "    UDIV %s, %s, %s\n", reg_res, reg_op1, reg_op2);
            break;
        case LAB:
            // Ex: (LAB, L1, , ) -> L1:
            fprintf(arquivoSaida, "%s:\n", tac->resultado);
            break;
        case GOTO:
            // Ex: (GOTO, L1, , ) -> BI #L1
            fprintf(arquivoSaida, "    BI #%s\n", tac->resultado); //
            break;
        case HALT:
            // (HALT, , , ) -> FINISH
            fprintf(arquivoSaida, "    FINISH\n"); //
            break;
        case LOAD: {
            // Verifica se a variável (tac->op1) é global
            Symbol* simbolo = find_symbol(tabela_simbolos, tac->op2, "GLOBAL");
            if (simbolo != NULL) {
                fprintf(arquivoSaida, "    ; Acessando variável global '%s'\n", tac->op2);
                // 1. Carrega o endereço da variável em um registrador auxiliar (R15)
                fprintf(arquivoSaida, "    MOVI R26, #%s\n", simbolo->offset);
                // 2. Carrega o VALOR a partir do endereço em R26 para o registrador de destino
                fprintf(arquivoSaida, "    LDR %s, [R26, #0]\n", reg_op1);
            } else {
                // Lógica para variáveis locais virá no Passo 4.B
                fprintf(arquivoSaida, "    ; TODO: LOAD para variável local '%s'\n", tac->op1);
            }
            break;
        }

        case STORE: {
            // A variável de destino está em tac->resultado
            Symbol* simbolo = find_symbol(tabela_simbolos, tac->resultado, "GLOBAL");
            if (simbolo != NULL) { // É global
                fprintf(arquivoSaida, "    ; Armazenando em variável global '%s'\n", tac->resultado);
                // 1. Carrega o endereço da variável em R15
                fprintf(arquivoSaida, "    MOVI R15, #%s\n", tac->resultado);
                // 2. Armazena o VALOR do registrador de origem (reg_op1) no endereço em R15
                fprintf(arquivoSaida, "    STR %s, [R15, #0]\n", reg_op1); //
            } else {
                fprintf(arquivoSaida, "    ; TODO: STORE para variável local '%s'\n", tac->resultado);
            }
            break;
        }
        
        case FUN: {
            fprintf(arquivoSaida, ".%s\n", tac->op2);
            break;
        }
        case END:
            break;
        case IFF:
            break;
        case CALL: {
            if (strcmp(tac->op2, "input") == 0)  {
                fprintf(arquivoSaida, "IN\n");
                fprintf(arquivoSaida, "MOV %s, R27\n", get_reg(tac->op1));

            }
            if (strcmp(tac->op2, "output") == 0)  {
                fprintf(arquivoSaida, "OUT\n");
            }
            
            break;
        } 
        case ARG:
            break;
        case PARAM:
            break;
        case ALLOC:
            break;
        case ASSIGN: {
            fprintf(arquivoSaida,"MOV %s, %s\n", get_reg(tac->op1), get_reg(tac->op2));
            break;
        }
        case RET:
            
            break;
            
        default:
            fprintf(arquivoSaida, "    ; AVISO: Operação TAC %d não reconhecida.\n", tac->operacao);
            break;
    }

    free(reg_res);
    free(reg_op1);
    free(reg_op2);
}


void gerar_codigo_final(FILE *arquivoSaida, Tac *listaTac, HashTable *tabela_simbolos) {
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
    fprintf(arquivoSaida, "b main\n");

    // Itera por toda a lista de instruções TAC
    TacNo *percorre = listaTac->inicio;
    while (percorre != NULL) {
        traduzir_tac_para_assembly(arquivoSaida, percorre, tabela_simbolos);
        percorre = percorre->proximo;
    }

    fprintf(arquivoSaida, "\n; Fim do programa\n");
}

