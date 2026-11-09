#ifndef FUNCOES_HASH_H
#define FUNCOES_HASH_H

#include <stddef.h>
#include <stdint.h>

typedef size_t (*FuncaoHash)(const char *matricula, size_t m);

size_t hash_divisao(const char *matricula, size_t m);

size_t hash_fnv1a(const char *matricula, size_t m);

uint64_t matricula_para_inteiro(const char *matricula);

size_t proximo_primo(size_t n);

#endif
