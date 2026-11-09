#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "funcoes_hash.h"
#include "hash_aberto.h"
#include "hash_encadeado.h"
#include "lista.h"
#include "registro.h"
#include "temporizador.h"

#define MAX_VOLUMES 8
#define NUM_ETAPAS 3
#define NUM_ESTRUTURAS 5

static const int PERCENTUAIS[NUM_ETAPAS] = {30, 60, 100};

typedef struct {
    const char *nome;
    void *obj;
    int eh_hash;
    int (*inserir)(void *, Registro *);
    Registro *(*buscar)(const void *, const char *);
    size_t (*colisoes)(const void *);
    double (*fator)(const void *);
    void (*liberar)(void *);
} Estrutura;

static int w_lista_ins(void *o, Registro *r) { return lista_inserir(o, r); }
static Registro *w_lista_bus(const void *o, const char *m) { return lista_buscar(o, m); }
static void w_lista_lib(void *o) { lista_liberar(o); }

static int w_ha_ins(void *o, Registro *r) { return ha_inserir(o, r); }
static Registro *w_ha_bus(const void *o, const char *m) { return ha_buscar(o, m); }
static size_t w_ha_col(const void *o) { return ha_colisoes(o); }
static double w_ha_fc(const void *o) { return ha_fator_carga(o); }
static void w_ha_lib(void *o) { ha_liberar(o); }

static int w_he_ins(void *o, Registro *r) { return he_inserir(o, r); }
static Registro *w_he_bus(const void *o, const char *m) { return he_buscar(o, m); }
static size_t w_he_col(const void *o) { return he_colisoes(o); }
static double w_he_fc(const void *o) { return he_fator_carga(o); }
static void w_he_lib(void *o) { he_liberar(o); }

typedef struct {
    size_t n;
    double t_ins, t_bus;
    size_t encontrados;
    size_t col_etapa, col_total;
    double alfa;
    int executado;
} Medida;

