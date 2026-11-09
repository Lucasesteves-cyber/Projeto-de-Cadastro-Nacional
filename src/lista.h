#ifndef LISTA_H
#define LISTA_H

#include "registro.h"

typedef struct Lista Lista;

Lista *lista_criar(void);
int lista_inserir(Lista *l, Registro *r);
Registro *lista_buscar(const Lista *l, const char *matricula);
size_t lista_tamanho(const Lista *l);
void lista_liberar(Lista *l);

#endif
