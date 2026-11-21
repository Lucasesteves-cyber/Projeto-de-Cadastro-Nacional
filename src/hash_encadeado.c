#include <stdlib.h>
#include <string.h>
#include "hash_encadeado.h"
#include "funcoes_hash.h"

typedef struct elem {
    Registro* reg;
    struct elem* prox;
} Elem;

struct hash_encadeado {
    Elem** tabela;
    char* usado;
    int m;
    int qtd;
    int colisoes;
    int tipo_hash;
};

HashEncadeado criar_hash_encadeado(int m, int tipo_hash) {
    HashEncadeado he = (HashEncadeado)malloc(sizeof(struct hash_encadeado));
    if (he == NULL) {
        return NULL;
    }

    he->tabela = (Elem**)malloc(m * sizeof(Elem*));
    he->usado = (char*)malloc(m * sizeof(char));
    if (he->tabela == NULL || he->usado == NULL) {
        free(he->tabela);
        free(he->usado);
        free(he);
        return NULL;
    }

    for (int i = 0; i < m; i++) {
        he->tabela[i] = NULL;
        he->usado[i] = 0;
    }

    he->m = m;
    he->qtd = 0;
    he->colisoes = 0;
    he->tipo_hash = tipo_hash;

    return he;
}

int inserir_hash_encadeado(HashEncadeado he, Registro* r) {
    if (he == NULL) {
        return 0;
    }

    Elem* no = (Elem*)malloc(sizeof(Elem));
    if (no == NULL) {
        return 0;
    }

    int pos = calcular_hash(r->matricula, he->m, he->tipo_hash);

    if (he->usado[pos]) {
        he->colisoes++;
    } else {
        he->usado[pos] = 1;
    }

    no->reg = r;
    no->prox = he->tabela[pos];
    he->tabela[pos] = no;
    he->qtd++;

    return 1;
}

Registro* buscar_hash_encadeado(HashEncadeado he, char* matricula) {
    if (he == NULL) {
        return NULL;
    }

    int pos = calcular_hash(matricula, he->m, he->tipo_hash);
    Elem* aux = he->tabela[pos];

    while (aux != NULL) {
        if (strcmp(aux->reg->matricula, matricula) == 0) {
            return aux->reg;
        }
        aux = aux->prox;
    }

    return NULL;
}

int colisoes_hash_encadeado(HashEncadeado he) {
    return he->colisoes;
}

double fator_carga_hash_encadeado(HashEncadeado he) {
    return (double)he->qtd / he->m;
}

void destruir_hash_encadeado(HashEncadeado he) {
    if (he == NULL) {
        return;
    }

    for (int i = 0; i < he->m; i++) {
        Elem* aux = he->tabela[i];
        while (aux != NULL) {
            Elem* atual = aux;
            aux = aux->prox;
            free(atual);
        }
    }

    free(he->tabela);
    free(he->usado);
    free(he);
}
