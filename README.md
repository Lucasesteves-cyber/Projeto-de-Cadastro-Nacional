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
| 10.000.000 | 14.285.717 |

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

O enunciado não fornece uma base de dados, então o `gerador` cria a base usada no
experimento. A base dos resultados abaixo tem 10 milhões de registros (~454 MB):

```bash
.\gerador.exe 10000000 base_10m.csv
```

```bash
.\comparador.exe base_10m.csv 100000,1000000,10000000 resultados\resultados.csv
```

Os volumes de 100 mil e 1 milhão usam os primeiros registros da mesma base.

Sem argumentos, o gerador pergunta a quantidade e grava em `alunos.csv`. As matrículas
seguem o formato `2026xxxxx`, são únicas e crescentes (`202600000 + 7·i + sorteio de 0 a 6`),
e nomes e cursos são sorteados de listas fixas. O gerador usa `srand(time(NULL))`, então
cada base gerada é diferente. Os arquivos `.csv` da raiz do projeto ficam fora do Git
(`.gitignore`), porque são grandes e podem ser recriados.

(No Linux/macOS, troque `.\gerador.exe` e `.\comparador.exe` por `./gerador` e `./comparador`.)

### Tempo de execução

A busca na **lista** é O(n). Com 10 milhões de registros, são até 10.000 × 5 milhões
de comparações por etapa. No experimento, as buscas na lista levaram cerca de 100 s no
volume de 1 milhão e cerca de **17 minutos** no de 10 milhões. As tabelas hash terminam em
segundos. A execução completa (três volumes) leva por volta de 20 a 25 minutos e usa alguns GB de RAM.

## Saída (`resultados.csv`)

```
volume,estrutura,etapa_pct,n,m,tempo_insercao_s,tempo_busca_s,buscas,encontrados,colisoes_etapa,colisoes_acumuladas,fator_carga
```

A coluna `encontrados` deve ser igual a `buscas` (10.000). Ela confirma que todas as
matrículas sorteadas foram achadas.

## Resultados

Experimento com a base de 10 milhões gerada pelo `gerador`, nos volumes de 100 mil,
1 milhão e 10 milhões. Os dados completos estão em
[`resultados/resultados.csv`](resultados/resultados.csv). Tempos em segundos; a busca
é o total das 10.000 buscas de cada etapa.

**Volume de 100.000 registros (m = 142.867)**

| Estrutura | Etapa | Inserção (s) | Busca (s) | Colisões etapa | Colisões total | α |
|-----------|------:|-------------:|----------:|---------------:|---------------:|---:|
| Lista encadeada | 30% | 0,0024 | 0,2799 | — | — | — |
| Lista encadeada | 60% | 0,0024 | 0,8876 | — | — | — |
| Lista encadeada | 100% | 0,0036 | 1,5932 | — | — | — |
| Hash aberto + Divisão | 30% | 0,0012 | 0,0012 | 1.401 | 1.401 | 0,21 |
| Hash aberto + Divisão | 60% | 0,0013 | 0,0030 | 6.683 | 8.084 | 0,42 |
| Hash aberto + Divisão | 100% | 0,0020 | 0,0017 | 16.280 | 24.364 | 0,70 |
| Hash aberto + Polinomial | 30% | 0,0013 | 0,0009 | 1.824 | 1.824 | 0,21 |
| Hash aberto + Polinomial | 60% | 0,0014 | 0,0017 | 7.032 | 8.856 | 0,42 |
| Hash aberto + Polinomial | 100% | 0,0027 | 0,0014 | 16.618 | 25.474 | 0,70 |
| Encadeado + Divisão | 30% | 0,0022 | 0,0015 | 1.401 | 1.401 | 0,21 |
| Encadeado + Divisão | 60% | 0,0023 | 0,0042 | 6.683 | 8.084 | 0,42 |
| Encadeado + Divisão | 100% | 0,0046 | 0,0026 | 16.280 | 24.364 | 0,70 |
| Encadeado + Polinomial | 30% | 0,0029 | 0,0019 | 1.824 | 1.824 | 0,21 |
| Encadeado + Polinomial | 60% | 0,0029 | 0,0022 | 7.032 | 8.856 | 0,42 |
| Encadeado + Polinomial | 100% | 0,0067 | 0,0020 | 16.618 | 25.474 | 0,70 |

**Volume de 1.000.000 registros (m = 1.428.587)**

