#include"funcoes_hash.h"

unsigned long long matricula_para_numero(char* matricula){
    unsigned long long k = 0;
    int i = 0;
    while(matricula[i] >= '0' && matricula[i] <= '9'){
        k = k * 10 + (matricula[i] - '0');
        i++;
    }
    if(matricula[i] == '\0') return k;

    k = 0;
    for(i = 0; matricula[i] != '\0'; i++){
        k = k * 31 + (unsigned char)matricula[i];
    }
    return k;
}

int hash_divisao(char* matricula, int m){
    return matricula_para_numero(matricula) % m;
}

int hash_fnv1a(char* matricula, int m){
    unsigned long long h = 14695981039346656037ULL;
    for(int i = 0; matricula[i] != '\0'; i++){
        h = h ^ (unsigned char)matricula[i];
        h = h * 1099511628211ULL;
    }
    return h % m;
}

int calcular_hash(char* matricula, int m, int tipo_hash){
    if(tipo_hash == DIVISAO)
        return hash_divisao(matricula, m);
    return hash_fnv1a(matricula, m);
}

int eh_primo(int n){
    if(n < 2) return 0;
    if(n % 2 == 0) return n == 2;
    for(int i = 3; i * i <= n; i += 2){
        if(n % i == 0) return 0;
    }
    return 1;
}

int proximo_primo(int n){
    while(!eh_primo(n)) n++;
    return n;
}
