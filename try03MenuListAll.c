// asteroid_crud.c
// CRUD of potential dangerous asteroids

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>


/* function's prototype*/
void loadingBar(const char *texto, int passos, int delay_us);
void basicTransition(const char *titulo);
void asteroidImpact(void);
void cleanWindow(void);


 /* BASIC COMMANDS TO USE DURING THE ENTIRE EXECUTION */
#ifdef _WIN32
  #include <windows.h>
  void sleepNew(int micros) { Sleep(micros / 1000); } // aproximação
#else
  #include <unistd.h>
  void sleepNew(int micros) { usleep(micros); }
#endif

void cleanWindow() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}


/* defining*/
#define LINE_MAX_LEN 2048
#define STR_MAX 128

/* STRUCTURES */
typedef struct {
    char date[16];                  // 2026-01-04
    char name[STR_MAX];             // 67381 (2000 OL8)
    long id;                        // id do NEO
    int isHazardous;                  // True / False
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


/* Animation Functions */
void asteroidImpact(void) {

    int i;
    const char *frames[] = {
        "           .\n\n  Terra:  (____)\n",
        "         ...\n\n  Terra:  (____)\n",
        "      .........\n\n  Terra:  (____)\n",
        "   ...............\n\n  Terra:  (____)\n",
        "********* IMPACTO *********\n\n  Terra:  (____)\n"
    };

    for (i = 0; i < 5; i++) {
        cleanWindow();
        printf("%s", frames[i]);
        sleepNew(100000);
    }
}

void basicTransition(const char *titulo) {
    cleanWindow();
    printf("=========================================\n");
    printf("   %s\n", titulo);
    printf("=========================================\n\n");
    loadingBar("Loading", 24, 45000);
}

void loadingBar(const char *texto, int passos, int delay_us) {
    const char *frames[] = {"[=     ]", "[==    ]", "[===   ]", "[====  ]", "[===== ]", "[======]"};
    int nframes = 6, i;

    printf("%s ", texto);
    fflush(stdout);

    for (i = 0; i < passos; i++) {
        printf("\r%s %s %3d%%", texto, frames[i % nframes], (i + 1) * 100 / passos);
        fflush(stdout);
        sleepNew(delay_us);
    }
    printf("\n");
}


/* Useful functions */
static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
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

// split bem simples por vírgula (não lida com vírgula dentro de aspas)
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

    // skip header
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

    a.isHazardous =
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

static void print_one(const Asteroid *a) {
    printf("%-10s | %-22s | %-6ld | %-3s | %6.1f m | %6.1f m | %10.0f km | %6.2f km/s\n",
           a->date,
           a->name,
           a->id,
           a->isHazardous ? "YES" : "NO",
           a->diameter_min_m,
           a->diameter_max_m,
           a->miss_distance_km,
           a->velocity_km_s);
}

static void print_header(void) {
    printf("DATE       | NAME                   | ID     | HZD | Dmin(m) | Dmax(m) | MISS_DIST(km) | VEL(km/s)\n");
    printf("-----------------------------------------------------------------------------------------------\n");
}

/* CRUD Functions */
static void list_all(const AsteroidDB *db) {
    print_header();
    size_t i;
    for (i = 0; i < db->size; i++) print_one(&db->data[i]);
}

static void list_hazardous(const AsteroidDB *db) {
    basicTransition("PLANETARY DEFENSE MODE: FILTERING THE MOST DANGEROUS ONES");
    asteroidImpact(); 

    print_header();
    size_t i;
    for (i = 0; i < db->size; i++) {
        if (db->data[i].isHazardous) print_one(&db->data[i]);
    }
}

static void show_menu(void) {
    printf("\n=== WELCOMEEEEE EXPLORER!! ===\n");
    printf("1) List all\n");
    printf("2) List the most dangerous ones <notworking>\n");
    printf("3) Search by name <notworking>\n");
    printf("4) New register <notworking>\n");
    printf("5) Update <notworking>\n");
    printf("6) Delete <notworking>\n");
    printf("7) Save CSV <notworking>\n");
    printf("0) QUIT <notworking>\n");
}




int main(void) {
    AsteroidDB db;
    db_init(&db);

    char path_in[256] = "asteroids.csv";
    read_string("Caminho do CSV de entrada (ex: neos.csv): ", path_in, sizeof(path_in));

    basicTransition("STARTING MISSION SYSTEMS");
    loadingBar("Getting NEOs catalogs", 28, 40000);

    if (!load_csv(path_in, &db)) {
        printf("Failed to load CSV. Finishing.\n");
        db_free(&db);
        return 1;
    }

    loadingBar("Syncroning db and memory", 20, 35000);
    printf("OK! %zu registers loaded!\n", db.size);

    show_menu();

    for (;;) {
        
        int op = read_int("Your choice: ");

        if (op == 0) break;
        else if (op == 1) list_all(&db);
        printf("Do you want to do something else? (Y - 0 / N - 1)");
        int nx = read_int(" ");

        if(nx == 1) break;
        else show_menu();

    }
    printf("That's all baby!!\n");
    return 0;
}