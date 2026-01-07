// asteroid_crud.c
// CRUD de asteroides potencialmente perigosos (CSV <-> Memória)
// Tópicos: if/else, loops, funções, arquivos (fopen/fgets/fprintf), ponteiros e alocação dinâmica (calloc/realloc)

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
  #include <windows.h>
  void dormir(int micros) { Sleep(micros / 1000); } // aproximação
#else
  #include <unistd.h>
  void dormir(int micros) { usleep(micros); }
#endif

void limparTela() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void barraCarregando(const char *texto, int passos, int delay_us) {
    const char *frames[] = {"[=     ]", "[==    ]", "[===   ]", "[====  ]", "[===== ]", "[======]"};
    int nframes = 6;

    printf("%s ", texto);
    fflush(stdout);

    for (int i = 0; i < passos; i++) {
        printf("\r%s %s %3d%%", texto, frames[i % nframes], (i + 1) * 100 / passos);
        fflush(stdout);
        dormir(delay_us);
    }
    printf("\n");
}

void transicao(const char *titulo) {
    limparTela();
    printf("=========================================\n");
    printf("   %s\n", titulo);
    printf("=========================================\n\n");
    barraCarregando("Processando", 24, 45000);
}

void desenharFogueteSubindo(int altura, int flameFrame) {
    printf("     .        *        .        *\n");
    printf("   *     .        *        .     \n");
    printf("        .     *        .       * \n\n");

    for (int i = 0; i < altura; i++) printf("\n");

    printf("        /\\\n");
    printf("       /  \\\n");
    printf("       |  |\n");
    printf("       |  |\n");
    printf("      /____\\\n");
    printf("       /||\\\n");

    if (flameFrame % 3 == 0) {
        printf("        **\n");
        printf("        **\n");
    } else if (flameFrame % 3 == 1) {
        printf("       *  *\n");
        printf("        **\n");
    } else {
        printf("        **\n");
        printf("       *  *\n");
    }

    printf("\nTerra: ███████████████████████████████\n");
}

void viagemHorizontal(int pos, int largura) {
    char linha[256];
    if (largura > 200) largura = 200;

    for (int i = 0; i < largura; i++) linha[i] = ' ';
    linha[largura] = '\0';

    if (pos >= 0 && pos < largura) linha[pos] = '>';
    if (pos - 1 >= 0 && pos - 1 < largura) linha[pos - 1] = '=';
    if (pos - 2 >= 0 && pos - 2 < largura) linha[pos - 2] = '=';

    int planeta = largura - 6;
    if (planeta >= 0) {
        linha[planeta + 0] = '(';
        linha[planeta + 1] = 'O';
        linha[planeta + 2] = 'O';
        linha[planeta + 3] = ')';
    }

    printf("\r%s", linha);
    fflush(stdout);
}

void pousoNoPlaneta(int passo) {
    limparTela();

    printf("      *        .     *        .      \n");
    printf("   .        *      .      *        . \n\n");
    printf("                (OO--)\n");
    printf("                (OOOO)   Planeta X\n\n");

    int espacos = 8 - passo;
    if (espacos < 0) espacos = 0;
    for (int i = 0; i < espacos; i++) printf("\n");

    printf("          /\\\n");
    printf("         /  \\\n");
    printf("         |  |\n");
    printf("         |  |\n");
    printf("        /____\\\n");
    printf("         /||\\\n");

    if (passo < 7) printf("          **\n");
    else          printf("          ||\n");

    printf("\nSolo:  ███████████████████████████████\n");
    fflush(stdout);
}

void animacaoMissaoRapida(void) {
    // mini versão (rápida) só pra não atrasar o CRUD
    for (int t = 0; t < 8; t++) {
        limparTela();
        int altura = 10 - t;
        desenharFogueteSubindo(altura, t);
        dormir(80000);
    }

    printf("\nViagem: ");
    fflush(stdout);
    int largura = 50;
    for (int p = 2; p < largura - 10; p++) {
        viagemHorizontal(p, largura);
        dormir(35000);
    }
    printf("\n");

    for (int k = 0; k <= 6; k++) {
        pousoNoPlaneta(k);
        dormir(90000);
    }
}

void impactoAsteroide(void) {
    // efeito curtinho pra “alerta de perigo”
    const char *frames[] = {
        "           .\n\n  Terra:  (____)\n",
        "         ...\n\n  Terra:  (____)\n",
        "      .........\n\n  Terra:  (____)\n",
        "   ...............\n\n  Terra:  (____)\n",
        "********* IMPACTO *********\n\n  Terra:  (____)\n"
    };

    for (int i = 0; i < 5; i++) {
        limparTela();
        printf("%s", frames[i]);
        dormir(100000);
    }
}

