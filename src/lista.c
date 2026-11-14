#include<stdlib.h>
#include<string.h>
#include"lista.h"
#include"funcoes_hash.h"

struct elem{
    unsigned long long numero;
    Registro* reg;
    struct elem* prox;
};
typedef struct elem Elem;

struct lista{
    int qtd;
    Elem* inicio;
};

Lista criar_lista(){
    Lista li = malloc(sizeof(struct lista));
    if(li != NULL){
        li->qtd = 0;
        li->inicio = NULL;
    }
    return li;
}

int inserir_inicio(Lista li, Registro* r){
    Elem* no = malloc(sizeof(Elem));
    if(no != NULL){
        no->numero = matricula_para_numero(r->matricula);
        no->reg = r;
        no->prox = li->inicio;
        li->inicio = no;
        li->qtd++;
        return 1;
    }
    return 0;
}

Registro* buscar_lista(Lista li, char* matricula){
    unsigned long long numero = matricula_para_numero(matricula);
    Elem* aux = li->inicio;
    while(aux != NULL){
        if(aux->numero == numero && strcmp(aux->reg->matricula, matricula) == 0)
            return aux->reg;
        aux = aux->prox;
    }
    return NULL;
}

int tamanho_lista(Lista li){
    return li->qtd;
}

void destruir_lista(Lista li){
    if(li == NULL) return;
    Elem* aux = li->inicio;
    while(aux != NULL){
        Elem* atual = aux;
        aux = aux->prox;
        free(atual);
    }
    free(li);
}
