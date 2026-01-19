// asteroid_crud.c
// CRUD of potential dangerous asteroids

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "edit_data.h"
#include "asteroid_db.h"
#include "delete_data.h"

/* function's prototype*/
void loadingBar(const char *texto, int passos, int delay_us);
void basicTransition(const char *titulo);
void asteroidImpact(void);
void cleanWindow(void);

#define _XOPEN_SOURCE 700

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

typedef struct {
    int start;            // ex: 20251201
    int end;              // ex: 20251208
    const char *csv;      // ex: "dez01.csv"
} RangeMap;

/* ---------- DB (memory) ---------- */
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
void tolower_str(char *s) {
    for (; s && *s; s++) *s = (char)tolower((unsigned char)*s);
}

void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n-1] == '\n' || s[n-1] == '\r')) {
        s[n-1] = '\0';
        n--;
    }
}

long read_long(const char *prompt) {
    char buf[256];
    long x;
    for (;;) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) return 0;
        if (sscanf(buf, "%ld", &x) == 1) return x;
        printf("Invalid input, hacker. Try again!\n");
    }
}

double read_double(const char *prompt) {
    char buf[256];
    double x;
    for (;;) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) return 0.0;
        if (sscanf(buf, "%lf", &x) == 1) return x;
        printf("Invalid input, hacker. Try again!\n");
    }   
}


void read_string(const char *prompt, char *out, size_t outsz) {
    printf("%s", prompt);
    if (fgets(out, (int)outsz, stdin)) {
        trim_newline(out);
    } else {
        out[0] = '\0';
    }
}

int read_int(const char *prompt) {
    char buf[256];
    int x;
    for (;;) {
        printf("%s", prompt);
        if (!fgets(buf, sizeof(buf), stdin)) return 0;
        if (sscanf(buf, "%d", &x) == 1) return x;
        printf("Entrada inválida. Tente novamente.\n");
    }
}

int split_csv_simple(char *line, char *fields[], int max_fields) {
    int count = 0;
    char *p = line;
    while (*p && count < max_fields) {
        fields[count++] = p;
        while (*p && *p != ',') p++;
        if (*p == ',') { *p = '\0'; p++; }
    }
    return count;
}


