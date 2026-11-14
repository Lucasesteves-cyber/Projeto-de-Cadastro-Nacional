#include "hash_aberto.h"

#include <stdlib.h>
#include <string.h>

/*
 * Otimizacao: "usado" (indices iniciais ja gerados, para contar colisoes)
 * e um vetor de bits, 8x menor que um byte por indice.
 */
struct HashAberto {
    Registro **slots;
    unsigned char *usado; /* 1 bit por indice */
    size_t m, n, colisoes;
    FuncaoHash h;
};

HashAberto *ha_criar(size_t m, FuncaoHash h) {
    HashAberto *t = malloc(sizeof(HashAberto));
    if (!t) return NULL;
    t->slots = calloc(m, sizeof(Registro *));
    t->usado = calloc(m / 8 + 1, 1);
    if (!t->slots || !t->usado) {
        free(t->slots);
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

int ha_inserir(HashAberto *t, Registro *r) {
    if (t->n >= t->m) return 0;
    size_t i = t->h(r->matricula, t->m);
    unsigned char bit = (unsigned char)(1u << (i & 7));
    if (t->usado[i >> 3] & bit) t->colisoes++;
    else t->usado[i >> 3] |= bit;

    while (t->slots[i] != NULL) {
        if (++i == t->m) i = 0;
    }
    t->slots[i] = r;
    t->n++;
    return 1;
}

Registro *ha_buscar(const HashAberto *t, const char *matricula) {
    size_t i = t->h(matricula, t->m);
    size_t passos = 0;
    while (t->slots[i] != NULL && passos < t->m) {
        if (strcmp(t->slots[i]->matricula, matricula) == 0) return t->slots[i];
        if (++i == t->m) i = 0;
        passos++;
    }
    return NULL;
}

size_t ha_tamanho(const HashAberto *t) { return t->n; }
size_t ha_capacidade(const HashAberto *t) { return t->m; }
double ha_fator_carga(const HashAberto *t) { return (double)t->n / (double)t->m; }
size_t ha_colisoes(const HashAberto *t) { return t->colisoes; }

void ha_liberar(HashAberto *t) {
    if (!t) return;
    free(t->slots);
    free(t->usado);
    free(t);
}
