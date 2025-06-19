#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../globals.h"

// Define the current_scope variable
char *current_scope = "GLOBAL";

HashTable* create_table() {
    HashTable *ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) return NULL;
    for (int i = 0; i < TABLE_SIZE; i++)
        ht->table[i] = NULL;
    return ht;
}

// hash
unsigned int hash(char *scope, char *name) {
    char key[200];  // Buffer to store "scope+name"
    snprintf(key, sizeof(key), "%s+%s", scope, name);

    unsigned int hash = 0;
    for (int i = 0; key[i] != '\0'; i++) {
        hash = (hash * 31) + key[i];  // Simple hash function
    }

    return hash;
}


InfoFrame frames[50];
int num_frames = 0;

int get_frame_index(char* scope) {
    for (int i = 0; i < num_frames; i++) {
        if (strcmp(frames[i].nome_escopo, scope) == 0) {
            return i;
        }
    }
    // If not found, create a new frame
    if (num_frames < 50) {
        strncpy(frames[num_frames].nome_escopo, scope, MAXLEXEME);
        frames[num_frames].proximo_offset_disponivel = 0;
        num_frames++;
        return num_frames - 1; // Return the index of the new frame
    }
    return -1; // Not found
}

int atualizar_offset(char* scope, int size) {
    int index = get_frame_index(scope);
    if (index == -1) {
        fprintf(stderr, "Erro: Escopo %s não encontrado.\n", scope);
        return -1; // Error
    }
    
    frames[index].proximo_offset_disponivel += size;
    return frames[index].proximo_offset_disponivel - size; // Return the previous offset
}


void iterate_tree(No* root, HashTable* symbol_table) {
    if (root == NULL) {
        return;
    }
    
    // CONDIÇÃO CORRIGIDA:
    // Processa apenas os nós que iniciam uma declaração (os nós de tipo).
    if (root->kind_node == declaration_k && (strcmp(root->lexmema, "int") == 0 || strcmp(root->lexmema, "void") == 0)) {
        Symbol* new_symbol = NULL;
        
        // O nó de declaração de tipo (int/void) é o pai.
        // As informações do símbolo (nome, tipo de decl) estão no filho.
        if (root->filho[0] != NULL) {
            No* symbol_node = root->filho[0];
            DeclarationKind decl_kind = symbol_node->kind_union.decl;
            char* scope = get_scope(root); // O escopo é determinado pelo pai da declaração

            switch (decl_kind) {
                case var_k:
                    // Variável simples: int x;
                    new_symbol = create_symbol(symbol_node->lexmema, symbol_node->linha, decl_kind, root->lexmema, scope, 1, atualizar_offset(scope, 1));
                    break;

                case array_k:
                    // Declaração de array: int a[10];
                    if (symbol_node->filho[0] != NULL) {
                        int size = atoi(symbol_node->filho[0]->lexmema);
                        new_symbol = create_symbol(symbol_node->lexmema, symbol_node->linha, decl_kind, root->lexmema, scope, size, atualizar_offset(scope, size));
                    }
                    break;

                case fun_k:
                    // Declaração de função: int main(...)
                    // O escopo de uma função é sempre GLOBAL
                    new_symbol = create_symbol(symbol_node->lexmema, symbol_node->linha, decl_kind, root->lexmema, "GLOBAL", 0, 0);
                    break;

                case param_k:
                    // Parâmetro de função: int x
                    new_symbol = create_symbol(symbol_node->lexmema, symbol_node->linha, decl_kind, root->lexmema, scope, 1, atualizar_offset(scope, 1));
                    break;
                
                default:
                    // Tipo de declaração desconhecido ou não aplicável
                    break;
            }

            // Adiciona o símbolo à tabela se ele foi criado
            if (new_symbol != NULL) {
                add_to_hash_table(new_symbol, symbol_table);
            }
            free(scope); // Libera a memória alocada por get_scope
        }
    }

    // Continua a travessia recursiva para todos os filhos e irmãos
    for (int i = 0; i < NUMMAXFILHOS; i++) {
        iterate_tree(root->filho[i], symbol_table);
    }
    iterate_tree(root->irmao, symbol_table);
}

