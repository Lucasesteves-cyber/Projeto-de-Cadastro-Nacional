#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include"registro.h"
#include"lista.h"
#include"hash_aberto.h"
#include"hash_encadeado.h"
#include"funcoes_hash.h"

#define NUM_BUSCAS 10000
#define FATOR_CARGA_ALVO 0.7
#define NUM_ETAPAS 3
#define NUM_ESTRUTURAS 5
#define MAX_VOLUMES 8
#define SEMENTE 2025

#define LISTA 0
#define ABERTO_DIVISAO 1
#define ABERTO_FNV1A 2
#define ENCADEADO_DIVISAO 3
#define ENCADEADO_FNV1A 4

char* NOMES[NUM_ESTRUTURAS] = {
    "Lista encadeada",
    "Hash aberto + Divisao",
    "Hash aberto + FNV-1a",
    "Encadeado + Divisao",
    "Encadeado + FNV-1a"
};
int PORCENTAGENS[NUM_ETAPAS] = {30, 60, 100};

struct medida{
    int qtd;
    double tempo_insercao;
    double tempo_busca;
    int encontrados;
    int colisoes_etapa;
    int colisoes_total;
    double fator_carga;
};
typedef struct medida Medida;

double tempo_atual(){
    struct timespec t;
    timespec_get(&t, TIME_UTC);
    return t.tv_sec + t.tv_nsec / 1e9;
}

unsigned long long estado_aleatorio;

