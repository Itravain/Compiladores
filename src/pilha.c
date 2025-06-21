#include "../globals.h"
#include "stdlib.h"
#include <string.h> // Necessário para strncpy
#include <stdio.h>  // Necessário para fprintf

Pilha* criar_pilha() {
    Pilha *p = (Pilha*) malloc(sizeof(Pilha));
    if (p != NULL) {
        p->qtd_nos = 0;
        p->topo = NULL;
    }
    return p;
}

void push(Pilha *p, const char *palavra) {
    if (p == NULL) return;

    NoPilha *novo_no = (NoPilha*) malloc(sizeof(NoPilha));
    if (novo_no == NULL) {
        fprintf(stderr, "Erro: Falha na alocação de memória para o nó da pilha.\n");
        return;
    }

    strncpy(novo_no->palavra, palavra, MAXLEXEME - 1);
    novo_no->palavra[MAXLEXEME - 1] = '\0';
    
    novo_no->proximo = p->topo;
    p->topo = novo_no;        
    p->qtd_nos++;
}

void pop(Pilha *p) {
    if (p == NULL || p->topo == NULL) {
        return;
    }

    NoPilha *temp = p->topo;
    p->topo = p->topo->proximo;
    free(temp);
    p->qtd_nos--;
}

char* peek(Pilha *p) {
    if (p != NULL && p->topo != NULL) {
        return p->topo->palavra;
    }
    return NULL;
}

void free_stack(Pilha *p) {
    if (p == NULL) return;

    while (p->topo != NULL) {
        pop(p);
    }
    free(p);
}




