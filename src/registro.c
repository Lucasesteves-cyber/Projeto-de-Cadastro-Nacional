#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include"registro.h"

#define TAM_LINHA 4096
#define MAX_CAMPOS 32

struct base{
    Registro* registros;
    int qtd;
};

char* copiar_texto(char* texto){
    char* copia = malloc(strlen(texto) + 1);
    if(copia != NULL){
        strcpy(copia, texto);
    }
    return copia;
}

void remover_quebra(char* linha){
    int tam = strlen(linha);
    while(tam > 0 && (linha[tam - 1] == '\n' || linha[tam - 1] == '\r')){
        linha[tam - 1] = '\0';
        tam--;
    }
}

char descobrir_separador(char* linha){
    int virgulas = 0, pontos_virgula = 0, tabs = 0;
    for(int i = 0; linha[i] != '\0'; i++){
        if(linha[i] == ',') virgulas++;
        if(linha[i] == ';') pontos_virgula++;
        if(linha[i] == '\t') tabs++;
    }
    if(pontos_virgula > virgulas && pontos_virgula >= tabs) return ';';
    if(tabs > virgulas) return '\t';
    return ',';
}

char* limpar_campo(char* campo){
    while(*campo == ' ' || *campo == '"') campo++;
    int tam = strlen(campo);
    while(tam > 0 && (campo[tam - 1] == ' ' || campo[tam - 1] == '"')){
        campo[tam - 1] = '\0';
        tam--;
    }
    return campo;
}

int separar_campos(char* linha, char separador, char** campos){
    int qtd = 0;
    campos[qtd++] = linha;
    for(int i = 0; linha[i] != '\0' && qtd < MAX_CAMPOS; i++){
        if(linha[i] == separador){
            linha[i] = '\0';
            campos[qtd++] = &linha[i + 1];
        }
    }
    for(int i = 0; i < qtd; i++){
        campos[i] = limpar_campo(campos[i]);
    }
    return qtd;
}

int tem_digito(char* texto){
    for(int i = 0; texto[i] != '\0'; i++){
        if(isdigit((unsigned char)texto[i])) return 1;
    }
    return 0;
}

int contem_palavra(char* texto, char* palavra){
    int tam = strlen(palavra);
    for(int i = 0; texto[i] != '\0'; i++){
        int j = 0;
        while(j < tam && tolower((unsigned char)texto[i + j]) == palavra[j]) j++;
        if(j == tam) return 1;
    }
    return 0;
}

Base carregar_base(char* arquivo, int limite){
    FILE* f = fopen(arquivo, "r");
    if(f == NULL){
        printf("Erro ao abrir o arquivo %s\n", arquivo);
        return NULL;
    }
    Base b = malloc(sizeof(struct base));
    if(b == NULL){
        fclose(f);
        return NULL;
    }
    b->registros = malloc(limite * sizeof(Registro));
    if(b->registros == NULL){
        free(b);
        fclose(f);
        return NULL;
    }
    b->qtd = 0;

    char linha[TAM_LINHA];
    char* campos[MAX_CAMPOS];
    char separador = 0;
    int col_matricula = 0, col_nome = 1, col_curso = 2;
    int primeira_linha = 1;

    while(b->qtd < limite && fgets(linha, TAM_LINHA, f) != NULL){
        remover_quebra(linha);
        char* inicio = linha;
        if(primeira_linha && (unsigned char)inicio[0] == 0xEF) inicio += 3;
        if(inicio[0] == '\0') continue;
        if(separador == 0) separador = descobrir_separador(inicio);

        int qtd_campos = separar_campos(inicio, separador, campos);

        if(primeira_linha){
            primeira_linha = 0;
            if(!tem_digito(campos[0])){
                for(int i = 0; i < qtd_campos; i++){
                    if(contem_palavra(campos[i], "matr")) col_matricula = i;
                    else if(contem_palavra(campos[i], "nome")) col_nome = i;
                    else if(contem_palavra(campos[i], "curso")) col_curso = i;
                }
                continue;
            }
        }
        if(qtd_campos <= col_matricula) continue;

        Registro* r = &b->registros[b->qtd];
        strncpy(r->matricula, campos[col_matricula], TAM_MATRICULA - 1);
        r->matricula[TAM_MATRICULA - 1] = '\0';
        r->nome = copiar_texto(qtd_campos > col_nome ? campos[col_nome] : "");
        r->curso = copiar_texto(qtd_campos > col_curso ? campos[col_curso] : "");
        if(r->nome == NULL || r->curso == NULL){
            printf("Memoria insuficiente ao ler a base\n");
            free(r->nome);
            free(r->curso);
            break;
        }
        b->qtd++;
    }
    fclose(f);
    return b;
}

int tamanho_base(Base b){
    return b->qtd;
}

Registro* acessar_registro(Base b, int posicao){
    return &b->registros[posicao];
}

void destruir_base(Base b){
    if(b == NULL) return;
    for(int i = 0; i < b->qtd; i++){
        free(b->registros[i].nome);
        free(b->registros[i].curso);
    }
    free(b->registros);
    free(b);
}
