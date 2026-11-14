#include"registro.h"

typedef struct hash_encadeado* HashEncadeado;

HashEncadeado criar_hash_encadeado(int m, int tipo_hash);
int inserir_hash_encadeado(HashEncadeado he, Registro* r);
Registro* buscar_hash_encadeado(HashEncadeado he, char* matricula);
int colisoes_hash_encadeado(HashEncadeado he);
double fator_carga_hash_encadeado(HashEncadeado he);
void destruir_hash_encadeado(HashEncadeado he);
