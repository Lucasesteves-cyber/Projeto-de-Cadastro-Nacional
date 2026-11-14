#include<stdio.h>
#include<stdlib.h>

#define QTD_NOMES 24
#define QTD_SOBRENOMES 20
#define QTD_CURSOS 12

char* NOMES[QTD_NOMES] = {"Ana", "Bruno", "Carla", "Daniel", "Eduarda", "Felipe", "Gabriela", "Henrique",
                          "Isabela", "Joao", "Larissa", "Lucas", "Mariana", "Pedro", "Rafaela", "Thiago",
                          "Beatriz", "Gustavo", "Julia", "Matheus", "Leticia", "Rodrigo", "Camila", "Vinicius"};
char* SOBRENOMES[QTD_SOBRENOMES] = {"Silva", "Santos", "Oliveira", "Souza", "Rodrigues", "Ferreira", "Alves",
                                    "Pereira", "Lima", "Gomes", "Costa", "Ribeiro", "Martins", "Carvalho",
                                    "Almeida", "Lopes", "Soares", "Fernandes", "Vieira", "Barbosa"};
char* CURSOS[QTD_CURSOS] = {"Ciencia da Computacao", "Engenharia Civil", "Direito", "Medicina",
                            "Administracao", "Psicologia", "Arquitetura", "Engenharia de Software",
                            "Enfermagem", "Pedagogia", "Sistemas de Informacao", "Contabilidade"};

int main(int argc, char* argv[]){
    if(argc < 3){
        printf("Uso: %s <quantidade> <saida.csv>\n", argv[0]);
        return 1;
    }
    long long qtd = atoll(argv[1]);
    FILE* f = fopen(argv[2], "w");
    if(f == NULL){
        printf("Erro ao criar o arquivo %s\n", argv[2]);
        return 1;
    }

    fprintf(f, "matricula,nome,curso\n");
    unsigned long long x = 88172645463325252ULL;
    for(long long i = 0; i < qtd; i++){
        unsigned long long matricula = 1000000000ULL + (i * 2654435761ULL) % 9000000000ULL;
        x = x ^ (x << 13);
        x = x ^ (x >> 7);
        x = x ^ (x << 17);
        fprintf(f, "%llu,%s %s %s,%s\n", matricula, NOMES[x % QTD_NOMES],
                SOBRENOMES[(x >> 8) % QTD_SOBRENOMES], SOBRENOMES[(x >> 16) % QTD_SOBRENOMES],
                CURSOS[(x >> 24) % QTD_CURSOS]);
    }
    fclose(f);
    printf("%lld registros gravados em %s\n", qtd, argv[2]);
    return 0;
}
