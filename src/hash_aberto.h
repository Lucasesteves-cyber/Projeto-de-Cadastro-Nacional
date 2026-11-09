#ifndef HASH_ABERTO_H
#define HASH_ABERTO_H

#include "funcoes_hash.h"
#include "registro.h"

typedef struct HashAberto HashAberto;

HashAberto *ha_criar(size_t m, FuncaoHash h);
int ha_inserir(HashAberto *t, Registro *r);
Registro *ha_buscar(const HashAberto *t, const char *matricula);

size_t ha_tamanho(const HashAberto *t);
size_t ha_capacidade(const HashAberto *t);
double ha_fator_carga(const HashAberto *t);
size_t ha_colisoes(const HashAberto *t);
void ha_liberar(HashAberto *t);

#endif
