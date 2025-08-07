#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_LINE 512

extern bool user_value_exists(const char *filename, const char *username);
extern bool group_value_exists(const char *filename, const char *groupname);

void usermod_append_group(const char *username, const char *groupname) {
    if (!user_value_exists("users.txt", username)) {
        printf("❌ User '%s' does not exist.\n", username);
        return;
    }
    if (!group_value_exists("groups.txt", groupname)) {
        printf("❌ Group '%s' does not exist.\n", groupname);
        return;
    }

    // --- Update users.txt ---
    FILE *ufile = fopen("users.txt", "r");
    FILE *utemp = fopen("users_tmp.txt", "w");
    if (!ufile || !utemp) {
        printf("❌ Failed to open users.txt or temp file.\n");
        return;
    }

    char line[MAX_LINE];
    bool user_modified = false;

    while (fgets(line, sizeof(line), ufile)) {
        char original[MAX_LINE];
        strcpy(original, line);
        line[strcspn(line, "\n")] = '\0';

        char user_in_line[MAX_LINE];
        if (sscanf(line, "%s", user_in_line) == 1 && strcmp(user_in_line, username) == 0) {
            if (strstr(line, groupname) == NULL) {
                strcat(line, " ");
                strcat(line, groupname);
                user_modified = true;
            }
            fprintf(utemp, "%s\n", line);
        } else {
            fprintf(utemp, "%s", original);
        }
    }

    fclose(ufile);
    fclose(utemp);
    remove("users.txt");
    rename("users_tmp.txt", "users.txt");

    // --- Update groups.txt ---
    FILE *gfile = fopen("groups.txt", "r");
    FILE *gtemp = fopen("groups_tmp.txt", "w");
    if (!gfile || !gtemp) {
        printf("❌ Failed to open groups.txt or temp file.\n");
        return;
    }

    bool group_modified = false;

    while (fgets(line, sizeof(line), gfile)) {
        char original[MAX_LINE];
        strcpy(original, line);
        line[strcspn(line, "\n")] = '\0';

        char group_in_line[MAX_LINE];
        if (sscanf(line, "%s", group_in_line) == 1 && strcmp(group_in_line, groupname) == 0) {
            if (strstr(line, username) == NULL) {
                strcat(line, " ");
                strcat(line, username);
                group_modified = true;
            }
            fprintf(gtemp, "%s\n", line);
        } else {
            fprintf(gtemp, "%s", original);
        }
    }

    fclose(gfile);
    fclose(gtemp);
    remove("groups.txt");
    rename("groups_tmp.txt", "groups.txt");

    // --- Final status ---
    if (user_modified || group_modified)
        printf("✅ Group '%s' added to user '%s' in users.txt and groups.txt.\n", groupname, username);
    else
        printf("⚠️ User '%s' already in group '%s'.\n", username, groupname);
}
