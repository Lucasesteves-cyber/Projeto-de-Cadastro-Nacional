#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static const char *NOMES[] = {"Ana", "Bruno", "Carla", "Daniel", "Eduarda", "Felipe", "Gabriela", "Henrique",
                              "Isabela", "Joao", "Larissa", "Lucas", "Mariana", "Pedro", "Rafaela", "Thiago",
                              "Beatriz", "Gustavo", "Julia", "Matheus", "Leticia", "Rodrigo", "Camila", "Vinicius"};
static const char *SOBRENOMES[] = {"Silva", "Santos", "Oliveira", "Souza", "Rodrigues", "Ferreira", "Alves",
                                   "Pereira", "Lima", "Gomes", "Costa", "Ribeiro", "Martins", "Carvalho",
                                   "Almeida", "Lopes", "Soares", "Fernandes", "Vieira", "Barbosa"};
static const char *CURSOS[] = {"Ciencia da Computacao", "Engenharia Civil", "Direito", "Medicina",
                               "Administracao", "Psicologia", "Arquitetura", "Engenharia de Software",
                               "Enfermagem", "Pedagogia", "Sistemas de Informacao", "Contabilidade"};

#define TAM(v) (sizeof(v) / sizeof((v)[0]))

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <quantidade> <saida.csv>\n", argv[0]);
        return 1;
    }
    unsigned long long n = strtoull(argv[1], NULL, 10);
    FILE *f = fopen(argv[2], "w");
    if (!f) { perror(argv[2]); return 1; }

    fprintf(f, "matricula,nome,curso\n");
    uint64_t x = 88172645463325252ULL;
    for (unsigned long long i = 0; i < n; i++) {
        uint64_t mat = 1000000000ULL + (i * 2654435761ULL) % 9000000000ULL;
        x ^= x << 13; x ^= x >> 7; x ^= x << 17;
        fprintf(f, "%llu,%s %s %s,%s\n", (unsigned long long)mat, NOMES[x % TAM(NOMES)],
                SOBRENOMES[(x >> 8) % TAM(SOBRENOMES)], SOBRENOMES[(x >> 16) % TAM(SOBRENOMES)],
                CURSOS[(x >> 24) % TAM(CURSOS)]);
    }
    fclose(f);
    printf("%llu registros gravados em %s\n", n, argv[2]);
    return 0;
}
