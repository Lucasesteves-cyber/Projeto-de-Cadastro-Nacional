# Projeto 2º GQ — Comparação de Estruturas de Dados

Comparação do desempenho de inserção e busca por matrícula em uma base de até
**10 milhões** de registros de estudantes (matrícula, nome, curso), em linguagem C.

## Implementações

| # | Estrutura | Tratamento de colisão | Função hash |
|---|-----------|----------------------|-------------|
| 1 | Lista unicamente encadeada | — | — |
| 2 | Tabela hash — endereçamento aberto | Sondagem linear | Divisão |
| 3 | Tabela hash — endereçamento aberto | Sondagem linear | Polinomial |
| 4 | Tabela hash — encadeamento separado | Listas encadeadas | Divisão |
| 5 | Tabela hash — encadeamento separado | Listas encadeadas | Polinomial |

Todas usam alocação dinâmica (`malloc` por nó) e estão organizadas como TADs, no
mesmo padrão das aulas: o `.h` expõe só o tipo (`typedef struct lista* Lista;`) e
as funções (`criar_lista`, `inserir_inicio`, `buscar_lista`, `destruir_lista`...);
a `struct` fica escondida no `.c`.

```
src/
  registro.h/.c        TAD Registro e leitura da base CSV
  lista.h/.c           Lista unicamente encadeada (inserção no início O(1), busca O(n))
  funcoes_hash.h/.c    Funções hash (divisão e polinomial) e cálculo de primo
  hash_aberto.h/.c     Hash com endereçamento aberto (sondagem linear)
  hash_encadeado.h/.c  Hash com encadeamento separado
  main.c               Execução dos experimentos, cronômetro e geração das tabelas
ferramentas/
  gerador.c            Gera uma base sintética para testes
```

### Funções hash

1. **Método da divisão** — `h(k) = k mod m`. A matrícula é convertida para
   inteiro de 64 bits com `strtoull` (o `atol` não serve: no Windows o `long` tem
   32 bits e estoura com matrículas de 10 dígitos ou mais).
   `m` é primo, como recomendado para esse método.
2. **Polinomial** — método pesquisado. Trata a matrícula como texto e percorre
   seus caracteres, como um polinômio de base 31 (regra de Horner):
   `h = (h × 31 + caractere) mod m`, começando em `h = 0`. Cada dígito, e a
   posição dele, influencia o resultado, então matrículas parecidas tendem a cair
   em posições diferentes.

### Contagem de colisões

Conforme o enunciado, uma colisão é contada quando uma nova matrícula produz um
**índice hash inicial já produzido** por outra matrícula. Cada tabela tem um vetor
`usado[m]` que marca os índices iniciais já gerados. Por isso, para a mesma função
hash, o número de colisões é igual na sondagem e no encadeamento: é uma
propriedade da função, não do tratamento.

### Tamanho da tabela (justificativa)

Para cada volume `N`, as quatro tabelas usam o mesmo `m`, fixo durante todo o experimento:

```
m = menor primo ≥ N / 0,7
```

- No **endereçamento aberto**, `α` precisa ser menor que 1, e o custo médio da
  sondagem linear cresce muito quando `α` passa de ~0,7 a 0,8 (agrupamento primário).
  Com `α` final ≈ **0,70**, as etapas ficam em α ≈ 0,21, 0,42 e 0,70.
- No **encadeamento**, o mesmo `m` dá listas com média de 0,7 elemento.
- **Primo** porque o método da divisão distribui melhor com `m` primo (evita que
  padrões nas matrículas coincidam com divisores de `m`).

| Volume | m |
|--------|---|
| 100.000 | 142.867 |
| 1.000.000 | 1.428.587 |
| 10.000.000 | ≈ 14.285.729 |

O valor alvo é a constante `FATOR_CARGA_ALVO` no `main.c`.

### Busca na lista

Cada nó da lista guarda, além do ponteiro para o registro, a matrícula convertida
para número. Na busca, o `strcmp` só é chamado quando o número bate, o que deixa a
busca sequencial bem mais rápida sem mudar a estrutura (continua O(n)).

