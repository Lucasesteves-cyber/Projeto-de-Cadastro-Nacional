#define DIVISAO 0
#define FNV1A 1

unsigned long long matricula_para_numero(char* matricula);
int hash_divisao(char* matricula, int m);
int hash_fnv1a(char* matricula, int m);
int calcular_hash(char* matricula, int m, int tipo_hash);
int proximo_primo(int n);
