#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "group.h"


#define USER_FILE       "users.txt"
#define GROUP_FILE      "groups.txt"
#define MAX_LINE        512
#define MAX_GROUPS      50
extern char current_user[50];


static void log_event(const char* user, const char* action, const char* target, const char* result) {
    FILE* fp = fopen("audit.log", "a");
    if (!fp) return;
    time_t now = time(NULL);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(fp, "%s | user=%s | action=%s | target=%s | result=%s\n",
            ts,
            (user && *user) ? user : "(none)",
            (action && *action) ? action : "(none)",
            (target && *target) ? target : "(none)",
            (result && *result) ? result : "(none)");
    fclose(fp);
}


// ---------------- Utility Functions ----------------

bool user_value_exists(const char* filename, const char* value) {
    FILE* file = fopen(filename, "r");
    if (!file) return false;

    char line[256], first_word[100];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        if (sscanf(line, "%99s", first_word) == 1) {
            if (strcmp(first_word, value) == 0) {
                fclose(file);
                return true;
            }
        }
    }
    fclose(file);
    return false;
}

static void append_user_if_not_exists(const char* filename, const char* value) {
    if (!user_value_exists(filename, value)) {
        FILE* file = fopen(filename, "a");
        if (file) {
            // username followed by default primary group (same as username)
            fprintf(file, "%s %s\n", value, value);
            fclose(file);
            printf("✅ User '%s' added.\n", value);
            log_event(current_user, "useradd", value, "success");
        } else {
            printf("❌ Could not open %s to write.\n", filename);
            log_event(current_user, "useradd", value, "open_failed");
        }
    } else {
        printf("⚠️ User '%s' already exists.\n", value);
        log_event(current_user, "useradd", value, "exists");
    }
}

// ---------------- User Management ----------------

void adduser(const char* username) {
    if (!username || !*username) {
        printf("❌ Invalid username.\n");
        log_event(current_user, "useradd", "(empty)", "invalid");
        return;
    }
    append_user_if_not_exists(USER_FILE, username);

    // Automatically create a personal group (same name)
    // addgroup is defined in group.c; it can log too if you add logging there.
    addgroup(username);
}

void deluser(const char* username) {
    if (!username || !*username) {
        printf("❌ Invalid username.\n");
        log_event(current_user, "deluser", "(empty)", "invalid");
        return;
    }

    // Remove from users.txt
    FILE* ufile = fopen(USER_FILE, "r");
    FILE* utemp = fopen("users_tmp.txt", "w");
    if (!ufile || !utemp) {
        if (ufile) fclose(ufile);
        if (utemp) fclose(utemp);
        printf("❌ Failed to open user files.\n");
        log_event(current_user, "deluser", username, "open_failed");
        return;
    }

    char line[MAX_LINE];
    bool found = false;
    while (fgets(line, sizeof(line), ufile)) {
        char user_in_line[100] = {0};
        sscanf(line, "%99s", user_in_line);
        if (strcmp(user_in_line, username) != 0) {
            fputs(line, utemp);
        } else {
            found = true;
        }
    }
    fclose(ufile);
    fclose(utemp);

    if (!found) {
        // nothing changed; clean temp and exit
        remove("users_tmp.txt");
        printf("⚠️ User '%s' not found.\n", username);
        log_event(current_user, "deluser", username, "noent");
        return;
    }

    // Replace original file
    if (remove(USER_FILE) != 0 || rename("users_tmp.txt", USER_FILE) != 0) {
        printf("❌ Failed to update %s.\n", USER_FILE);
        log_event(current_user, "deluser", username, "update_failed");
        // Attempt to roll back is omitted in this simple model
        return;
    }

    // Remove user tokens from groups.txt
    FILE* gfile = fopen(GROUP_FILE, "r");
    FILE* gtemp = fopen("groups_tmp.txt", "w");
    if (!gfile || !gtemp) {
        if (gfile) fclose(gfile);
        if (gtemp) fclose(gtemp);
        printf("❌ Failed to update groups.\n");
        log_event(current_user, "deluser", username, "group_open_failed");
        return;
    }

    while (fgets(line, sizeof(line), gfile)) {
        char newline[MAX_LINE] = "";
        char line_copy[MAX_LINE];
        strncpy(line_copy, line, sizeof(line_copy) - 1);
        line_copy[sizeof(line_copy) - 1] = '\0';

        char* token = strtok(line_copy, " \n");
        bool wrote_any = false;
        while (token) {
            if (strcmp(token, username) != 0) {
                if (wrote_any) strncat(newline, " ", sizeof(newline) - strlen(newline) - 1);
                strncat(newline, token, sizeof(newline) - strlen(newline) - 1);
                wrote_any = true;
            }
            token = strtok(NULL, " \n");
        }
        if (wrote_any) fprintf(gtemp, "%s\n", newline);
        // else: drop the whole line if it would be empty after removal
    }
    fclose(gfile);
    fclose(gtemp);

    if (remove(GROUP_FILE) != 0 || rename("groups_tmp.txt", GROUP_FILE) != 0) {
        printf("❌ Failed to finalize %s.\n", GROUP_FILE);
        log_event(current_user, "deluser", username, "group_update_failed");
        return;
    }

    printf("✅ User '%s' deleted successfully.\n", username);
    log_event(current_user, "deluser", username, "success");
}

int get_user_groups(const char* username, char groups[MAX_GROUPS][50]) {
    if (!username || !*username) return 0;

    FILE* file = fopen(USER_FILE, "r");
    if (!file) return 0;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        char* token = strtok(line, " ");
        if (token && strcmp(token, username) == 0) {
            int count = 0;
            while ((token = strtok(NULL, " ")) && count < MAX_GROUPS) {
                strncpy(groups[count], token, 49);
                groups[count][49] = '\0';
                count++;
            }
            fclose(file);
            return count;
        }
    }
    fclose(file);
    return 0;
}

bool user_in_group(const char* username, const char* groupname) {
    if (!username || !*username || !groupname || !*groupname) return false;

    FILE* fp = fopen(USER_FILE, "r");  // FIX: use USER_FILE (was "users.txt")
    if (!fp) {
        // Don't spam logs here; membership checks are frequent
        return false;
    }

    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\n")] = '\0';

        char* token = strtok(line, " ");
        if (!token) continue;

        // First token = username
        if (strcmp(token, username) != 0) continue;

        // Remaining tokens = groups
        token = strtok(NULL, " ");
        while (token) {
            if (strcmp(token, groupname) == 0) {
                fclose(fp);
                return true; // Found match
            }
            token = strtok(NULL, " ");
        }
        fclose(fp);
        return false; // user found, group not listed
    }

    fclose(fp);
    return false; // user not found
}

// ---------------- Permission Helpers ----------------

int get_user_type(const char* file_owner, const char* file_group, const char* username) {
    if (strcmp(username, file_owner) == 0) return 0; // Owner
    if (user_in_group(username, file_group)) return 1; // Group
    return 2; // Others
}

int has_permission(int perm, char mode, int user_type) {
    int owner = perm / 100;
    int group = (perm / 10) % 10;
    int other = perm % 10;
    int bits = (user_type == 0) ? owner : (user_type == 1 ? group : other);

    if (mode == 'r') return (bits & 4) != 0;
    if (mode == 'w') return (bits & 2) != 0;
    if (mode == 'x') return (bits & 1) != 0;
    return 0;
}