### Cronômetro

O tempo é medido com `timespec_get` (padrão do C11, `time.h`), que tem resolução de
microssegundos. O `clock()` não serve aqui: no Windows ele só mede milissegundos, e
10.000 buscas em tabela hash levam por volta de 2 ms.

## Metodologia (conforme o enunciado)

- Para cada volume (100 mil, 1 milhão, 10 milhões), as cinco estruturas começam vazias.
- Os registros são inseridos **na ordem do arquivo**, em três etapas: 0–30%, 30–60% e 60–100%.
  O tempo de cada etapa é medido separadamente.
- Após cada etapa, são sorteadas **10.000 matrículas já inseridas** (`srand` com
  semente fixa, ou seja, reprodutível). Como no Windows o `rand()` só vai até 32.767,
  dois sorteios são combinados (`rand() * (RAND_MAX + 1) + rand()`) para alcançar
  qualquer posição, mesmo com 10 milhões de registros. As **mesmas matrículas, na mesma
  ordem**, são buscadas nas cinco estruturas, e o tempo total é medido.
- Não entram na medição: a leitura do arquivo e o sorteio das matrículas.
- Também são registrados: colisões da etapa, colisões acumuladas e fator de carga `α = n/m`.
- As matrículas são consideradas únicas, como diz o enunciado. Por isso a inserção
  não verifica duplicatas em nenhuma estrutura (mantém a comparação justa e a lista em O(1)).

## Compilação

É preciso ter o `gcc` instalado (no Windows, pelo MinGW/MSYS2). Dentro da pasta do projeto:

```bash
gcc -O2 -std=c11 -Wall -Wextra -Isrc -o comparador src/*.c
```

```bash
gcc -O2 -std=c11 -o gerador ferramentas/gerador.c
```

No Windows são gerados `comparador.exe` e `gerador.exe`.

## Execução

No PowerShell (Windows):

```bash
.\comparador.exe base.csv
```

No Linux/macOS:

```bash
./comparador base.csv
```

Isso roda os três volumes (100 mil, 1 milhão e 10 milhões). Imprime as tabelas na
tela e salva tudo em `resultados/resultados.csv` (a pasta `resultados` precisa existir).

Também dá para escolher os volumes (2º argumento) e o arquivo de saída (3º argumento):

```bash
.\comparador.exe base.csv 100000,1000000 resultados/teste.csv
```

As demais configurações são constantes no início do `main.c`: `NUM_BUSCAS` (10.000),
`FATOR_CARGA_ALVO` (0,7) e `SEMENTE` (2025, usada no sorteio das matrículas).

### Formato da base

CSV com as colunas matrícula, nome e curso. O separador (`,` `;` ou TAB) e o
cabeçalho são detectados sozinhos. Se houver cabeçalho, as colunas são achadas pelo
nome (`matricula`, `nome`, `curso`), então a ordem pode ser outra.

Para testar sem a base oficial:

```bash
.\gerador.exe 1000000 base_teste.csv
```

```bash
.\comparador.exe base_teste.csv 100000,1000000
```

(No Linux/macOS, troque `.\gerador.exe` e `.\comparador.exe` por `./gerador` e `./comparador`.)

### Tempo de execução

A busca na **lista** é O(n). Com 10 milhões de registros, são até 10.000 × 5 milhões
de comparações por etapa, e isso pode levar **horas** (no teste com 1 milhão, as buscas
na lista levaram ~105 s no total). As tabelas hash terminam em segundos. Rode o
experimento completo com antecedência.

## Saída (`resultados.csv`)

```
volume,estrutura,etapa_pct,n,m,tempo_insercao_s,tempo_busca_s,buscas,encontrados,colisoes_etapa,colisoes_acumuladas,fator_carga
```

A coluna `encontrados` deve ser igual a `buscas` (10.000). Ela confirma que todas as
matrículas sorteadas foram achadas.
