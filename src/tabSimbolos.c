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

void iterate_tree(No* root, HashTable* symbol_table) {
    if (root == NULL) {
        return;
    }
    
    if (strcmp(root->lexmema, "int") == 0 || strcmp(root->lexmema, "void") == 0) {
        Symbol* new_symbol;
        if (root->filho[0] != NULL){
            new_symbol = create_symbol(root->filho[0]->lexmema, root->linha, root->kind_union.decl, root->lexmema, get_scope(root), 0);
        } else {
            new_symbol = create_symbol(root->lexmema, root->linha, root->kind_union.decl, root->lexmema, get_scope(root), 1);
        }
        if (strcmp(new_symbol->name, "int") != 0 && strcmp(new_symbol->name, "void") != 0)
        {
            add_to_hash_table(new_symbol, symbol_table);
        }
        else{
            free(new_symbol);
        }
        
    }

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


Symbol* create_symbol(char* name, int linha, DeclarationKind id_type, char* type, char* scope, int size) {
    Symbol* new_symbol = malloc(sizeof(Symbol));
    new_symbol->hash_key = hash(scope, name);
    new_symbol->linha = linha;
    new_symbol->name = name;
    new_symbol->id_type = id_type;
    new_symbol->type = type;
    new_symbol->scope = scope;
    new_symbol->size = size;
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
            fprintf(file, "Hash: %u Linha: %d, Name: %s, ID Type: %d, Type: %s, Scope: %s, Size: %d\n", current->hash_key, current->linha, current->name, current->id_type, current->type, current->scope, current->size);
            current = current->next;
        }
    }
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