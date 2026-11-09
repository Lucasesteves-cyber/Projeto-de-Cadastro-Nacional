#include "hash_encadeado.h"

#include <stdlib.h>
#include <string.h>

typedef struct NoHash {
    Registro *reg;
    struct NoHash *prox;
} NoHash;

struct HashEncadeado {
    NoHash **baldes;
    unsigned char *usado;
    size_t m, n, colisoes;
    FuncaoHash h;
};

HashEncadeado *he_criar(size_t m, FuncaoHash h) {
    HashEncadeado *t = malloc(sizeof(HashEncadeado));
    if (!t) return NULL;
    t->baldes = calloc(m, sizeof(NoHash *));
    t->usado = calloc(m, 1);
    if (!t->baldes || !t->usado) {
        free(t->baldes);
        free(t->usado);
        free(t);
        return NULL;
    }
    t->m = m;
    t->n = 0;
    t->colisoes = 0;
    t->h = h;
    return t;
}

int he_inserir(HashEncadeado *t, Registro *r) {
    NoHash *novo = malloc(sizeof(NoHash));
    if (!novo) return 0;
    size_t i = t->h(r->matricula, t->m);
    if (t->usado[i]) t->colisoes++;
    else t->usado[i] = 1;

    novo->reg = r;
    novo->prox = t->baldes[i];
    t->baldes[i] = novo;
    t->n++;
    return 1;
}

Registro *he_buscar(const HashEncadeado *t, const char *matricula) {
    size_t i = t->h(matricula, t->m);
    for (NoHash *p = t->baldes[i]; p; p = p->prox)
        if (strcmp(p->reg->matricula, matricula) == 0) return p->reg;
    return NULL;
}

size_t he_tamanho(const HashEncadeado *t) { return t->n; }
size_t he_capacidade(const HashEncadeado *t) { return t->m; }
double he_fator_carga(const HashEncadeado *t) { return (double)t->n / (double)t->m; }
size_t he_colisoes(const HashEncadeado *t) { return t->colisoes; }

size_t he_maior_lista(const HashEncadeado *t) {
    size_t maior = 0;
    for (size_t i = 0; i < t->m; i++) {
        size_t c = 0;
        for (NoHash *p = t->baldes[i]; p; p = p->prox) c++;
        if (c > maior) maior = c;
    }
    return maior;
}

void he_liberar(HashEncadeado *t) {
    if (!t) return;
    for (size_t i = 0; i < t->m; i++) {
        NoHash *p = t->baldes[i];
        while (p) {
            NoHash *q = p->prox;
            free(p);
            p = q;
        }
    }
    free(t->baldes);
    free(t->usado);
    free(t);
}
