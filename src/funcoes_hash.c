#include "funcoes_hash.h"

uint64_t matricula_para_inteiro(const char *s) {
    uint64_t k = 0;
    const char *p = s;
    while (*p >= '0' && *p <= '9') { k = k * 10u + (uint64_t)(*p - '0'); p++; }
    if (*p == '\0') return k;

    k = 0;
    for (p = s; *p; p++) k = k * 31u + (unsigned char)*p;
    return k;
}

size_t hash_divisao(const char *matricula, size_t m) {
    return (size_t)(matricula_para_inteiro(matricula) % (uint64_t)m);
}

size_t hash_fnv1a(const char *matricula, size_t m) {
    uint64_t h = 14695981039346656037ULL;
    for (const unsigned char *p = (const unsigned char *)matricula; *p; p++) {
        h ^= *p;
        h *= 1099511628211ULL;
    }
    return (size_t)(h % (uint64_t)m);
}

static int eh_primo(size_t n) {
    if (n < 2) return 0;
    if (n % 2 == 0) return n == 2;
    if (n % 3 == 0) return n == 3;
    for (size_t i = 5; i * i <= n; i += 6)
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    return 1;
}

size_t proximo_primo(size_t n) {
    while (!eh_primo(n)) n++;
    return n;
}