#define LINE_MAX_LEN 2048
#define STR_MAX 128

typedef struct {
    char date[16];                  // 2026-01-04
    char name[STR_MAX];             // 67381 (2000 OL8)
    long id;                        // id do NEO
    int hazardous;                  // True / False
    double absolute_magnitude_h;    // H
    double diameter_min_m;
    double diameter_max_m;
    double miss_distance_km;
    double velocity_km_s;
} Asteroid;


typedef struct {
    Asteroid *data;
    size_t size;    // quantos registros usados
    size_t cap;     // capacidade alocada
} AsteroidDB;

/* ---------- utilidades ---------- */

static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

static void tolower_str(char *s) {
    for (; s && *s; s++) *s = (char)tolower((unsigned char)*s);
}

static int read_int(const char *prompt) {
    char buf[256];
    int x;
    for (;;) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) return 0;
        if (sscanf(buf, "%d", &x) == 1) return x;
        printf("Entrada inválida. Tente novamente.\n");
    }
}

static double read_double(const char *prompt) {
    char buf[256];
    double x;
    for (;;) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) return 0.0;
        if (sscanf(buf, "%lf", &x) == 1) return x;
        printf("Entrada inválida. Tente novamente.\n");
    }
}

static void read_string(const char *prompt, char *out, size_t outsz) {
    printf("%s", prompt);
    if (fgets(out, (int)outsz, stdin)) {
        trim_newline(out);
    } else {
        out[0] = '\0';
    }
}

/* ---------- DB (memória) ---------- */

static void db_init(AsteroidDB *db) {
    db->data = NULL;
    db->size = 0;
    db->cap  = 0;
}

static void db_free(AsteroidDB *db) {
    free(db->data);
    db_init(db);
}

static int db_reserve(AsteroidDB *db, size_t newcap) {
    if (newcap <= db->cap) return 1;
    Asteroid *p = (Asteroid*)realloc(db->data, newcap * sizeof(Asteroid));
    if (!p) return 0;
    db->data = p;
    db->cap = newcap;
    return 1;
}

static int db_push(AsteroidDB *db, Asteroid a) {
    if (db->size == db->cap) {
        size_t next = (db->cap == 0) ? 64 : db->cap * 2;
        if (!db_reserve(db, next)) return 0;
    }
    db->data[db->size++] = a;
    return 1;
}

static int db_find_index_by_id(const AsteroidDB *db, int id) {
    for (size_t i = 0; i < db->size; i++) {
        if (db->data[i].id == id) return (int)i;
    }
    return -1;
}

static int db_next_id(const AsteroidDB *db) {
    int max_id = 0;
    for (size_t i = 0; i < db->size; i++) {
        if (db->data[i].id > max_id) max_id = db->data[i].id;
    }
    return max_id + 1;
}

static void db_delete_index(AsteroidDB *db, size_t idx) {
    if (idx >= db->size) return;
    for (size_t i = idx; i + 1 < db->size; i++) {
        db->data[i] = db->data[i + 1];
    }
    db->size--;
}

/* ---------- CSV parsing simples ----------
   IMPORTANTE: ajuste conforme seu CSV real.
   Este parser espera 5 campos:
   id,name,hazardous,diameter_km,miss_distance_au

   - hazardous aceita: 0/1, true/false, y/n, yes/no
   - Se seu CSV tiver outras colunas, você pode:
     (a) ignorar as que não quer, ou
     (b) adaptar para ler na posição correta.
----------------------------------------- */

static int parse_bool(const char *s) {
    if (!s) return 0;
    char tmp[64];
    strncpy(tmp, s, sizeof(tmp)-1);
    tmp[sizeof(tmp)-1] = '\0';
    trim_newline(tmp);
    tolower_str(tmp);

    if (strcmp(tmp, "1") == 0) return 1;
    if (strcmp(tmp, "0") == 0) return 0;
    if (strcmp(tmp, "true") == 0) return 1;
    if (strcmp(tmp, "false") == 0) return 0;
    if (strcmp(tmp, "y") == 0 || strcmp(tmp, "yes") == 0) return 1;
    if (strcmp(tmp, "n") == 0 || strcmp(tmp, "no") == 0) return 0;
    return 0;
}

// split bem simples por vírgula (não lida com vírgula dentro de aspas).
// Se seu CSV tiver campos com aspas/vírgulas internas, me manda 1 linha exemplo que eu adapto.
static int split_csv_simple(char *line, char *fields[], int max_fields) {
    int count = 0;
    char *p = line;
    while (*p && count < max_fields) {
        fields[count++] = p;
        while (*p && *p != ',') p++;
        if (*p == ',') { *p = '\0'; p++; }
    }
    return count;
}

