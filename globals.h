#define UNKNOWN_TYPE -1
#define TABLE_SIZE 512
#include <stdio.h>

#define NUMMAXFILHOS 3
#define MAXLEXEME 25

//Estrutura de pilha
// O nó que armazena o dado
typedef struct noPilha
{
    char palavra[MAXLEXEME];
    struct noPilha *proximo;
} NoPilha;

// A estrutura que gerencia a pilha
typedef struct pilha
{
    int qtd_nos;
    NoPilha *topo;
} Pilha;


/**
 * @brief Cria e inicializa uma nova pilha vazia.
 * @return Um ponteiro para a nova pilha alocada.
 */
Pilha* criar_pilha();

/**
 * @brief Adiciona uma nova string ao topo da pilha.
 * @param p Ponteiro para a pilha.
 * @param palavra A string a ser adicionada.
 */
void push(Pilha *p, const char *palavra);

/**
 * @brief Remove o elemento do topo da pilha.
 * @param p Ponteiro para a pilha.
 */
void pop(Pilha *p);

/**
 * @brief Retorna a string no topo da pilha sem removê-la.
 * @param p Ponteiro para a pilha.
 * @return A string no topo, ou NULL se a pilha estiver vazia.
 */
char* peek(Pilha *p);

/**
 * @brief Libera toda a memória alocada pela pilha e seus nós.
 * @param p Ponteiro para a pilha a ser liberada.
 */
void free_stack(Pilha *p);


// Árvore de sintaxe abstrata
typedef enum {statement_k, expression_k, declaration_k} NodeKind;
typedef enum {if_k, while_k, return_k, break_k, continue_k, expression_statement_k} StatementKind;
typedef enum {op_k, constant_k, id_k, type_k, arr_k, ativ_k, assign_k, parametro_exp_t} ExpressionKind;
typedef enum {var_k, fun_k, param_k, param_array_k, array_k, unknown_k} DeclarationKind;

typedef struct no
{
    int linha;
    char lexmema[MAXLEXEME];
    int max_index;
    NodeKind kind_node;
    union {
        StatementKind stmt;
        ExpressionKind expr;
        DeclarationKind decl;
    } kind_union;
    struct no *pai;
    struct no *filho[NUMMAXFILHOS];
    struct no *irmao;
    struct no *prev_irmao;
} No;

No * create_node(int linha, const char *lexmema, NodeKind kind, int kind_union);
No * create_tree(int linha, const char *lexmema, NodeKind kind, int kind_union);
No * add_filho(No *pai, No *filho);
No * add_irmao(No *irmao, No *novo);
void free_tree(No *tree);
void print_node(FILE *file, No *node);
void print_tree(FILE *file, No *tree, int depth, int is_irmao);



// Struct dos simbolos
typedef struct Symbol {
    unsigned int hash_key;
    int linha;
    char *name;
    DeclarationKind id_type;
    char *type;
    char *scope;
    int size;
    int offset;
    struct Symbol *next;
} Symbol;


typedef struct HashTable {
    Symbol *table[TABLE_SIZE];
} HashTable;

// Estrutura auxiliar para rastrear o offset de cada função
typedef struct {
    char nome_escopo[MAXLEXEME];
    int proximo_offset_disponivel;
    int proximo_offset_disponivel_param;
} InfoFrame;

extern char *id_lexema;
extern HashTable *symbol_table; // Tabela de simbolos
extern char *current_scope;  // Controla o escopo atual
extern int main_declared;

// Métodos
HashTable* create_table();
unsigned int hash(char *scope, char *name);
void iterate_tree(No* root, HashTable* symbol_table);
void add_to_hash_table(Symbol* symbol, HashTable* symbol_table);
Symbol* create_symbol(char* name, int linha, DeclarationKind id_type, char* type, char* scope, int size, int offset);
char* get_scope(No* root);
void print_symbol_table(FILE *file, HashTable* symbol_table);
Symbol* find_symbol(HashTable* symbol_table, char* name, char* scope);
void calcular_layout_de_pilha(HashTable* symbol_table);

// Análise semântica
void semantic_analysis(No* root, HashTable* symbol_table);
void check_main_function();
int count_symbol(char* name, char* scope, HashTable* symbol_table);
void free_table(HashTable* table);

//Código intermediário
enum operacoes {FUN, ARG, LOAD, EQUAL, GREATER, LESS, IFF, RET, GOTO, LAB, PARAM, DIV, MUL, SUB, CALL, END, STORE, HALT, SUM, ALLOC, ASSIGN};

typedef struct tacNo{
    enum operacoes operacao;
    char resultado[20];
    char op1[20];
    char op2[20];
    struct tacNo *proximo;
    struct tacNo *anterior;
} TacNo;

typedef struct tac{   
    int qtdNos;
    TacNo *inicio;
    TacNo *fim;
}Tac;



// Funções principais
Tac *criarTac(Tac *estrutura_tac);
Tac *criarNoTac(Tac *estrutura_tac, int operacao, const char *op1, const char *op2, const char *resultado);
Tac *liberarTac(Tac *estrutura_tac);
void imprimirTac(FILE *arqCodInterm, Tac *tac);

// Função para percorrer a árvore e gerar TAC
char* percorrer_arvore(No *node_tree, Tac **tac, HashTable *symbol_table ,int expression_parametro, int esq_assign);
void codigo_intermediario(No *node_tree, Tac *tac);

// Gerador de código assembly
typedef struct escopo
{
  char qtd_param[5];
  char escopo[MAXLEXEME];  
}Escopo;


void gerar_assembly(FILE *saida, Tac *lista_tac, HashTable *tabela_simbolos);
void traduzir_tac_para_assembly(FILE *arquivoSaida, TacNo *tac, HashTable *tabela_simbolos);
void gerar_codigo_final(FILE *arquivoSaida, Tac *listaTac, HashTable *tabela_simbolos);
char* get_reg(const char *temp_name);