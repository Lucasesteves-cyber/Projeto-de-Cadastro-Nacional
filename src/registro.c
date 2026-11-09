#include "registro.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCO_TEXTO (64u * 1024u * 1024u)
#define LINHA_MAX 4096
#define MAX_CAMPOS 32

typedef struct Bloco {
    struct Bloco *prox;
    size_t usado;
    char dados[];
} Bloco;

struct BaseDados {
    Registro *regs;
    size_t n, cap;
    Bloco *blocos;
};

static char *arena_copiar(BaseDados *b, const char *s, size_t len) {
    if (!b->blocos || b->blocos->usado + len + 1 > BLOCO_TEXTO) {
        Bloco *novo = malloc(sizeof(Bloco) + BLOCO_TEXTO);
        if (!novo) return NULL;
        novo->prox = b->blocos;
        novo->usado = 0;
        b->blocos = novo;
    }
    char *dst = b->blocos->dados + b->blocos->usado;
    memcpy(dst, s, len);
    dst[len] = '\0';
    b->blocos->usado += len + 1;
    return dst;
}

static int dividir_campos(char *linha, char sep, char **campos, int max) {
    int n = 0;
    char *p = linha;
    while (n < max) {
        while (*p == ' ') p++;
        if (*p == '"') {
            p++;
            campos[n++] = p;
            char *w = p;
            while (*p) {
                if (*p == '"' && p[1] == '"') { *w++ = '"'; p += 2; }
                else if (*p == '"') { p++; break; }
                else *w++ = *p++;
            }
            while (*p && *p != sep) p++;
            int fim = (*p == '\0');
            *w = '\0';
            if (fim) break;
            p++;
        } else {
            campos[n++] = p;
            char *s = strchr(p, sep);
            if (!s) break;
            *s = '\0';
            p = s + 1;
        }
    }
    for (int i = 0; i < n; i++) {
        size_t len = strlen(campos[i]);
        while (len > 0 && isspace((unsigned char)campos[i][len - 1])) campos[i][--len] = '\0';
    }
    return n;
}

static void tirar_quebra(char *s) {
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r')) s[--len] = '\0';
}

static char detectar_separador(const char *linha) {
    int virg = 0, pv = 0, tab = 0;
    for (const char *p = linha; *p; p++) {
        if (*p == ',') virg++;
        else if (*p == ';') pv++;
        else if (*p == '\t') tab++;
    }
    if (pv >= virg && pv >= tab && pv > 0) return ';';
    if (tab >= virg && tab > 0) return '\t';
    return ',';
}

static int contem_ci(const char *s, const char *sub) {
    size_t ls = strlen(sub);
    for (; *s; s++) {
        size_t i = 0;
        while (i < ls && s[i] && tolower((unsigned char)s[i]) == sub[i]) i++;
        if (i == ls) return 1;
    }
    return 0;
}

static int tem_digito(const char *s) {
    for (; *s; s++) if (isdigit((unsigned char)*s)) return 1;
    return 0;
}

BaseDados *base_carregar(const char *caminho, size_t limite) {
    FILE *f = fopen(caminho, "rb");
    if (!f) { perror(caminho); return NULL; }

    BaseDados *b = calloc(1, sizeof(BaseDados));
    if (!b) { fclose(f); return NULL; }
    b->cap = limite ? limite : 1024;
    b->regs = malloc(b->cap * sizeof(Registro));
    if (!b->regs) { free(b); fclose(f); return NULL; }

    static char linha[LINHA_MAX];
    char *campos[MAX_CAMPOS];
    char sep = 0;
    int col_mat = 0, col_nome = 1, col_curso = 2;
    int primeira = 1;

    while ((limite == 0 || b->n < limite) && fgets(linha, sizeof linha, f)) {
        tirar_quebra(linha);
        char *ini = linha;
        if (primeira && (unsigned char)ini[0] == 0xEF && (unsigned char)ini[1] == 0xBB &&
            (unsigned char)ini[2] == 0xBF)
            ini += 3;
        if (*ini == '\0') continue;
        if (!sep) sep = detectar_separador(ini);

        int nc = dividir_campos(ini, sep, campos, MAX_CAMPOS);

        if (primeira) {
            primeira = 0;
            if (!tem_digito(campos[0])) {
                for (int i = 0; i < nc; i++) {
                    if (contem_ci(campos[i], "matr")) col_mat = i;
                    else if (contem_ci(campos[i], "nome")) col_nome = i;
                    else if (contem_ci(campos[i], "curso")) col_curso = i;
                }
                continue;
            }
        }
        if (nc <= col_mat) continue;

        if (b->n == b->cap) {
            size_t nova = b->cap * 2;
            Registro *r = realloc(b->regs, nova * sizeof(Registro));
            if (!r) { fprintf(stderr, "Memoria insuficiente ao ler a base.\n"); break; }
            b->regs = r;
            b->cap = nova;
        }
        Registro *r = &b->regs[b->n];
        strncpy(r->matricula, campos[col_mat], MAT_MAX - 1);
        r->matricula[MAT_MAX - 1] = '\0';
        const char *nome = nc > col_nome ? campos[col_nome] : "";
        const char *curso = nc > col_curso ? campos[col_curso] : "";
        r->nome = arena_copiar(b, nome, strlen(nome));
        r->curso = arena_copiar(b, curso, strlen(curso));
        if (!r->nome || !r->curso) { fprintf(stderr, "Memoria insuficiente ao ler a base.\n"); break; }
        b->n++;
    }
    fclose(f);
    return b;
}

size_t base_tamanho(const BaseDados *b) { return b->n; }

Registro *base_registro(BaseDados *b, size_t i) { return &b->regs[i]; }

void base_liberar(BaseDados *b) {
    if (!b) return;
    Bloco *bl = b->blocos;
    while (bl) {
        Bloco *p = bl->prox;
        free(bl);
        bl = p;
    }
    free(b->regs);
    free(b);
}