static int parse_csv_line(char *line, Asteroid *out) {
    trim_newline(line);
    if (line[0] == '\0') return 0;

    // pula header
    if (strncmp(line, "date,", 5) == 0) return 0;

    char *fields[16] = {0};
    int n = split_csv_simple(line, fields, 16);
    if (n < 9) return 0;

    Asteroid a;

    strncpy(a.date, fields[0], sizeof(a.date)-1);
    a.date[sizeof(a.date)-1] = '\0';

    strncpy(a.name, fields[1], sizeof(a.name)-1);
    a.name[sizeof(a.name)-1] = '\0';

    a.id = atol(fields[2]);

    a.hazardous =
        (strcmp(fields[3], "True") == 0 || strcmp(fields[3], "true") == 0);

    a.absolute_magnitude_h = atof(fields[4]);
    a.diameter_min_m      = atof(fields[5]);
    a.diameter_max_m      = atof(fields[6]);
    a.miss_distance_km    = atof(fields[7]);
    a.velocity_km_s       = atof(fields[8]);

    *out = a;
    return 1;
}

static int load_csv(const char *path, AsteroidDB *db) {
    FILE *fp = fopen(path, "r"); // modo "r" conforme tabela de modos :contentReference[oaicite:3]{index=3}
    if (!fp) {
        printf("Erro: não consegui abrir '%s'\n", path);
        return 0;
    }

    char line[LINE_MAX_LEN];
    while (fgets(line, sizeof(line), fp)) {
        Asteroid a;
        if (parse_csv_line(line, &a)) {
            if (!db_push(db, a)) {
                fclose(fp);
                printf("Erro: memória insuficiente.\n");
                return 0;
            }
        }
    }
    fclose(fp);
    return 1;
}

static int save_csv(const char *path, const AsteroidDB *db) {
    FILE *fp = fopen(path, "w"); // "w" sobrescreve :contentReference[oaicite:4]{index=4}
    if (!fp) {
        printf("Erro: não consegui criar '%s'\n", path);
        return 0;
    }

    // header
    fprintf(fp, "id,name,hazardous,diameter_km,miss_distance_au\n");

    for (size_t i = 0; i < db->size; i++) {
        const Asteroid *a = &db->data[i];
        fprintf(fp, "%d,%s,%d,%.6f,%.6f\n",
                a->id, a->name, a->is_hazardous, a->diameter_km, a->miss_distance_au);
    }

    fclose(fp);
    return 1;
}

static void print_header(void) {
    printf("DATE       | NAME                   | ID     | HZD | Dmin(m) | Dmax(m) | MISS_DIST(km) | VEL(km/s)\n");
    printf("-----------------------------------------------------------------------------------------------\n");
}

/* ---------- READ (listar/buscar/filtrar) ---------- */

static void print_one(const Asteroid *a) {
    printf("%-10s | %-22s | %-6ld | %-4s | %6.1f m | %6.1f m | %10.0f km | %6.2f km/s\n",
           a->date,
           a->name,
           a->id,
           a->hazardous ? "YES" : "NO",
           a->diameter_min_m,
           a->diameter_max_m,
           a->miss_distance_km,
           a->velocity_km_s);
}

static void print_one(const Asteroid *a) {
    printf("%-6d | %-24s | %-9s | %-11.3f | %-14.6f\n",
           a->id, a->name, a->is_hazardous ? "YES" : "NO",
           a->diameter_km, a->miss_distance_au);
}

static void list_all(const AsteroidDB *db) {
    print_header();
    for (size_t i = 0; i < db->size; i++) print_one(&db->data[i]);
}

static void list_hazardous(const AsteroidDB *db) {
    transicao("MODO DEFESA PLANETÁRIA: FILTRANDO PERIGOSOS");
    impactoAsteroide(); 

    print_header();
    for (size_t i = 0; i < db->size; i++) {
        if (db->data[i].is_hazardous) print_one(&db->data[i]);
    }
}

static void search_by_name(const AsteroidDB *db) {
    char q[STR_MAX];
    read_string("Digite parte do nome (case-insensitive): ", q, sizeof(q));
    char qlow[STR_MAX];
    strncpy(qlow, q, sizeof(qlow)-1); qlow[sizeof(qlow)-1] = '\0';
    tolower_str(qlow);

    print_header();
    for (size_t i = 0; i < db->size; i++) {
        char name_low[STR_MAX];
        strncpy(name_low, db->data[i].name, sizeof(name_low)-1); name_low[sizeof(name_low)-1] = '\0';
        tolower_str(name_low);

        if (strstr(name_low, qlow)) print_one(&db->data[i]);
    }
}

