#include"registro.h"

typedef struct lista* Lista;

Lista criar_lista();
int inserir_inicio(Lista li, Registro* r);
Registro* buscar_lista(Lista li, char* matricula);
int tamanho_lista(Lista li);
void destruir_lista(Lista li);