int parse_csv_line(char *line, Asteroid *out) {
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

void print_one(const Asteroid *a) {
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

void print_header(void) {
    printf("DATE       | NAME                   | ID     | HZD | Dmin(m) | Dmax(m) | MISS_DIST(km) | VEL(km/s)\n");
    printf("-----------------------------------------------------------------------------------------------\n");
}

static int append_asteroid_csv(const char *path, const Asteroid *a) {
    FILE *fp = fopen(path, "a");
    if (!fp) {
        printf("Erro: I could not open '%s' to write (append).\n", path);
        return 0;
    }

    // format: date,name,id,hazardous,absolute_magnitude_h,
    //          diameter_min_m,diameter_max_m,miss_distance_km,velocity_km_s
    fprintf(fp, "%s,%s,%ld,%s,%.10f,%.10f,%.10f,%.10f,%.10f\n",
            a->date,
            a->name,
            a->id,
            a->isHazardous ? "True" : "False",
            a->absolute_magnitude_h,
            a->diameter_min_m,
            a->diameter_max_m,
            a->miss_distance_km,
            a->velocity_km_s);

    fclose(fp);
    return 1;
}


/* CRUD Functions */
void list_all(const AsteroidDB *db) {
    print_header();
    size_t i;
    for (i = 0; i < db->size; i++) print_one(&db->data[i]);
}

/*void list_hazardous(const AsteroidDB *db) {
    basicTransition("PLANETARY DEFENSE MODE: FILTERING THE MOST DANGEROUS ONES");
    asteroidImpact(); 

    print_header();
    size_t i;
    for (i = 0; i < db->size; i++) {
        if (db->data[i].isHazardous) print_one(&db->data[i]);
    }
}*/

void show_menu(void) {
    printf("\n=== WELCOMEEEEE EXPLORER!! ===\n");
    printf("1) List all\n");
    printf("2) Change the date range\n");
    printf("3) Search by name\n");
    printf("4) New register\n");
    printf("5) Update\n");
    printf("6) Delete\n");
    printf("0) QUIT\n");
}

void search_by_name(const AsteroidDB *db) {
    char q[STR_MAX];
    read_string("Digite parte do nome (case-insensitive): ", q, sizeof(q));
    char qlow[STR_MAX];
    strncpy(qlow, q, sizeof(qlow)-1); qlow[sizeof(qlow)-1] = '\0';
    tolower_str(qlow);

    print_header();
    size_t i;
    for (i = 0; i < db->size; i++) {
        char name_low[STR_MAX];
        strncpy(name_low, db->data[i].name, sizeof(name_low)-1); name_low[sizeof(name_low)-1] = '\0';
        tolower_str(name_low);

        if (strstr(name_low, qlow)) print_one(&db->data[i]);
    }
}

 /*  LUCAS - NEW REGISTER */
static int datekey_from_ymd_dash(const char *s) {
    int y, m, d;
    if (sscanf(s, "%d-%d-%d", &y, &m, &d) != 3) return -1;
    if (y < 1900 || m < 1 || m > 12 || d < 1 || d > 31) return -1;
    return y * 10000 + m * 100 + d;
}

static const char* csv_for_key(int key, const RangeMap *maps, int maps_n) {
    for (int i = 0; i < maps_n; i++) {
        if (key >= maps[i].start && key <= maps[i].end) return maps[i].csv;
    }
    return NULL;
}

static long generate_next_id(const AsteroidDB *db) {
    long max_id = 0;

    for (size_t i = 0; i < db->size; i++) {
        if (db->data[i].id > max_id) {
            max_id = db->data[i].id;
        }
    }
    return max_id + 1;
}

void new_register(AsteroidDB *db, char *g_csv_path, const RangeMap *maps, int maps_n) {
    basicTransition("REGISTERING NEW NEAR-EARTH OBJECT");

    Asteroid a;
    read_string("Date (YYYY-MM-DD): ", a.date, sizeof(a.date));
    int new_key = datekey_from_ymd_dash(a.date);
    if (new_key < 0) {
        printf("[ERROR] Invalid date format. Use YYYY-MM-DD.\n");
        return;
    }

    const char *target_csv = csv_for_key(new_key, maps, maps_n);
    if (!target_csv) {
        printf("[ERROR] Sorry, there is no CSV data for this date (%s).\n", a.date);
        return;
    }

    if (strcmp(g_csv_path, target_csv) != 0) {
        printf("\n[WARNING] This record belongs to '%s', but you are currently using '%s'.\n",
               target_csv, g_csv_path);
        printf("Do you want to switch to '%s' and save the new record there? (1=yes, 0=no): ",
               target_csv);

        int ans = read_int("");
        if (ans != 1) {
            printf("Canceled. Tip: change the date range in the menu first.\n");
            return;
        }

        strcpy(g_csv_path, target_csv);
        db->size = 0;
        if (!load_csv(g_csv_path, db)) {
            printf("[ERROR] Failed to load CSV '%s'. Canceling insert.\n", g_csv_path);
            return;
        }
        printf("[OK] Switched to %s. Database reloaded.\n", g_csv_path);
    }

    read_string("Name: ", a.name, sizeof(a.name));
    //a.id = read_long("NEO ID (integer, ex: 2067381): ");

    char hazbuf[16];
    read_string("Hazardous? (True/False): ", hazbuf, sizeof(hazbuf));
    for (char *p = hazbuf; *p; p++) *p = (char)tolower((unsigned char)*p);
    a.isHazardous = (strcmp(hazbuf, "true") == 0 || strcmp(hazbuf, "yes") == 0 || strcmp(hazbuf, "1") == 0);

    a.absolute_magnitude_h = read_double("Absolute magnitude H: ");
    a.diameter_min_m       = read_double("Min diameter (m): ");
    a.diameter_max_m       = read_double("Max diameter (m): ");
    a.miss_distance_km     = read_double("Miss distance (km): ");
    a.velocity_km_s        = read_double("Velocity (km/s): ");
    a.id = generate_next_id(db);
    printf("Generated NEO ID: %ld\n", a.id);


    if (!db_push(db, a)) {
        printf("Erro: memória insuficiente ao inserir novo registro.\n");
        return;
    }

    if (!append_asteroid_csv(g_csv_path, &a)) {
        printf("[WARNING] Saved in memory, but FAILED to update CSV.\n");
    } else {
        printf("[SUCCESS] New asteroid saved in %s\n", g_csv_path);
    }

    print_header();
    print_one(&a);
}


int main(void) {
    AsteroidDB db;
    db_init(&db);

    char path_in[256] = "";
    char input[64];

    const RangeMap maps[] = {
        {20251201, 20251208, "dez01.csv"},
        {20251209, 20251216, "dez02.csv"},
        {20251217, 20251224, "dez03.csv"},
        {20260101, 20260105, "jan01.csv"},
    };
    const int maps_n = (int)(sizeof(maps) / sizeof(maps[0]));

    printf("Type a date to unblock the secret data (YYYY-MM-DD): ");
    if (!fgets(input, sizeof(input), stdin)) {
        printf("Input error.\n");
        return 1;
    }

    int year, month, day;
    if (sscanf(input, "%d-%d-%d", &year, &month, &day) != 3) {
        printf("Sorry, invalid input format! Not this time, hacker.\n");
        return 1;
    }


    if (year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
        printf("Sorry, invalid date values.\n");
        return 1;
    }

    int key = year * 10000 + month * 100 + day; 
    int i;

    for (i = 0; i < maps_n; i++) {
        if (key >= maps[i].start && key <= maps[i].end) {
            strcpy(path_in, maps[i].csv);  
            break;
        }
    }

    if (path_in[0] == '\0') {
        printf("Sorry, there is no data for this range! Let's explore more.\n");
        db_free(&db);
        return 1;
    }

    basicTransition("STARTING MISSION SYSTEMS");
    loadingBar("Getting NEOs catalogs", 28, 40000);

    if (!load_csv(path_in, &db)) {
        printf("Failed to load CSV. Finishing.\n");
        db_free(&db);
        return 1;
    }

    loadingBar("Synchronizing db and memory", 20, 35000);
    printf("OK! %zu registers loaded from %s!\n", db.size, path_in);

    show_menu();

    for (;;) {
        int op = read_int("Your choice: ");
        if (op == 0) break;
        else if (op == 1) list_all(&db);
//        else if (op == 2) list_hazardous(&db);
        else if (op == 2){
        path_in[0] = '\0';
        char new_input[64];
            printf("Type a date to unblock the secret data (YYYY-MM-DD): ");
            if (!fgets(new_input, sizeof(new_input), stdin)) {
                printf("Input error.\n");
                return 1;
            }

            int new_year, new_month, new_day;            
            if (sscanf(new_input, "%d-%d-%d", &new_year, &new_month, &new_day) != 3) {
                printf("Sorry, invalid input format! Not this time, hacker.\n");
                return 1;
            }
            if (new_year < 1900 || new_month < 1 || new_month > 12 || new_day < 1 || new_day > 31) {
                printf("Sorry, invalid date values.\n");
                return 1;
            }

            int i;
            int new_key = new_year * 10000 + new_month * 100 + new_day; // YYYYMMDD
            for (i = 0; i < maps_n; i++) {
                if (new_key >= maps[i].start && new_key <= maps[i].end) {
                    strcpy(path_in, maps[i].csv);   
                    break;
                }
            }

            if (path_in[0] == '\0') {
                printf("Sorry, there is no data for this range! Let's explore more.\n");
                db_free(&db);
                return 1;
            }

            basicTransition("STARTING MISSION SYSTEMS");
            loadingBar("Getting NEOs catalogs", 28, 40000);

            if (!load_csv(path_in, &db)) {
                printf("Failed to load CSV. Finishing.\n");
                db_free(&db);
                return 1;
            }

            loadingBar("Synchronizing db and memory", 20, 35000);
            printf("OK! %zu registers loaded from %s!\n", db.size, path_in);

        }
        else if (op == 3) search_by_name(&db);
        else if(op == 4) new_register(&db, path_in, maps, maps_n);
        else if (op == 5) edit_data(&db);
        else if(op == 6) delete_data(&db, path_in);

        int again = read_int("Do you want to explore more? (1=yes, 0=no): ");
        if (again == 0) break;
        show_menu();
    }

    db_free(&db);
    printf("That's all baby!!\n");
    return 0;
}