/* ---------- CREATE / UPDATE / DELETE ---------- */

static void create_asteroid(AsteroidDB *db) {
    transicao("CADASTRANDO NOVO ASTEROIDE");
    
    Asteroid a;
    a.id = db_next_id(db);

    read_string("Nome: ", a.name, sizeof(a.name));
    a.is_hazardous = read_int("Potencialmente perigoso? (1=sim, 0=nao): ");
    a.diameter_km = read_double("Diametro (km): ");
    a.miss_distance_au = read_double("Miss distance (AU): ");

    if (db_push(db, a)) {
        printf("Criado com sucesso! ID=%d\n", a.id);
    } else {
        printf("Erro: não consegui inserir (memória insuficiente).\n");
    }
}

static void update_asteroid(AsteroidDB *db) {
    transicao("ATUALIZANDO REGISTRO");

    int id = read_int("ID para atualizar: ");
    int idx = db_find_index_by_id(db, id);
    if (idx < 0) {
        printf("Não encontrado.\n");
        return;
    }

    Asteroid *a = &db->data[idx]; // ponteiro para atualizar direto (pass-by-reference) :contentReference[oaicite:5]{index=5}
    printf("Atualizando ID=%d (deixe vazio para manter nome)\n", a->id);

    char buf[STR_MAX];
    read_string("Novo nome (enter para manter): ", buf, sizeof(buf));
    if (buf[0] != '\0') {
        strncpy(a->name, buf, sizeof(a->name)-1);
        a->name[sizeof(a->name)-1] = '\0';
    }

    a->is_hazardous = read_int("Hazardous? (1/0): ");
    a->diameter_km = read_double("Diametro (km): ");
    a->miss_distance_au = read_double("Miss distance (AU): ");

    printf("Atualizado.\n");
}

static void delete_asteroid(AsteroidDB *db) {
    transicao("REMOVENDO DO CATÁLOGO");

    int id = read_int("ID para deletar: ");
    int idx = db_find_index_by_id(db, id);
    if (idx < 0) {
        printf("Não encontrado.\n");
        return;
    }
    db_delete_index(db, (size_t)idx);
    printf("Deletado.\n");
}

/* ---------- MENU ---------- */

static void show_menu(void) {
    printf("\n=== ASTEROID CRUD ===\n");
    printf("1) Listar todos\n");
    printf("2) Listar apenas potencialmente perigosos\n");
    printf("3) Buscar por nome\n");
    printf("4) Criar novo registro\n");
    printf("5) Atualizar por ID\n");
    printf("6) Deletar por ID\n");
    printf("7) Salvar CSV\n");
    printf("0) Sair\n");
}

int main(void) {
    AsteroidDB db;
    db_init(&db);

    char path_in[256];
    read_string("Caminho do CSV de entrada (ex: neos.csv): ", path_in, sizeof(path_in));

    transicao("INICIANDO SISTEMAS DA MISSÃO");
    barraCarregando("Carregando catálogo de NEOs", 28, 40000);

    if (!load_csv(path_in, &db)) {
        printf("Falha ao carregar CSV. Encerrando.\n");
        db_free(&db);
        return 1;
    }

    barraCarregando("Sincronizando banco em memória", 20, 35000);
    printf("OK! Carregados %zu registros.\n", db.size);

    for (;;) {
        show_menu();
        int op = read_int("Escolha: ");

        if (op == 0) break;
        else if (op == 1) list_all(&db);
        else if (op == 2) list_hazardous(&db);
        else if (op == 3) search_by_name(&db);
        else if (op == 4) create_asteroid(&db);
        else if (op == 5) update_asteroid(&db);
        else if (op == 6) delete_asteroid(&db);
        else if (op == 7) {
            char path_out[256];
            read_string("Caminho do CSV para salvar (ex: neos_updated.csv): ", path_out, sizeof(path_out));

            transicao("ENVIANDO DADOS PARA O CENTRO DE CONTROLE");
            barraCarregando("Gravando CSV", 30, 30000);

            if (save_csv(path_out, &db)) {
                animacaoMissaoRapida();
                printf("\nArquivo salvo! Missão completa. 🚀\n");
            } else {
                printf("Erro ao salvar.\n");
            }
        } else {
            printf("Opção inválida.\n");
        }
    }

    db_free(&db);
    printf("Tchau!\n");
    return 0;
}
