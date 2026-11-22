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

Sem nenhum argumento, o programa lê `alunos.csv` (o arquivo que o gerador cria por padrão).

### Formato da base

CSV com uma linha por aluno, nesta ordem: **matrícula, nome, curso**. Exemplo:

```
matricula,nome,curso
202600002,Vinicius Almeida Fernandes,Contabilidade
```

- O separador (`,`, `;` ou TAB) é detectado na primeira linha.
- O cabeçalho é opcional: a primeira linha só é pulada se não começar com dígito.
- Cada linha pode ter até 255 caracteres. Linhas vazias ou com menos de três campos são ignoradas.

### Base de teste (gerador)

Enquanto a base oficial não está disponível, o `gerador` cria uma base sintética:

```bash
.\gerador.exe 1000000 base_teste.csv
```

```bash
.\comparador.exe base_teste.csv 100000,1000000
```

Sem argumentos, o gerador pergunta a quantidade e grava em `alunos.csv`. As matrículas
seguem o formato `2026xxxxx`, são únicas e crescentes (`202600000 + 7·i + sorteio de 0 a 6`),
e nomes e cursos são sorteados de listas fixas. O gerador usa `srand(time(NULL))`, então
cada base gerada é diferente. Os arquivos `.csv` da raiz do projeto ficam fora do Git
(`.gitignore`), porque são grandes e podem ser recriados.

(No Linux/macOS, troque `.\gerador.exe` e `.\comparador.exe` por `./gerador` e `./comparador`.)

### Tempo de execução

A busca na **lista** é O(n). Com 10 milhões de registros, são até 10.000 × 5 milhões
de comparações por etapa, e isso pode levar **horas** (no teste com 1 milhão, as buscas
na lista levaram cerca de 2 minutos no total). As tabelas hash terminam em segundos. Rode o
experimento completo com antecedência.

## Saída (`resultados.csv`)

```
volume,estrutura,etapa_pct,n,m,tempo_insercao_s,tempo_busca_s,buscas,encontrados,colisoes_etapa,colisoes_acumuladas,fator_carga
```

A coluna `encontrados` deve ser igual a `buscas` (10.000). Ela confirma que todas as
matrículas sorteadas foram achadas.

## Resultados (base de teste)

Teste com a base gerada pelo `gerador` (1 milhão de registros), volumes de 100 mil e
1 milhão. Os dados completos estão em [`resultados/teste.csv`](resultados/teste.csv).

**Volume de 1.000.000 registros (m = 1.428.587)**

| Estrutura | Etapa | Inserção (s) | Busca, 10.000 (s) | Colisões etapa | Colisões total | α |
|-----------|------:|-------------:|------------------:|---------------:|---------------:|---:|
| Lista encadeada | 30% | 0,0205 | 17,2618 | — | — | — |
| Lista encadeada | 60% | 0,0219 | 43,3908 | — | — | — |
| Lista encadeada | 100% | 0,0316 | 66,0476 | — | — | — |
| Hash aberto + Divisão | 30% | 0,0120 | 0,0017 | 13.672 | 13.672 | 0,21 |
| Hash aberto + Divisão | 60% | 0,0148 | 0,0017 | 65.962 | 79.634 | 0,42 |
| Hash aberto + Divisão | 100% | 0,0224 | 0,0022 | 163.876 | 243.510 | 0,70 |
| Hash aberto + Polinomial | 30% | 0,0227 | 0,0020 | 46.942 | 46.942 | 0,21 |
| Hash aberto + Polinomial | 60% | 0,0287 | 0,0018 | 90.674 | 137.616 | 0,42 |
| Hash aberto + Polinomial | 100% | 0,0321 | 0,0026 | 173.426 | 311.042 | 0,70 |
| Encadeado + Divisão | 30% | 0,0222 | 0,0036 | 13.672 | 13.672 | 0,21 |
| Encadeado + Divisão | 60% | 0,0248 | 0,0028 | 65.962 | 79.634 | 0,42 |
| Encadeado + Divisão | 100% | 0,0360 | 0,0032 | 163.876 | 243.510 | 0,70 |
| Encadeado + Polinomial | 30% | 0,0322 | 0,0031 | 46.942 | 46.942 | 0,21 |
| Encadeado + Polinomial | 60% | 0,0412 | 0,0026 | 90.674 | 137.616 | 0,42 |
| Encadeado + Polinomial | 100% | 0,0453 | 0,0034 | 173.426 | 311.042 | 0,70 |

**O que o teste mostra**

- **Lista x hash:** com 1 milhão de registros, as 10.000 buscas levam cerca de 66 s na lista
  e cerca de 2 ms nas tabelas hash, uma diferença de mais de 20.000 vezes. O tempo da lista
  cresce junto com o número de registros (O(n)); o das tabelas hash quase não muda (O(1) em média).
- **Inserção:** é rápida em todas as estruturas, porque a lista insere no início e as
  tabelas hash calculam a posição direto.
- **Divisão x Polinomial:** nesta base, a Divisão gerou menos colisões (243.510 contra 311.042).
  As matrículas do gerador são números crescentes com passo quase fixo, e `k mod m` com `m` primo
  espalha bem esse tipo de sequência. A Polinomial trata a matrícula como texto e se comporta
  mais como uma distribuição aleatória.
- **Aberto x encadeado:** as colisões são iguais para a mesma função, porque dependem só do índice
  inicial. O hash aberto foi um pouco mais rápido nas buscas, já que percorre um vetor contínuo
  na memória, enquanto o encadeado segue ponteiros para nós espalhados.

> Esses números são da base sintética. O experimento oficial (100 mil, 1 milhão e
> 10 milhões, com a base disponibilizada) deve ser rodado com `.\comparador.exe base.csv`,
> e os resultados ficam em `resultados/resultados.csv`.
