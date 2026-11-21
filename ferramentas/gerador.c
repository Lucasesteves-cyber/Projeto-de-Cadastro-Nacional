#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define QTD_NOMES 24
#define QTD_SOBRENOMES 20
#define QTD_CURSOS 12

const char* NOMES[QTD_NOMES] = {
    "Ana", "Bruno", "Carla", "Daniel", "Eduarda", "Felipe", "Gabriela", "Henrique",
    "Isabela", "Joao", "Larissa", "Lucas", "Mariana", "Pedro", "Rafaela", "Thiago",
    "Beatriz", "Gustavo", "Julia", "Matheus", "Leticia", "Rodrigo", "Camila", "Vinicius"
};

const char* SOBRENOMES[QTD_SOBRENOMES] = {
    "Silva", "Santos", "Oliveira", "Souza", "Rodrigues", "Ferreira", "Alves",
    "Pereira", "Lima", "Gomes", "Costa", "Ribeiro", "Martins", "Carvalho",
    "Almeida", "Lopes", "Soares", "Fernandes", "Vieira", "Barbosa"
};

const char* CURSOS[QTD_CURSOS] = {
    "Ciencia da Computacao", "Engenharia Civil", "Direito", "Medicina",
    "Administracao", "Psicologia", "Arquitetura", "Engenharia de Software",
    "Enfermagem", "Pedagogia", "Sistemas de Informacao", "Contabilidade"
};

int main(int argc, char* argv[]) {
    int qtd;
    const char* arquivo = "alunos.csv";

    if (argc >= 2) {
        qtd = atoi(argv[1]);
    } else {
        printf("Digite a quantidade de registros: ");
        scanf("%d", &qtd);
    }
    if (argc >= 3) {
        arquivo = argv[2];
    }

    FILE* f = fopen(arquivo, "w");
    if (f == NULL) {
        printf("Erro ao criar o arquivo.\n");
        return 1;
    }

    srand(time(NULL));
    fprintf(f, "matricula,nome,curso\n");

    for (int i = 0; i < qtd; i++) {

        int matricula = 202600000 + (i * 7 + rand() % 7);

        const char* nome = NOMES[rand() % QTD_NOMES];
        const char* sobrenome1 = SOBRENOMES[rand() % QTD_SOBRENOMES];
        const char* sobrenome2 = SOBRENOMES[rand() % QTD_SOBRENOMES];
        const char* curso = CURSOS[rand() % QTD_CURSOS];

        fprintf(f, "%d,%s %s %s,%s\n", matricula, nome, sobrenome1, sobrenome2, curso);
    }

    fclose(f);
    printf("%d registros gravados em %s\n", qtd, arquivo);

    return 0;
}