static uint64_t rng_estado;
static uint64_t rng_proximo(void) {
    uint64_t z = (rng_estado += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

typedef struct {
    const char *arquivo;
    const char *saida;
    size_t volumes[MAX_VOLUMES];
    int nvol;
    double alfa_alvo;
    size_t buscas;
    size_t lista_max;
    uint64_t semente;
} Opcoes;

static void ajuda(const char *prog) {
    printf("Uso: %s <base.csv> [opcoes]\n\n", prog);
    printf("  -v, --volumes A,B,C   volumes a testar (padrao: 100000,1000000,10000000)\n");
    printf("  -a, --alfa X          fator de carga alvo ao final (define m; padrao 0.7)\n");
    printf("  -b, --buscas N        buscas por etapa (padrao 10000)\n");
    printf("  -s, --semente N       semente do sorteio das matriculas (padrao 2025)\n");
    printf("  -o, --saida ARQ       arquivo CSV de resultados (padrao resultados/resultados.csv)\n");
    printf("  --lista-max N         nao executa a lista para volumes > N (padrao: sempre executa)\n");
    printf("  -h, --ajuda           mostra esta ajuda\n");
}

static int ler_opcoes(int argc, char **argv, Opcoes *op) {
    op->arquivo = NULL;
    op->saida = "resultados/resultados.csv";
    op->volumes[0] = 100000;
    op->volumes[1] = 1000000;
    op->volumes[2] = 10000000;
    op->nvol = 3;
    op->alfa_alvo = 0.7;
    op->buscas = 10000;
    op->lista_max = 0;
    op->semente = 2025;

    for (int i = 1; i < argc; i++) {
        const char *a = argv[i];
        int tem_valor = (i + 1 < argc);
        if (!strcmp(a, "-h") || !strcmp(a, "--ajuda")) { ajuda(argv[0]); exit(0); }
        else if ((!strcmp(a, "-v") || !strcmp(a, "--volumes")) && tem_valor) {
            op->nvol = 0;
            char *copia = argv[++i];
            for (char *tok = strtok(copia, ","); tok && op->nvol < MAX_VOLUMES; tok = strtok(NULL, ","))
                op->volumes[op->nvol++] = (size_t)strtoull(tok, NULL, 10);
        }
        else if ((!strcmp(a, "-a") || !strcmp(a, "--alfa")) && tem_valor) op->alfa_alvo = atof(argv[++i]);
        else if ((!strcmp(a, "-b") || !strcmp(a, "--buscas")) && tem_valor) op->buscas = (size_t)strtoull(argv[++i], NULL, 10);
        else if ((!strcmp(a, "-s") || !strcmp(a, "--semente")) && tem_valor) op->semente = strtoull(argv[++i], NULL, 10);
        else if ((!strcmp(a, "-o") || !strcmp(a, "--saida")) && tem_valor) op->saida = argv[++i];
        else if (!strcmp(a, "--lista-max") && tem_valor) op->lista_max = (size_t)strtoull(argv[++i], NULL, 10);
        else if (a[0] != '-' && !op->arquivo) op->arquivo = a;
        else { fprintf(stderr, "Opcao invalida: %s\n", a); return 0; }
    }
    if (!op->arquivo) { ajuda(argv[0]); return 0; }
    if (op->alfa_alvo <= 0.0 || op->alfa_alvo >= 1.0) {
        fprintf(stderr, "O fator de carga alvo deve estar entre 0 e 1 (enderecamento aberto exige n < m).\n");
        return 0;
    }
    return 1;
}

static int criar_estruturas(Estrutura e[NUM_ESTRUTURAS], size_t m, int usar_lista) {
    memset(e, 0, sizeof(Estrutura) * NUM_ESTRUTURAS);

    e[0] = (Estrutura){"Lista encadeada", usar_lista ? lista_criar() : NULL, 0,
                       w_lista_ins, w_lista_bus, NULL, NULL, w_lista_lib};
    e[1] = (Estrutura){"Hash aberto + Divisao", ha_criar(m, hash_divisao), 1,
                       w_ha_ins, w_ha_bus, w_ha_col, w_ha_fc, w_ha_lib};
    e[2] = (Estrutura){"Hash aberto + FNV-1a", ha_criar(m, hash_fnv1a), 1,
                       w_ha_ins, w_ha_bus, w_ha_col, w_ha_fc, w_ha_lib};
    e[3] = (Estrutura){"Encadeado + Divisao", he_criar(m, hash_divisao), 1,
                       w_he_ins, w_he_bus, w_he_col, w_he_fc, w_he_lib};
    e[4] = (Estrutura){"Encadeado + FNV-1a", he_criar(m, hash_fnv1a), 1,
                       w_he_ins, w_he_bus, w_he_col, w_he_fc, w_he_lib};

    for (int k = 1; k < NUM_ESTRUTURAS; k++)
        if (!e[k].obj) return 0;
    if (usar_lista && !e[0].obj) return 0;
    return 1;
}

static void liberar_estruturas(Estrutura e[NUM_ESTRUTURAS]) {
    for (int k = 0; k < NUM_ESTRUTURAS; k++)
        if (e[k].obj) { e[k].liberar(e[k].obj); e[k].obj = NULL; }
}

static void imprimir_tabela(size_t volume, size_t m, Estrutura e[NUM_ESTRUTURAS],
                            Medida med[NUM_ESTRUTURAS][NUM_ETAPAS], size_t buscas) {
    printf("\n=== Volume: %zu registros | tamanho da tabela m = %zu ===\n", volume, m);
    printf("%-24s %5s %10s %14s %16s %14s %14s %8s\n", "Estrutura", "Etapa", "n",
           "Insercao (s)", "Busca (s)", "Colis. etapa", "Colis. total", "alfa");
    for (int k = 0; k < NUM_ESTRUTURAS; k++) {
        for (int s = 0; s < NUM_ETAPAS; s++) {
            Medida *x = &med[k][s];
            if (!x->executado) {
                printf("%-24s %4d%% %10s %14s %16s\n", e[k].nome, PERCENTUAIS[s], "-", "(pulado)", "-");
                continue;
            }
            printf("%-24s %4d%% %10zu %14.6f %16.6f", e[k].nome, PERCENTUAIS[s], x->n, x->t_ins, x->t_bus);
            if (e[k].eh_hash)
                printf(" %14zu %14zu %8.4f", x->col_etapa, x->col_total, x->alfa);
            else
                printf(" %14s %14s %8s", "-", "-", "-");
            if (x->encontrados != buscas) printf("  [!] achou %zu/%zu", x->encontrados, buscas);
            printf("\n");
        }
    }
    fflush(stdout);
}

static int executar_volume(BaseDados *base, size_t volume, const Opcoes *op, FILE *csv) {
    size_t m = proximo_primo((size_t)((double)volume / op->alfa_alvo) + 1);
    int usar_lista = (op->lista_max == 0 || volume <= op->lista_max);

    Estrutura e[NUM_ESTRUTURAS];
    if (!criar_estruturas(e, m, usar_lista)) {
        fprintf(stderr, "Memoria insuficiente para criar as estruturas (volume %zu).\n", volume);
        liberar_estruturas(e);
        return 0;
    }

    Medida med[NUM_ESTRUTURAS][NUM_ETAPAS];
    memset(med, 0, sizeof med);
    const char **chaves = malloc(op->buscas * sizeof(char *));
    if (!chaves) { liberar_estruturas(e); return 0; }

    size_t inicio = 0;
    for (int s = 0; s < NUM_ETAPAS; s++) {
        size_t fim = (size_t)((unsigned long long)volume * PERCENTUAIS[s] / 100ULL);
        printf("  [%zu] etapa %d%%: inserindo %zu..%zu\n", volume, PERCENTUAIS[s], inicio, fim);
        fflush(stdout);

        for (int k = 0; k < NUM_ESTRUTURAS; k++) {
            if (!e[k].obj) continue;
            size_t col_antes = e[k].eh_hash ? e[k].colisoes(e[k].obj) : 0;
            double t0 = agora_segundos();
            for (size_t i = inicio; i < fim; i++) {
                if (!e[k].inserir(e[k].obj, base_registro(base, i))) {
                    fprintf(stderr, "Falha ao inserir em '%s' (memoria/tabela cheia).\n", e[k].nome);
                    free(chaves);
                    liberar_estruturas(e);
                    return 0;
                }
            }
            double t1 = agora_segundos();
            med[k][s].t_ins = t1 - t0;
            med[k][s].n = fim;
            med[k][s].executado = 1;
            if (e[k].eh_hash) {
                med[k][s].col_total = e[k].colisoes(e[k].obj);
                med[k][s].col_etapa = med[k][s].col_total - col_antes;
                med[k][s].alfa = e[k].fator(e[k].obj);
            }
        }

        rng_estado = op->semente ^ ((uint64_t)volume * 1000003ULL) ^ (uint64_t)PERCENTUAIS[s];
        for (size_t j = 0; j < op->buscas; j++)
            chaves[j] = base_registro(base, (size_t)(rng_proximo() % fim))->matricula;

        for (int k = 0; k < NUM_ESTRUTURAS; k++) {
            if (!e[k].obj) continue;
            if (k == 0) { printf("  [%zu] etapa %d%%: buscando na lista (pode demorar)...\n", volume, PERCENTUAIS[s]); fflush(stdout); }
            size_t achou = 0;
            double t0 = agora_segundos();
            for (size_t j = 0; j < op->buscas; j++)
                if (e[k].buscar(e[k].obj, chaves[j])) achou++;
            double t1 = agora_segundos();
            med[k][s].t_bus = t1 - t0;
            med[k][s].encontrados = achou;
        }
        inicio = fim;
    }

    imprimir_tabela(volume, m, e, med, op->buscas);

    if (csv) {
        for (int k = 0; k < NUM_ESTRUTURAS; k++)
            for (int s = 0; s < NUM_ETAPAS; s++) {
                Medida *x = &med[k][s];
                if (!x->executado) continue;
                fprintf(csv, "%zu,%s,%d,%zu,%zu,%.6f,%.6f,%zu,%zu,", volume, e[k].nome, PERCENTUAIS[s],
                        x->n, e[k].eh_hash ? m : 0, x->t_ins, x->t_bus, op->buscas, x->encontrados);
                if (e[k].eh_hash) fprintf(csv, "%zu,%zu,%.6f\n", x->col_etapa, x->col_total, x->alfa);
                else fprintf(csv, ",,\n");
            }
        fflush(csv);
    }

    free(chaves);
    liberar_estruturas(e);
    return 1;
}

int main(int argc, char **argv) {
    Opcoes op;
    if (!ler_opcoes(argc, argv, &op)) return 1;

    size_t maior = 0;
    for (int v = 0; v < op.nvol; v++)
        if (op.volumes[v] > maior) maior = op.volumes[v];

    printf("Lendo ate %zu registros de '%s'...\n", maior, op.arquivo);
    fflush(stdout);
    double t0 = agora_segundos();
    BaseDados *base = base_carregar(op.arquivo, maior);
    if (!base) return 1;
    printf("%zu registros carregados em %.2f s (tempo de leitura NAO entra nas medicoes).\n",
           base_tamanho(base), agora_segundos() - t0);
    if (base_tamanho(base) == 0) { base_liberar(base); return 1; }

    FILE *csv = fopen(op.saida, "w");
    if (!csv) fprintf(stderr, "Aviso: nao foi possivel criar '%s'; resultados so na tela.\n", op.saida);
    else fprintf(csv, "volume,estrutura,etapa_pct,n,m,tempo_insercao_s,tempo_busca_s,buscas,encontrados,"
                      "colisoes_etapa,colisoes_acumuladas,fator_carga\n");

    for (int v = 0; v < op.nvol; v++) {
        size_t vol = op.volumes[v];
        if (vol > base_tamanho(base)) {
            fprintf(stderr, "Aviso: a base tem so %zu registros; volume %zu ajustado.\n", base_tamanho(base), vol);
            vol = base_tamanho(base);
        }
        if (!executar_volume(base, vol, &op, csv)) break;
    }

    if (csv) { fclose(csv); printf("\nResultados salvos em %s\n", op.saida); }
    base_liberar(base);
    return 0;
}
