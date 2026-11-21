#include <stdlib.h>
#include <string.h>
#include "hash_aberto.h"
#include "funcoes_hash.h"

struct hash_aberto {
    Registro** tabela;
    char* usado;
    int m;
    int qtd;
    int colisoes;
    int tipo_hash;
};

HashAberto criar_hash_aberto(int m, int tipo_hash) {
    HashAberto ha = (HashAberto)malloc(sizeof(struct hash_aberto));
    if (ha == NULL) {
        return NULL;
    }

    ha->tabela = (Registro**)malloc(m * sizeof(Registro*));
    ha->usado = (char*)malloc(m * sizeof(char));
    if (ha->tabela == NULL || ha->usado == NULL) {
        free(ha->tabela);
        free(ha->usado);
        free(ha);
        return NULL;
    }

    for (int i = 0; i < m; i++) {
        ha->tabela[i] = NULL;
        ha->usado[i] = 0;
    }

    ha->m = m;
    ha->qtd = 0;
    ha->colisoes = 0;
    ha->tipo_hash = tipo_hash;

    return ha;
}

int inserir_hash_aberto(HashAberto ha, Registro* r) {
    if (ha == NULL || ha->qtd == ha->m) {
        return 0;
    }

    int pos = calcular_hash(r->matricula, ha->m, ha->tipo_hash);

    if (ha->usado[pos]) {
        ha->colisoes++;
    } else {
        ha->usado[pos] = 1;
    }

    while (ha->tabela[pos] != NULL) {
        pos = (pos + 1) % ha->m;
    }

    ha->tabela[pos] = r;
    ha->qtd++;
    return 1;
}

Registro* buscar_hash_aberto(HashAberto ha, char* matricula) {
    if (ha == NULL) {
        return NULL;
    }

    int pos = calcular_hash(matricula, ha->m, ha->tipo_hash);
    int passos = 0;

    while (ha->tabela[pos] != NULL && passos < ha->m) {
        if (strcmp(ha->tabela[pos]->matricula, matricula) == 0) {
            return ha->tabela[pos];
        }
        pos = (pos + 1) % ha->m;
        passos++;
    }

    return NULL;
}

int colisoes_hash_aberto(HashAberto ha) {
    return ha->colisoes;
}

double fator_carga_hash_aberto(HashAberto ha) {
    return (double)ha->qtd / ha->m;
}

void destruir_hash_aberto(HashAberto ha) {
    if (ha == NULL) {
        return;
    }
    free(ha->tabela);
    free(ha->usado);
    free(ha);
}
