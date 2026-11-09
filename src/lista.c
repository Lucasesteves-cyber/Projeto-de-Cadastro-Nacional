#include "lista.h"

#include <stdlib.h>
#include <string.h>

typedef struct No {
    Registro *reg;
    struct No *prox;
} No;

struct Lista {
    No *inicio;
    size_t n;
};

Lista *lista_criar(void) {
    return calloc(1, sizeof(Lista));
}

int lista_inserir(Lista *l, Registro *r) {
    No *novo = malloc(sizeof(No));
    if (!novo) return 0;
    novo->reg = r;
    novo->prox = l->inicio;
    l->inicio = novo;
    l->n++;
    return 1;
}

Registro *lista_buscar(const Lista *l, const char *matricula) {
    for (No *p = l->inicio; p; p = p->prox)
        if (strcmp(p->reg->matricula, matricula) == 0) return p->reg;
    return NULL;
}

size_t lista_tamanho(const Lista *l) { return l->n; }

void lista_liberar(Lista *l) {
    if (!l) return;
    No *p = l->inicio;
    while (p) {
        No *q = p->prox;
        free(p);
        p = q;
    }
    free(l);
}