| Estrutura | Etapa | Inserção (s) | Busca (s) | Colisões etapa | Colisões total | α |
|-----------|------:|-------------:|----------:|---------------:|---------------:|---:|
| Lista encadeada | 30% | 0,0215 | 14,1905 | — | — | — |
| Lista encadeada | 60% | 0,0221 | 31,9675 | — | — | — |
| Lista encadeada | 100% | 0,0296 | 52,7673 | — | — | — |
| Hash aberto + Divisão | 30% | 0,0132 | 0,0015 | 13.702 | 13.702 | 0,21 |
| Hash aberto + Divisão | 60% | 0,0127 | 0,0017 | 66.561 | 80.263 | 0,42 |
| Hash aberto + Divisão | 100% | 0,0195 | 0,0020 | 162.648 | 242.911 | 0,70 |
| Hash aberto + Polinomial | 30% | 0,0216 | 0,0013 | 47.011 | 47.011 | 0,21 |
| Hash aberto + Polinomial | 60% | 0,0203 | 0,0019 | 90.376 | 137.387 | 0,42 |
| Hash aberto + Polinomial | 100% | 0,0321 | 0,0026 | 173.040 | 310.427 | 0,70 |
| Encadeado + Divisão | 30% | 0,0224 | 0,0024 | 13.702 | 13.702 | 0,21 |
| Encadeado + Divisão | 60% | 0,0236 | 0,0028 | 66.561 | 80.263 | 0,42 |
| Encadeado + Divisão | 100% | 0,0316 | 0,0033 | 162.648 | 242.911 | 0,70 |
| Encadeado + Polinomial | 30% | 0,0361 | 0,0023 | 47.011 | 47.011 | 0,21 |
| Encadeado + Polinomial | 60% | 0,0308 | 0,0026 | 90.376 | 137.387 | 0,42 |
| Encadeado + Polinomial | 100% | 0,0411 | 0,0030 | 173.040 | 310.427 | 0,70 |

**Volume de 10.000.000 registros (m = 14.285.717)**

| Estrutura | Etapa | Inserção (s) | Busca (s) | Colisões etapa | Colisões total | α |
|-----------|------:|-------------:|----------:|---------------:|---------------:|---:|
| Lista encadeada | 30% | 0,2158 | 161,6944 | — | — | — |
| Lista encadeada | 60% | 0,2231 | 333,5586 | — | — | — |
| Lista encadeada | 100% | 0,3106 | 555,9513 | — | — | — |
| Hash aberto + Divisão | 30% | 0,1201 | 0,0021 | 136.724 | 136.724 | 0,21 |
| Hash aberto + Divisão | 60% | 0,1354 | 0,0031 | 663.305 | 800.029 | 0,42 |
| Hash aberto + Divisão | 100% | 0,2120 | 0,0029 | 1.634.757 | 2.434.786 | 0,70 |
| Hash aberto + Polinomial | 30% | 0,1843 | 0,0025 | 588.025 | 588.025 | 0,21 |
| Hash aberto + Polinomial | 60% | 2,3959 | 0,0166 | 1.007.405 | 1.595.430 | 0,42 |
| Hash aberto + Polinomial | 100% | 7,3900 | 0,0329 | 1.491.940 | 3.087.370 | 0,70 |
| Encadeado + Divisão | 30% | 0,2193 | 0,0032 | 136.724 | 136.724 | 0,21 |
| Encadeado + Divisão | 60% | 0,2278 | 0,0042 | 663.305 | 800.029 | 0,42 |
| Encadeado + Divisão | 100% | 0,3426 | 0,0054 | 1.634.757 | 2.434.786 | 0,70 |
| Encadeado + Polinomial | 30% | 0,3171 | 0,0031 | 588.025 | 588.025 | 0,21 |
| Encadeado + Polinomial | 60% | 0,3406 | 0,0045 | 1.007.405 | 1.595.430 | 0,42 |
| Encadeado + Polinomial | 100% | 0,4588 | 0,0053 | 1.491.940 | 3.087.370 | 0,70 |

Em todas as etapas, as 10.000 matrículas sorteadas foram encontradas pelas cinco estruturas
(coluna `encontrados` do CSV).

**Análise**

- **Lista x hash:** a busca na lista cresce junto com o volume (O(n)). As 10.000 buscas da última
  etapa levam ~1,6 s com 100 mil registros, ~53 s com 1 milhão e ~556 s (mais de 9 minutos) com
  10 milhões, cerca de 10 vezes mais a cada volume. Nas tabelas hash, o mesmo trabalho leva
  poucos milissegundos em qualquer volume (O(1) em média). Com 10 milhões, a diferença passa de
  100.000 vezes.
- **Inserção:** é rápida em quase todas as estruturas. A lista insere no início (O(1)), e as tabelas
  hash calculam a posição direto. A exceção é o hash aberto com a Polinomial em 10 milhões (próximo item).
- **Divisão x Polinomial:** nesta base, a Divisão gera menos colisões em todos os volumes
  (2.434.786 contra 3.087.370 em 10 milhões). As matrículas do gerador são números crescentes
  com passo quase fixo, e `k mod m` com `m` primo espalha bem esse tipo de sequência. A Polinomial
  trata a matrícula como texto. Como matrículas vizinhas diferem só nos últimos dígitos, os índices
  delas saem próximos entre si e formam grupos na tabela.
- **Agrupamento primário:** no hash aberto, esses grupos da Polinomial se juntam com a sondagem
  linear. Em 10 milhões, a inserção da etapa 60–100% subiu para ~7,4 s (contra ~0,2 s da Divisão) e
  a busca para ~33 ms, 10 vezes mais que as outras tabelas. O encadeamento separado com a mesma
  função não sofre esse efeito (0,46 s), porque cada colisão fica na lista do próprio índice e não
  ocupa as posições vizinhas. Isso confirma por que o fator de carga foi limitado a 0,7.
- **Aberto x encadeado:** as colisões são iguais para a mesma função, porque dependem só do índice
  inicial. Com a Divisão, o hash aberto é um pouco mais rápido que o encadeado, já que percorre um
  vetor contínuo na memória, enquanto o encadeado segue ponteiros para nós espalhados.
