#ifndef REGISTRO_H
#define REGISTRO_H

#define TAM_MATRICULA 24

struct registro{
    char matricula[TAM_MATRICULA];
    char* nome;
    char* curso;
};
typedef struct registro Registro;

typedef struct base* Base;

Base carregar_base(char* arquivo, int limite);
int tamanho_base(Base b);
Registro* acessar_registro(Base b, int posicao);
void destruir_base(Base b);

#endif
