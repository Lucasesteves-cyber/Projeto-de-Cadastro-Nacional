#include <stdlib.h>
#include <string.h>
#include "lista.h"

typedef struct elem {
    unsigned long long numero;
    Registro* reg;
    struct elem* prox;
} Elem;

struct lista {
    int qtd;
    Elem* inicio;
};

Lista criar_lista() {
    Lista li = (Lista)malloc(sizeof(struct lista));
    if (li != NULL) {
        li->qtd = 0;
        li->inicio = NULL;
    }
    return li;
}

int inserir_inicio(Lista li, Registro* r) {
    if (li == NULL) {
        return 0;
    }

    Elem* no = (Elem*)malloc(sizeof(Elem));
    if (no == NULL) {
        return 0;
    }

    no->numero = strtoull(r->matricula, NULL, 10);
    no->reg = r;
    no->prox = li->inicio;
    li->inicio = no;
    li->qtd++;

    return 1;
}

Registro* buscar_lista(Lista li, char* matricula) {
    if (li == NULL) {
        return NULL;
    }

    unsigned long long numero = strtoull(matricula, NULL, 10);
    Elem* aux = li->inicio;
    while (aux != NULL) {
        if (aux->numero == numero && strcmp(aux->reg->matricula, matricula) == 0) {
            return aux->reg;
        }
        aux = aux->prox;
    }

    return NULL;
}

int tamanho_lista(Lista li) {
    if (li == NULL) {
        return 0;
    }
    return li->qtd;
}

void destruir_lista(Lista li) {
    if (li == NULL) {
        return;
    }

    Elem* aux = li->inicio;
    while (aux != NULL) {
        Elem* atual = aux;
        aux = aux->prox;
        free(atual);
    }

    free(li);
}
