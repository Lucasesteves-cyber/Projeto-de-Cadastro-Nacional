#include <stdlib.h>
#include "funcoes_hash.h"

int hash_divisao(char* matricula, int m) {
    unsigned long long num = strtoull(matricula, NULL, 10);
    return num % m;
}

int hash_polinomial(char* matricula, int m) {
    unsigned int h = 0;
    for (int i = 0; matricula[i] != '\0'; i++) {
        h = (h * 31 + matricula[i]) % m;
    }
    return h;
}

int calcular_hash(char* matricula, int m, int tipo_hash) {
    if (tipo_hash == DIVISAO) {
        return hash_divisao(matricula, m);
    }
    return hash_polinomial(matricula, m);
}

int eh_primo(int n) {
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int proximo_primo(int n) {
    while (!eh_primo(n)) {
        n++;
    }
    return n;
}
