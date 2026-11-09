#ifndef HASH_ENCADEADO_H
#define HASH_ENCADEADO_H

#include "funcoes_hash.h"
#include "registro.h"

typedef struct HashEncadeado HashEncadeado;

HashEncadeado *he_criar(size_t m, FuncaoHash h);
int he_inserir(HashEncadeado *t, Registro *r);
Registro *he_buscar(const HashEncadeado *t, const char *matricula);

size_t he_tamanho(const HashEncadeado *t);
size_t he_capacidade(const HashEncadeado *t);
double he_fator_carga(const HashEncadeado *t);
size_t he_colisoes(const HashEncadeado *t);
size_t he_maior_lista(const HashEncadeado *t);
void he_liberar(HashEncadeado *t);

#endif
