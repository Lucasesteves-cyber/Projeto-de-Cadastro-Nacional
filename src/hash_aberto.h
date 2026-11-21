#ifndef HASH_ABERTO_H
#define HASH_ABERTO_H

#include "registro.h"

typedef struct hash_aberto* HashAberto;

HashAberto criar_hash_aberto(int m, int tipo_hash);
int inserir_hash_aberto(HashAberto ha, Registro* r);
Registro* buscar_hash_aberto(HashAberto ha, char* matricula);
int colisoes_hash_aberto(HashAberto ha);
double fator_carga_hash_aberto(HashAberto ha);
void destruir_hash_aberto(HashAberto ha);

#endif