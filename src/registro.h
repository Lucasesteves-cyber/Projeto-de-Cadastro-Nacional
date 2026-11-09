#ifndef REGISTRO_H
#define REGISTRO_H

#include <stddef.h>

#define MAT_MAX 24

typedef struct {
    char matricula[MAT_MAX];
    char *nome;
    char *curso;
} Registro;

typedef struct BaseDados BaseDados;

BaseDados *base_carregar(const char *caminho, size_t limite);

size_t base_tamanho(const BaseDados *b);
Registro *base_registro(BaseDados *b, size_t i);
void base_liberar(BaseDados *b);

#endif