void add_to_hash_table(Symbol* new_symbol, HashTable* symbol_table) {
    unsigned int hash_key = new_symbol->hash_key;
    unsigned int index = hash_key % TABLE_SIZE;
    
    new_symbol->next = symbol_table->table[index];  // Insert at the head (chaining)
    
    symbol_table->table[index] = new_symbol;
}


Symbol* create_symbol(char* name, int linha, DeclarationKind id_type, char* type, char* scope, int size, int offset) {
    // Validação de entrada para evitar que strdup receba NULL
    if (!name || !type || !scope) {
        fprintf(stderr, "Erro: Tentativa de criar símbolo com nome, tipo ou escopo nulos na linha %d.\n", linha);
        return NULL;
    }

    Symbol* new_symbol = malloc(sizeof(Symbol));
    if (!new_symbol) {
        fprintf(stderr, "Falha ao alocar memória para novo símbolo.\n");
        return NULL;
    }
    // Aloca memória para as strings do símbolo
    new_symbol->name = strdup(name);
    new_symbol->type = strdup(type);
    new_symbol->scope = strdup(scope);
    
    // Verifica se as alocações de memória foram bem-sucedidas
    if (!new_symbol->name || !new_symbol->type || !new_symbol->scope) {
        fprintf(stderr, "Falha ao alocar memória para strings do símbolo (strdup).\n");
        // Libera o que foi alocado com sucesso antes de falhar
        free(new_symbol->name);
        free(new_symbol->type);
        free(new_symbol->scope);
        free(new_symbol);
        return NULL;
    }

    new_symbol->hash_key = hash(new_symbol->scope, new_symbol->name);
    new_symbol->linha = linha;
    new_symbol->id_type = id_type;
    new_symbol->size = size;
    new_symbol->offset = offset;
    new_symbol->next = NULL;

    return new_symbol;
}

char* get_scope(No* node) {
    char* ret = malloc(MAXLEXEME);
    if (!ret) {
        fprintf(stderr, "Memory allocation error in get_scope\n");
        exit(1);
    }
    
    No *first_brother = node;
    while (first_brother->prev_irmao != NULL) {
        first_brother = first_brother->prev_irmao;
    }
    
    if (first_brother->pai == NULL) {
        strncpy(ret, current_scope, MAXLEXEME);
        return ret;
    } else if (first_brother->kind_union.decl != fun_k) {
        char* parent_scope = get_scope(first_brother->pai);
        strncpy(ret, parent_scope, MAXLEXEME);
        free(parent_scope);
        return ret;
    } else {
        strncpy(ret, node->lexmema, MAXLEXEME);
        return ret;
    }
}

void print_symbol_table(FILE* file, HashTable* symbol_table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol* current = symbol_table->table[i];
        while (current != NULL) {
            fprintf(file, "Hash: %u Linha: %d, Name: %s, ID Type: %d, Type: %s, Scope: %s, Size: %d, Offset: %d\n", current->hash_key, current->linha, current->name, current->id_type, current->type, current->scope, current->size, current->offset);
            current = current->next;
        }
    }
}

void free_table(HashTable* symbol_table) {
    if (symbol_table == NULL) return;

    for (int i = 0; i < TABLE_SIZE; i++) {
        Symbol* current = symbol_table->table[i];
        while (current != NULL) {
            Symbol* to_free = current;
            current = current->next;

            // Libera as strings alocadas com strdup
            free(to_free->name);
            free(to_free->type);
            free(to_free->scope);
            // Libera o próprio nó do símbolo
            free(to_free);
        }
    }
    // Libera a estrutura da tabela
    free(symbol_table);
}

Symbol* find_symbol(HashTable* symbol_table, char* name, char* scope) {
    unsigned int hash_key = hash(scope, name);
    unsigned int index = hash_key % TABLE_SIZE;
    
    Symbol* current = symbol_table->table[index];
    while (current != NULL) {
        if (current->hash_key == hash_key && strcmp(current->name, name) == 0 && strcmp(current->scope, scope) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

int count_symbol(char* name, char* scope, HashTable* symbol_table) {
    unsigned int hash_key = hash(scope, name);
    unsigned int index = hash_key % TABLE_SIZE;
    
    Symbol* current = symbol_table->table[index];
    int count = 0;
    while (current != NULL) {
        if (current->hash_key == hash_key && strcmp(current->name, name) == 0 && strcmp(current->scope, scope) == 0) {
            count++;
        }
        current = current->next;
    }
    
    return count;
}