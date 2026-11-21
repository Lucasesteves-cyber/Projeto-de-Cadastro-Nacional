#ifndef FUNCOES_HASH_H
#define FUNCOES_HASH_H

#define DIVISAO 0
#define POLINOMIAL 1

int hash_divisao(char* matricula, int m);
int hash_polinomial(char* matricula, int m);
int calcular_hash(char* matricula, int m, int tipo_hash);
int proximo_primo(int n);

#endif