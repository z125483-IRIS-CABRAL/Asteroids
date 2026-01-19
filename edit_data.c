#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "edit_data.h" 

static void local_read_string(const char *prompt, char *out, size_t size) {
    printf("%s", prompt);
    if (fgets(out, (int)size, stdin)) {
        size_t len = strlen(out);
        if (len > 0 && out[len - 1] == '\n') {
            out[len - 1] = '\0';
        }
    } else {
        out[0] = '\0';
    }
}

// Input double
static double local_read_double(const char *prompt) {
    char buf[64];
    double value;
    printf("%s", prompt);
    if (fgets(buf, sizeof(buf), stdin)) {
        if (sscanf(buf, "%lf", &value) == 1) return value;
    }
    return 0.0;
}

// Input int
static int local_read_int(const char *prompt) {
    char buf[64];
    int value;
    printf("%s", prompt);
    if (fgets(buf, sizeof(buf), stdin)) {
        if (sscanf(buf, "%d", &value) == 1) return value;
    }
    return 0;
}


/* ---  Edit Data (Flowchart #4) --- */
void edit_data(AsteroidDB *db) {
    printf("\n=========================================\n");
    printf("     EDIT MODE: UPDATE ASTEROID DATA     \n");
    printf("=========================================\n");

    // 1. Request asteroid name
    char targetName[STR_MAX];
    local_read_string("Enter the name of the asteroid to edit: ", targetName, sizeof(targetName));

    // 2. Search logic
    Asteroid *found = NULL;
    int i;
    for (i = 0; i < db->size; i++) {
        if (strcmp(db->data[i].name, targetName) == 0) {
            found = &db->data[i]; //
            break;
        }
    }

    // 3. Name found?
    
    // Case No: Ouput error message and return
    if (found == NULL) {
        printf("\n[ERROR] Asteroid '%s' not found in database.\n", targetName);
        printf("Returning to menu...\n");
        return; 
    }

    // Case Yes: Show current data and keep edit
    printf("\n--- Current Data for '%s' ---\n", found->name);
    printf(" Found Date : %s\n", found->date);
    printf(" Hazardous  : %s\n", found->isHazardous ? "Yes" : "No");
    printf(" Velocity   : %.2f km/s\n", found->velocity_km_s);
    printf(" Diameter   : %.1f m (min) - %.1f m (max)\n", found->diameter_min_m, found->diameter_max_m);
    printf(" Abs Magnitude   : %.2f H\n", found->absolute_magnitude_h);
    printf(" Miss Distance   : %.2f km\n", found->miss_distance_km);
    printf("-----------------------------------\n");
    
    printf(">> Please enter new values below:\n");

    // 4. New Data
    local_read_string("New Date (YYYY-MM-DD): ", found->date, sizeof(found->date));

    int h = local_read_int("Is Hazardous? (1=True, 0=False): ");
    found->isHazardous = (h == 1);
    found->velocity_km_s  = local_read_double("New Velocity (km/s): ");
    found->diameter_min_m = local_read_double("New Min Diameter (m): ");
    found->diameter_max_m = local_read_double("New Max Diameter (m): ");
    found->absolute_magnitude_h = local_read_double("New Abs Magnitude (H): ");
    found->miss_distance_km = local_read_double("New Miss Distance (km)");

    // if not hazard: delete!
    //if date is not nbetween the range of the current CSV, change CSV and save on the correct one

    // 5. Update asteroid data
    printf("\n[SUCCESS] Data updated successfully!\n");
    
    printf("Updated: [%s] %s (Vel: %.2f km/s)\n", found->date, found->name, found->velocity_km_s);
    printf("Press Enter to return to menu...");
    getchar();
}
