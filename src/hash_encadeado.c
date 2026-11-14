#include "hash_encadeado.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Otimizacoes (o tratamento continua sendo encadeamento separado):
 *  - Nos alocados em blocos (pool) em vez de um malloc por no.
 *  - Cada no guarda os 8 primeiros bytes da matricula; strcmp so e chamado
 *    quando o prefixo bate.
 *  - "usado" (indices iniciais ja gerados, para contar colisoes) virou um
 *    vetor de bits: ocupa 8x menos memoria.
 */

#define NOS_POR_BLOCO 65536u

typedef struct NoHash {
    uint64_t prefixo;
    Registro *reg;
    struct NoHash *prox;
} NoHash;

typedef struct BlocoNos {
    struct BlocoNos *prox;
    size_t usados;
    NoHash nos[NOS_POR_BLOCO];
} BlocoNos;

struct HashEncadeado {
    NoHash **baldes;
    unsigned char *usado; /* 1 bit por indice */
    size_t m, n, colisoes;
    FuncaoHash h;
    BlocoNos *blocos;
};

static uint64_t prefixo_matricula(const char *s) {
    char buf[8] = {0};
    for (int i = 0; i < 8 && s[i]; i++) buf[i] = s[i];
    uint64_t p;
    memcpy(&p, buf, sizeof p);
    return p;
}

static NoHash *novo_no(HashEncadeado *t) {
    if (!t->blocos || t->blocos->usados == NOS_POR_BLOCO) {
        BlocoNos *b = malloc(sizeof(BlocoNos));
        if (!b) return NULL;
        b->prox = t->blocos;
        b->usados = 0;
        t->blocos = b;
    }
    return &t->blocos->nos[t->blocos->usados++];
}

HashEncadeado *he_criar(size_t m, FuncaoHash h) {
    HashEncadeado *t = malloc(sizeof(HashEncadeado));
    if (!t) return NULL;
    t->baldes = calloc(m, sizeof(NoHash *));
    t->usado = calloc(m / 8 + 1, 1);
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
    t->blocos = NULL;
    return t;
}

int he_inserir(HashEncadeado *t, Registro *r) {
    NoHash *novo = novo_no(t);
    if (!novo) return 0;
    size_t i = t->h(r->matricula, t->m);
    unsigned char bit = (unsigned char)(1u << (i & 7));
    if (t->usado[i >> 3] & bit) t->colisoes++;
    else t->usado[i >> 3] |= bit;

    novo->prefixo = prefixo_matricula(r->matricula);
    novo->reg = r;
    novo->prox = t->baldes[i];
    t->baldes[i] = novo;
    t->n++;
    return 1;
}

Registro *he_buscar(const HashEncadeado *t, const char *matricula) {
    size_t i = t->h(matricula, t->m);
    uint64_t pre = prefixo_matricula(matricula);
    for (NoHash *p = t->baldes[i]; p; p = p->prox)
        if (p->prefixo == pre && strcmp(p->reg->matricula, matricula) == 0) return p->reg;
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
    BlocoNos *b = t->blocos;
    while (b) {
        BlocoNos *q = b->prox;
        free(b);
        b = q;
    }
    free(t->baldes);
    free(t->usado);
    free(t);
}
