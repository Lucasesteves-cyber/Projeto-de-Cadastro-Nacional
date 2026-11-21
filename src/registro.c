#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registro.h"

#define TAM_LINHA 256

struct base {
    Registro* registros;
    int qtd;
};

char* copiar_texto(const char* texto) {
    char* copia = (char*)malloc(strlen(texto) + 1);
    if (copia != NULL) {
        strcpy(copia, texto);
    }
    return copia;
}

Base carregar_base(const char* arquivo, int limite) {
    FILE* f = fopen(arquivo, "r");
    if (f == NULL) {
        printf("Erro ao abrir o arquivo %s\n", arquivo);
        return NULL;
    }

    Base b = (Base)malloc(sizeof(struct base));
    if (b == NULL) {
        fclose(f);
        return NULL;
    }

    b->registros = (Registro*)malloc(limite * sizeof(Registro));
    if (b->registros == NULL) {
        free(b);
        fclose(f);
        return NULL;
    }
    b->qtd = 0;

    char linha[TAM_LINHA];
    char separador[2] = ",";
    int primeira_linha = 1;

    while (b->qtd < limite && fgets(linha, TAM_LINHA, f) != NULL) {
        linha[strcspn(linha, "\r\n")] = '\0';
        if (linha[0] == '\0') {
            continue;
        }

        if (primeira_linha) {
            primeira_linha = 0;
            if (strchr(linha, ';') != NULL) {
                separador[0] = ';';
            } else if (strchr(linha, '\t') != NULL) {
                separador[0] = '\t';
            }
            if (linha[0] < '0' || linha[0] > '9') {
                continue;
            }
        }

        char* mat = strtok(linha, separador);
        char* nome = strtok(NULL, separador);
        char* curso = strtok(NULL, separador);

        if (mat == NULL || nome == NULL || curso == NULL) {
            continue;
        }

        Registro* r = &b->registros[b->qtd];
        strncpy(r->matricula, mat, TAM_MATRICULA - 1);
        r->matricula[TAM_MATRICULA - 1] = '\0';

        r->nome = copiar_texto(nome);
        r->curso = copiar_texto(curso);

        if (r->nome == NULL || r->curso == NULL) {
            printf("Memoria insuficiente ao ler a base.\n");
            free(r->nome);
            free(r->curso);
            break;
        }

        b->qtd++;
    }

    fclose(f);
    return b;
}

int tamanho_base(Base b) {
    if (b == NULL) {
        return 0;
    }
    return b->qtd;
}

Registro* acessar_registro(Base b, int posicao) {
    return &b->registros[posicao];
}

void destruir_base(Base b) {
    if (b == NULL) {
        return;
    }

    for (int i = 0; i < b->qtd; i++) {
        free(b->registros[i].nome);
        free(b->registros[i].curso);
    }

    free(b->registros);
    free(b);
}
