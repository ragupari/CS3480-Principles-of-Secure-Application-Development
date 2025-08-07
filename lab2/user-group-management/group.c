#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define FILENAME "groups.txt"
#define MAX_LINE 512

bool group_value_exists(const char* filename, const char* value) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    char line[256];
    char first_word[100];

    while (fgets(line, sizeof(line), file)) {
        // Remove newline
        line[strcspn(line, "\n")] = '\0';

        if (sscanf(line, "%s", first_word) == 1) {
            if (strcmp(first_word, value) == 0) {
                fclose(file);
                return true;
            }
        }
    }

    fclose(file);
    return false;
}

void append_group_if_not_exists(const char* filename, const char* value) {
    if (!group_value_exists(filename, value)) {
        FILE* file = fopen(filename, "a");
        if (file) {
            fprintf(file, "%s\n", value);
            fclose(file);
            printf("Group '%s' added to file.\n", value);
        } else {
            printf("Could not open file to write.\n");
        }
    } else {
        printf("Group '%s' already exists in file.\n", value);
    }
}

void addgroup(const char* args) {
    append_group_if_not_exists(FILENAME, args);
}

void delgroup(const char *groupname) {
    // Step 1: Remove the group line from groups.txt
    FILE *gfile = fopen("groups.txt", "r");
    FILE *gtemp = fopen("groups_tmp.txt", "w");

    if (!gfile || !gtemp) {
        printf("❌ Could not open groups.txt or temporary file.\n");
        return;
    }

    char line[MAX_LINE];
    bool group_found = false;

    while (fgets(line, sizeof(line), gfile)) {
        char group_in_line[100];
        sscanf(line, "%s", group_in_line);
        if (strcmp(group_in_line, groupname) != 0) {
            fputs(line, gtemp);
        } else {
            group_found = true;
        }
    }

    fclose(gfile);
    fclose(gtemp);
    remove("groups.txt");
    rename("groups_tmp.txt", "groups.txt");

    if (!group_found) {
        printf("⚠️ Group '%s' not found in groups.txt.\n", groupname);
        return;
    }

    // Step 2: Remove group from all users in users.txt
    FILE *ufile = fopen("users.txt", "r");
    FILE *utemp = fopen("users_tmp.txt", "w");

    if (!ufile || !utemp) {
        printf("❌ Could not open users.txt or temporary file.\n");
        return;
    }

    while (fgets(line, sizeof(line), ufile)) {
        char newline[MAX_LINE] = "";
        char *token = strtok(line, " \n");

        while (token != NULL) {
            if (strcmp(token, groupname) != 0) {
                strcat(newline, token);
                strcat(newline, " ");
            }
            token = strtok(NULL, " \n");
        }

        if (strlen(newline) > 0) {
            newline[strcspn(newline, " ")] = '\0'; // remove trailing space
            fprintf(utemp, "%s\n", newline);
        }
    }

    fclose(ufile);
    fclose(utemp);
    remove("users.txt");
    rename("users_tmp.txt", "users.txt");

    printf("✅ Group '%s' deleted successfully.\n", groupname);
}