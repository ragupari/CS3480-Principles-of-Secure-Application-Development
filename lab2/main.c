#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "user-group-management/group.h"
#include "user-group-management/user.h"
#include "user-group-management/usermod.h"
#include "virtual-file-system/vfs.h"
#include "audit/audit.h"

#define MAX_INPUT 256
#define MAX_ARGS 10

char current_user[50] = "";

int is_logged_in() {
    return strlen(current_user) > 0;
}

int main() {
    char input[MAX_INPUT];
    char* args[MAX_ARGS];
    int arg_count = 0;

    init_fs();
    load_vfs();

    while (1) {
        printf("command> ");
        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';

        arg_count = 0;
        char* token = strtok(input, " ");
        while (token && arg_count < MAX_ARGS) {
            args[arg_count++] = token;
            token = strtok(NULL, " ");
        }
        if (arg_count == 0) continue;

        // Exit
        if (strcmp(args[0], "exit") == 0) {
            save_vfs();
            log_event(current_user, "exit", "-", "success");
            break;
        }

        // USER / GROUP MANAGEMENT
        else if (strcmp(args[0], "useradd") == 0 && arg_count == 2) {
            adduser(args[1]);
            log_event(current_user, "useradd", args[1], "success");
        }
        else if (strcmp(args[0], "groupadd") == 0 && arg_count == 2) {
            addgroup(args[1]);
            log_event(current_user, "groupadd", args[1], "success");
        }
        else if (strcmp(args[0], "usermod") == 0 && arg_count == 5 &&
                 strcmp(args[1], "-a") == 0 && strcmp(args[2], "-G") == 0) {
            if (!is_logged_in()) { printf("Please login first.\n"); continue; }
            usermod_append_group(args[4], args[3]);
            log_event(current_user, "usermod", args[4], "success");
        }
        else if (strcmp(args[0], "deluser") == 0 && arg_count == 2) {
            if (!is_logged_in()) { printf("Please login first.\n"); continue; }
            deluser(args[1]);
            log_event(current_user, "deluser", args[1], "success");
        }
        else if (strcmp(args[0], "delgroup") == 0 && arg_count == 2) {
            if (!is_logged_in()) { printf("Please login first.\n"); continue; }
            delgroup(args[1]);
            log_event(current_user, "delgroup", args[1], "success");
        }

        // LOGIN / LOGOUT
        else if (strcmp(args[0], "login") == 0 && arg_count == 2) {
            if (is_logged_in()) {
                printf("A user is already logged in as '%s'. Please logout first.\n", current_user);
                log_event(current_user, "login", args[1], "failed_already_logged_in");
            } else if (!user_value_exists("users.txt", args[1])) {
                printf("Login failed: user '%s' does not exist.\n", args[1]);
                log_event("(none)", "login", args[1], "failed_no_user");
            } else {
                strcpy(current_user, args[1]);
                printf("Logged in as %s\n", current_user);
                go_to_home_directory();
                log_event(current_user, "login", args[1], "success");
            }
        }
        else if (strcmp(args[0], "logout") == 0) {
            if (!is_logged_in()) {
                printf("No user is currently logged in.\n");
                log_event("(none)", "logout", "-", "failed_no_login");
            } else {
                printf("User %s logged out.\n", current_user);
                log_event(current_user, "logout", "-", "success");
                current_user[0] = '\0';
                cd_vfs("/");
            }
        }

        // VFS COMMANDS
        else if (strcmp(args[0], "mkdir") == 0 && arg_count == 2) {
            mkdir_vfs(args[1]);
            log_event(current_user, "mkdir", args[1], "success");
        }
        else if (strcmp(args[0], "touch") == 0 && arg_count == 2) {
            touch_vfs(args[1]);
            log_event(current_user, "touch", args[1], "success");
        }
        else if (strcmp(args[0], "ls") == 0) {
            if (arg_count == 2 && strcmp(args[1], "-l") == 0) ls_l_vfs();
            else ls_vfs();
            log_event(current_user, "ls", "-", "success");
        }
        else if (strcmp(args[0], "cd") == 0 && arg_count == 2) {
            cd_vfs(args[1]);
            log_event(current_user, "cd", args[1], "success");
        }
        else if (strcmp(args[0], "pwd") == 0) {
            pwd_vfs();
            log_event(current_user, "pwd", "-", "success");
        }
        else if (strcmp(args[0], "write") == 0 && arg_count >= 3) {
            char content[1024] = "";
            for (int i = 2; i < arg_count; ++i) {
                strcat(content, args[i]);
                if (i != arg_count - 1) strcat(content, " ");
            }
            write_vfs(args[1], content);
            log_event(current_user, "write", args[1], "success");
        }
        else if (strcmp(args[0], "rm") == 0 && arg_count == 2) {
            rm_vfs(args[1]);
            log_event(current_user, "rm", args[1], "success");
        }
        else if (strcmp(args[0], "rm") == 0 && arg_count == 3 && strcmp(args[1], "-r") == 0) {
            rm_r_vfs(args[2]);
            log_event(current_user, "rm -r", args[2], "success");
        }
        else if (strcmp(args[0], "read") == 0 && arg_count == 2) {
            read_vfs(args[1]);
            log_event(current_user, "read", args[1], "success");
        }
        else if (strcmp(args[0], "tree") == 0) {
            tree();
            log_event(current_user, "tree", "-", "success");
        }
        else if (strcmp(args[0], "save") == 0) {
            save_vfs();
            log_event(current_user, "save", "-", "success");
        }
        else if (strcmp(args[0], "load") == 0) {
            load_vfs();
            log_event(current_user, "load", "-", "success");
        }
        else if (strcmp(args[0], "chown") == 0 && arg_count >= 3) {
            char new_owner[50] = "", new_group[50] = "";
            char* colon_pos = strchr(args[1], ':');
            if (colon_pos) {
                if (colon_pos != args[1]) {
                    strncpy(new_owner, args[1], colon_pos - args[1]);
                    new_owner[colon_pos - args[1]] = '\0';
                }
                if (*(colon_pos + 1) != '\0') strcpy(new_group, colon_pos + 1);
            } else {
                strcpy(new_owner, args[1]);
            }
            chown_vfs(new_owner, new_group, args[2]);
            log_event(current_user, "chown", args[2], "success");
        }
        else if (strcmp(args[0], "chmod") == 0 && arg_count == 3) {
            chmod_vfs(args[1], args[2]);
            log_event(current_user, "chmod", args[1], "success");
        }
        else {
            printf("Unknown command: %s\n", args[0]);
            log_event(current_user, "unknown_command", args[0], "failed");
        }
    }
    return 0;
}
