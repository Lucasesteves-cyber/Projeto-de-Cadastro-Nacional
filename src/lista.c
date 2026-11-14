#include "lista.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*
 * Otimizacoes (a estrutura continua sendo uma lista unicamente encadeada):
 *  - Os nos sao alocados em blocos (pool) em vez de um malloc por no:
 *    insercao mais rapida e nos vizinhos na memoria (busca aproveita a cache).
 *  - Cada no guarda os 8 primeiros bytes da matricula (prefixo). Na busca,
 *    so se chama strcmp quando o prefixo bate; os outros nos sao descartados
 *    sem precisar acessar o Registro.
 */

#define NOS_POR_BLOCO 65536u

typedef struct No {
    uint64_t prefixo;
    Registro *reg;
    struct No *prox;
} No;

typedef struct BlocoNos {
    struct BlocoNos *prox;
    size_t usados;
    No nos[NOS_POR_BLOCO];
} BlocoNos;

struct Lista {
    No *inicio;
    size_t n;
    BlocoNos *blocos;
};

static uint64_t prefixo_matricula(const char *s) {
    char buf[8] = {0};
    for (int i = 0; i < 8 && s[i]; i++) buf[i] = s[i];
    uint64_t p;
    memcpy(&p, buf, sizeof p);
    return p;
}

Lista *lista_criar(void) {
    return calloc(1, sizeof(Lista));
}

static No *novo_no(Lista *l) {
    if (!l->blocos || l->blocos->usados == NOS_POR_BLOCO) {
        BlocoNos *b = malloc(sizeof(BlocoNos));
        if (!b) return NULL;
        b->prox = l->blocos;
        b->usados = 0;
        l->blocos = b;
    }
    return &l->blocos->nos[l->blocos->usados++];
}

int lista_inserir(Lista *l, Registro *r) {
    No *novo = novo_no(l);
    if (!novo) return 0;
    novo->prefixo = prefixo_matricula(r->matricula);
    novo->reg = r;
    novo->prox = l->inicio;
    l->inicio = novo;
    l->n++;
    return 1;
}

Registro *lista_buscar(const Lista *l, const char *matricula) {
    uint64_t pre = prefixo_matricula(matricula);
    for (No *p = l->inicio; p; p = p->prox)
        if (p->prefixo == pre && strcmp(p->reg->matricula, matricula) == 0) return p->reg;
    return NULL;
}

size_t lista_tamanho(const Lista *l) { return l->n; }

void lista_liberar(Lista *l) {
    if (!l) return;
    BlocoNos *b = l->blocos;
    while (b) {
        BlocoNos *q = b->prox;
        free(b);
        b = q;
    }
    free(l);
}
