#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../globals.h"
#include "../analise_sintatica.tab.h"

extern char *yytext;
extern int yylinenum;
extern No *raizArvore;
//extern Tac *tac;

int main(int argc, char **argv) {
    FILE *arvore = fopen("outputs/arvore.txt", "w");
    FILE *tabsimb = fopen("outputs/tabsimb.txt", "w");
    FILE *codInterm = fopen("outputs/codInterm.txt", "w");
    // yydebug = 0;
    Tac *tac = NULL;
    int token = 1;
    int sintatica = yyparse();
    if (sintatica == 0) {
        print_tree(arvore, raizArvore, 0, 0);
        
        HashTable *hashTable = create_table(TABLE_SIZE);
        iterate_tree(raizArvore, hashTable);
        print_symbol_table(tabsimb, hashTable);
    
        semantic_analysis(raizArvore, hashTable);
        check_main_function();

        //código intermediário
        tac = criarTac(tac);
        percorrer_arvore(raizArvore, &tac, hashTable, 0, 0);
        tac = criarNoTac(tac, HALT, "", "", "");
        imprimirTac(codInterm, tac);
        
        //Gerador de código assembly
        FILE *assembly = fopen("outputs/assembly.asm", "w");
        gerar_codigo_final(assembly, tac, hashTable);
        liberarTac(tac);
        free_table(hashTable);
        free_tree(raizArvore);
    } else {
        exit(1);
    }
    return 0;
}

// //Debugar com Valgrind

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "../globals.h"
// #include "../analise_sintatica.tab.h"

// // Variáveis externas do Flex e Bison
// extern int yylex(void);
// extern int yylinenum;
// extern char *yytext;
// extern FILE *yyin; // Variável do Flex que determina a fonte de entrada
// extern No *raizArvore;
// // extern int numeroDeErrosSintaticos; // Descomente se você tiver um contador de erros

// int main(int argc, char **argv) {
//     // 1. VERIFICA SE UM ARQUIVO FOI FORNECIDO
//     if (argc < 2) {
//         fprintf(stderr, "Erro: Nenhum arquivo de entrada especificado.\n");
//         fprintf(stderr, "Uso: %s <caminho_para_o_arquivo>\n", argv[0]);
//         return 1; // Encerra com erro
//     }

//     // 2. TENTA ABRIR O ARQUIVO DE ENTRADA
//     FILE *inputFile = fopen(argv[1], "r");
//     if (!inputFile) {
//         perror("Erro ao abrir o arquivo de entrada");
//         return 1; // Encerra com erro
//     }

//     // 3. DIRECIONA O ANALISADOR LÉXICO PARA LER DO ARQUIVO
//     yyin = inputFile;

//     // --- O resto do seu código main continua aqui ---
//     FILE *arvore = fopen("outputs/arvore.txt", "w");
//     FILE *tabsimb = fopen("outputs/tabsimb.txt", "w");
//     FILE *codInterm = fopen("outputs/codInterm.txt", "w");
    
//     printf("Iniciando análise do arquivo: %s\n", argv[1]);
    
//     int sintatica_status = yyparse();
    
//     if (sintatica_status == 0) {
//         printf("Análise sintática concluída com sucesso.\n");
//         print_tree(arvore, raizArvore, 0, 0);
        
//         HashTable *hashTable = create_table();
//         iterate_tree(raizArvore, hashTable);
        
//         // A chamada para calcular_layout_de_pilha está faltando aqui,
//         // mas o segmentation fault ocorre nela, então vamos adicioná-la para depuração.

//         print_symbol_table(tabsimb, hashTable);
    
//         // semantic_analysis(raizArvore, hashTable);
//         // check_main_function();

//         //código intermediário
//         Tac *tac = criarTac(NULL);
//         percorrer_arvore(raizArvore, &tac, 0);
//         tac = criarNoTac(tac, HALT, "", "", "");
//         imprimirTac(codInterm, tac);
        
//         //Gerador de código assembly
//         FILE *assembly = fopen("outputs/assembly.asm", "w");
//         gerar_codigo_final(assembly, tac, hashTable);
//         liberarTac(tac);
//         // Libera a memória da árvore sintática
//         free_table(hashTable);
//         free_tree(raizArvore);

//     } else {
//         fprintf(stderr, "Análise sintática falhou.\n");
//         exit(1);
//     }

//     // 4. FECHA TODOS OS ARQUIVOS
//     if (arvore) fclose(arvore);
//     if (tabsimb) fclose(tabsimb);
//     if (codInterm) fclose(codInterm);
//     fclose(inputFile); // Não se esqueça de fechar o arquivo de entrada!


//     return 0;
// }