unsigned long long proximo_aleatorio(){
    unsigned long long z = (estado_aleatorio += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

void sortear_matriculas(Base b, int qtd_inseridos, char** chaves, unsigned long long semente){
    estado_aleatorio = semente;
    for(int i = 0; i < NUM_BUSCAS; i++){
        int posicao = proximo_aleatorio() % qtd_inseridos;
        chaves[i] = acessar_registro(b, posicao)->matricula;
    }
}

double inserir_na_lista(Lista li, Base b, int inicio, int fim){
    double t0 = tempo_atual();
    for(int i = inicio; i < fim; i++){
        if(!inserir_inicio(li, acessar_registro(b, i))) return -1;
    }
    return tempo_atual() - t0;
}

double inserir_no_aberto(HashAberto ha, Base b, int inicio, int fim){
    double t0 = tempo_atual();
    for(int i = inicio; i < fim; i++){
        if(!inserir_hash_aberto(ha, acessar_registro(b, i))) return -1;
    }
    return tempo_atual() - t0;
}

double inserir_no_encadeado(HashEncadeado he, Base b, int inicio, int fim){
    double t0 = tempo_atual();
    for(int i = inicio; i < fim; i++){
        if(!inserir_hash_encadeado(he, acessar_registro(b, i))) return -1;
    }
    return tempo_atual() - t0;
}

double buscar_na_lista(Lista li, char** chaves, int* encontrados){
    *encontrados = 0;
    double t0 = tempo_atual();
    for(int i = 0; i < NUM_BUSCAS; i++){
        if(buscar_lista(li, chaves[i]) != NULL) (*encontrados)++;
    }
    return tempo_atual() - t0;
}

double buscar_no_aberto(HashAberto ha, char** chaves, int* encontrados){
    *encontrados = 0;
    double t0 = tempo_atual();
    for(int i = 0; i < NUM_BUSCAS; i++){
        if(buscar_hash_aberto(ha, chaves[i]) != NULL) (*encontrados)++;
    }
    return tempo_atual() - t0;
}

double buscar_no_encadeado(HashEncadeado he, char** chaves, int* encontrados){
    *encontrados = 0;
    double t0 = tempo_atual();
    for(int i = 0; i < NUM_BUSCAS; i++){
        if(buscar_hash_encadeado(he, chaves[i]) != NULL) (*encontrados)++;
    }
    return tempo_atual() - t0;
}

void imprimir_resultados(int volume, int m, Medida med[NUM_ESTRUTURAS][NUM_ETAPAS]){
    printf("\n=== Volume: %d registros | tamanho da tabela m = %d ===\n", volume, m);
    printf("%-24s %5s %10s %14s %14s %14s %14s %8s\n", "Estrutura", "Etapa", "n",
           "Insercao (s)", "Busca (s)", "Colis. etapa", "Colis. total", "alfa");
    for(int e = 0; e < NUM_ESTRUTURAS; e++){
        for(int s = 0; s < NUM_ETAPAS; s++){
            Medida x = med[e][s];
            printf("%-24s %4d%% %10d %14.6f %14.6f", NOMES[e], PORCENTAGENS[s], x.qtd,
                   x.tempo_insercao, x.tempo_busca);
            if(e == LISTA)
                printf(" %14s %14s %8s", "-", "-", "-");
            else
                printf(" %14d %14d %8.4f", x.colisoes_etapa, x.colisoes_total, x.fator_carga);
            if(x.encontrados != NUM_BUSCAS)
                printf("  [!] achou %d de %d", x.encontrados, NUM_BUSCAS);
            printf("\n");
        }
    }
}

void salvar_resultados(FILE* csv, int volume, int m, Medida med[NUM_ESTRUTURAS][NUM_ETAPAS]){
    for(int e = 0; e < NUM_ESTRUTURAS; e++){
        for(int s = 0; s < NUM_ETAPAS; s++){
            Medida x = med[e][s];
            fprintf(csv, "%d,%s,%d,%d,%d,%.6f,%.6f,%d,%d,", volume, NOMES[e], PORCENTAGENS[s],
                    x.qtd, e == LISTA ? 0 : m, x.tempo_insercao, x.tempo_busca, NUM_BUSCAS, x.encontrados);
            if(e == LISTA)
                fprintf(csv, ",,\n");
            else
                fprintf(csv, "%d,%d,%.6f\n", x.colisoes_etapa, x.colisoes_total, x.fator_carga);
        }
    }
}

int executar_volume(Base b, int volume, FILE* csv){
    int m = proximo_primo((int)(volume / FATOR_CARGA_ALVO) + 1);

    Lista li = criar_lista();
    HashAberto ha[2];
    HashEncadeado he[2];
    ha[DIVISAO] = criar_hash_aberto(m, DIVISAO);
    ha[FNV1A] = criar_hash_aberto(m, FNV1A);
    he[DIVISAO] = criar_hash_encadeado(m, DIVISAO);
    he[FNV1A] = criar_hash_encadeado(m, FNV1A);
    char** chaves = malloc(NUM_BUSCAS * sizeof(char*));

    int ok = li != NULL && chaves != NULL;
    for(int h = 0; h < 2; h++){
        if(ha[h] == NULL || he[h] == NULL) ok = 0;
    }

    Medida med[NUM_ESTRUTURAS][NUM_ETAPAS];
    memset(med, 0, sizeof(med));
    int inicio = 0;

    for(int s = 0; s < NUM_ETAPAS && ok; s++){
        int fim = (int)((long long)volume * PORCENTAGENS[s] / 100);
        printf("  [%d] etapa %d%%: inserindo registros %d a %d\n", volume, PORCENTAGENS[s], inicio, fim);

        med[LISTA][s].tempo_insercao = inserir_na_lista(li, b, inicio, fim);
        for(int h = 0; h < 2; h++){
            med[ABERTO_DIVISAO + h][s].tempo_insercao = inserir_no_aberto(ha[h], b, inicio, fim);
            med[ENCADEADO_DIVISAO + h][s].tempo_insercao = inserir_no_encadeado(he[h], b, inicio, fim);
        }
        for(int e = 0; e < NUM_ESTRUTURAS; e++){
            if(med[e][s].tempo_insercao < 0) ok = 0;
            med[e][s].qtd = fim;
        }
        if(!ok) break;

        for(int h = 0; h < 2; h++){
            Medida* a = &med[ABERTO_DIVISAO + h][s];
            Medida* c = &med[ENCADEADO_DIVISAO + h][s];
            a->colisoes_total = colisoes_hash_aberto(ha[h]);
            a->fator_carga = fator_carga_hash_aberto(ha[h]);
            c->colisoes_total = colisoes_hash_encadeado(he[h]);
            c->fator_carga = fator_carga_hash_encadeado(he[h]);
            int anterior_a = s > 0 ? med[ABERTO_DIVISAO + h][s - 1].colisoes_total : 0;
            int anterior_c = s > 0 ? med[ENCADEADO_DIVISAO + h][s - 1].colisoes_total : 0;
            a->colisoes_etapa = a->colisoes_total - anterior_a;
            c->colisoes_etapa = c->colisoes_total - anterior_c;
        }

        sortear_matriculas(b, fim, chaves, SEMENTE + (unsigned long long)volume * 1000 + s);

        printf("  [%d] etapa %d%%: buscando (a lista pode demorar)\n", volume, PORCENTAGENS[s]);
        med[LISTA][s].tempo_busca = buscar_na_lista(li, chaves, &med[LISTA][s].encontrados);
        for(int h = 0; h < 2; h++){
            med[ABERTO_DIVISAO + h][s].tempo_busca =
                buscar_no_aberto(ha[h], chaves, &med[ABERTO_DIVISAO + h][s].encontrados);
            med[ENCADEADO_DIVISAO + h][s].tempo_busca =
                buscar_no_encadeado(he[h], chaves, &med[ENCADEADO_DIVISAO + h][s].encontrados);
        }
        inicio = fim;
    }

    if(ok){
        imprimir_resultados(volume, m, med);
        if(csv != NULL) salvar_resultados(csv, volume, m, med);
    }else{
        printf("Memoria insuficiente para o volume %d\n", volume);
    }

    free(chaves);
    destruir_lista(li);
    for(int h = 0; h < 2; h++){
        destruir_hash_aberto(ha[h]);
        destruir_hash_encadeado(he[h]);
    }
    return ok;
}

int ler_volumes(char* texto, int* volumes){
    int qtd = 0;
    char* parte = strtok(texto, ",");
    while(parte != NULL && qtd < MAX_VOLUMES){
        volumes[qtd] = atoi(parte);
        if(volumes[qtd] > 0) qtd++;
        parte = strtok(NULL, ",");
    }
    return qtd;
}

int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Uso: %s <base.csv> [volumes] [saida.csv]\n", argv[0]);
        printf("Exemplo: %s base.csv 100000,1000000,10000000 resultados/resultados.csv\n", argv[0]);
        return 1;
    }

    int volumes[MAX_VOLUMES] = {100000, 1000000, 10000000};
    int qtd_volumes = 3;
    if(argc >= 3) qtd_volumes = ler_volumes(argv[2], volumes);
    char* saida = argc >= 4 ? argv[3] : "resultados/resultados.csv";

    int maior = 0;
    for(int v = 0; v < qtd_volumes; v++){
        if(volumes[v] > maior) maior = volumes[v];
    }
    if(maior == 0){
        printf("Nenhum volume valido\n");
        return 1;
    }

    printf("Lendo ate %d registros de %s\n", maior, argv[1]);
    Base b = carregar_base(argv[1], maior);
    if(b == NULL) return 1;
    printf("%d registros carregados\n", tamanho_base(b));
    if(tamanho_base(b) == 0){
        destruir_base(b);
        return 1;
    }

    FILE* csv = fopen(saida, "w");
    if(csv == NULL)
        printf("Aviso: nao foi possivel criar %s, resultados so na tela\n", saida);
    else
        fprintf(csv, "volume,estrutura,etapa_pct,n,m,tempo_insercao_s,tempo_busca_s,buscas,encontrados,"
                     "colisoes_etapa,colisoes_acumuladas,fator_carga\n");

    for(int v = 0; v < qtd_volumes; v++){
        int volume = volumes[v];
        if(volume > tamanho_base(b)){
            printf("Aviso: a base tem so %d registros, volume %d ajustado\n", tamanho_base(b), volume);
            volume = tamanho_base(b);
        }
        if(!executar_volume(b, volume, csv)) break;
    }

    if(csv != NULL){
        fclose(csv);
        printf("\nResultados salvos em %s\n", saida);
    }
    destruir_base(b);
    return 0;
}
