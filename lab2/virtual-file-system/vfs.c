#include "vfs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include "../user-group-management/user.h"
#include "../user-group-management/group.h"

// === External state ===
Directory* root = NULL;
Directory* current_dir = NULL;
extern char current_user[50];

// === Prototypes we rely on (likely defined elsewhere) ===
int get_user_type(const char* owner, const char* group, const char* username);
int has_permission(int perm, char mode, int user_type);

// === Forward decls (local) ===
static void rm_file_vfs(const char* name);
static void rm_dir_vfs(const char* name);
static void rm_dir_recursive(Directory* dir);

// --- helpers ---
static Directory* find_subdir(Directory* parent, const char* name) {
    for (Directory* d = parent->subdirs; d; d = d->next) {
        if (strcmp(d->name, name) == 0) return d;
    }
    return NULL;
}
static File* find_file(Directory* parent, const char* name) {
    for (File* f = parent->files; f; f = f->next) {
        if (strcmp(f->name, name) == 0) return f;
    }
    return NULL;
}

// === Initialization ===
void init_fs() {
    root = (Directory*)malloc(sizeof(Directory));
    strcpy(root->name, "/");
    strcpy(root->owner, "root");
    strcpy(root->group, "root");
    root->permission = 755;
    root->parent = NULL;
    root->subdirs = NULL;
    root->files = NULL;
    root->next = NULL;

    // Create a real /home directory so paths & save/load are consistent
    Directory* home = (Directory*)malloc(sizeof(Directory));
    strcpy(home->name, "home");
    strcpy(home->owner, "root");
    strcpy(home->group, "root");
    home->permission = 755;
    home->parent = root;
    home->subdirs = NULL;
    home->files = NULL;
    home->next = NULL;
    root->subdirs = home;

    current_dir = home; // start at /home (caller can call go_to_home_directory)
}

void go_to_home_directory() {
    Directory* home = find_subdir(root, "home");
    if (!home) {
        // Shouldn't happen, but be defensive
        init_fs();
        home = find_subdir(root, "home");
    }
    for (Directory* d = home->subdirs; d; d = d->next) {
        if (strcmp(d->name, current_user) == 0) {
            current_dir = d;
            return;
        }
    }
    Directory* dir = (Directory*)malloc(sizeof(Directory));
    strcpy(dir->name, current_user);
    strcpy(dir->owner, current_user);
    strcpy(dir->group, current_user); // TODO: replace with primary group if you have it
    dir->permission = 700;
    dir->parent = home;
    dir->subdirs = NULL;
    dir->files = NULL;
    dir->next = home->subdirs;
    home->subdirs = dir;
    current_dir = dir;
}

// === File & Directory Operations ===
void mkdir_vfs(const char* name) {
    // Need w+x on current dir (Linux semantics)
    int tdir = get_user_type(current_dir->owner, current_dir->group, current_user);
    if (!has_permission(current_dir->permission, 'w', tdir) ||
        !has_permission(current_dir->permission, 'x', tdir)) {
        printf("Permission denied.\n");
        return;
    }

    if (find_subdir(current_dir, name)) {
        printf("mkdir: cannot create directory '%s': File exists\n", name);
        return;
    }
    if (find_file(current_dir, name)) {
        printf("mkdir: cannot create directory '%s': A file with the same name exists\n", name);
        return;
    }

    Directory* dir = (Directory*)malloc(sizeof(Directory));
    strcpy(dir->name, name);
    strcpy(dir->owner, current_user);
    strcpy(dir->group, current_user);
    dir->permission = 755;
    dir->parent = current_dir;
    dir->subdirs = NULL;
    dir->files = NULL;
    dir->next = current_dir->subdirs;
    current_dir->subdirs = dir;
    printf("Directory '%s' created.\n", name);
    save_vfs();
}

void touch_vfs(const char* name) {
    // Need w+x on current dir (create)
    int tdir = get_user_type(current_dir->owner, current_dir->group, current_user);
    if (!has_permission(current_dir->permission, 'w', tdir) ||
        !has_permission(current_dir->permission, 'x', tdir)) {
        printf("Permission denied.\n");
        return;
    }

    if (find_file(current_dir, name)) {
        printf("touch: cannot create file '%s': File exists\n", name);
        return;
    }
    if (find_subdir(current_dir, name)) {
        printf("touch: cannot create file '%s': A directory with the same name exists\n", name);
        return;
    }

    File* file = (File*)malloc(sizeof(File));
    strcpy(file->name, name);
    file->permission = 644;
    strcpy(file->owner, current_user);
    strcpy(file->group, current_user);
    file->content[0] = '\0';
    file->next = current_dir->files;
    current_dir->files = file;
    printf("File '%s' created.\n", name);
    save_vfs();
}

void write_vfs(const char* name, const char* content) {
    File* f = find_file(current_dir, name);
    if (!f) { printf("File not found.\n"); return; }

    int t = get_user_type(f->owner, f->group, current_user);
    if (!has_permission(f->permission, 'w', t)) {
        printf("Permission denied.\n");
        return;
    }
    // NOTE: ensure File.content is large enough in your header
    strcpy(f->content, content);
    printf("Content written to '%s'.\n", name);
}

void read_vfs(const char* name) {
    File* f = find_file(current_dir, name);
    if (!f) { printf("File not found.\n"); return; }

    int t = get_user_type(f->owner, f->group, current_user);
    if (!has_permission(f->permission, 'r', t)) {
        printf("Permission denied.\n");
        return;
    }
    printf("%s\n", f->content);
}

// === Navigation ===
void cd_vfs(const char* name) {
    if (strcmp(name, "/") == 0) { current_dir = root; return; }

    if (strcmp(name, "..") == 0) {
        if (current_dir->parent) current_dir = current_dir->parent;
        return;
    }

    Directory* dir = find_subdir(current_dir, name);
    if (!dir) { printf("Directory not found.\n"); return; }

    int t = get_user_type(dir->owner, dir->group, current_user);
    if (!has_permission(dir->permission, 'x', t)) {
        printf("Permission denied.\n");
        return;
    }
    current_dir = dir;
}

void pwd_vfs() {
    // Build absolute path by walking to root
    char path[1024] = "";
    Directory* temp = current_dir;

    // special case: root
    if (temp == root) {
        printf("/\n");
        return;
    }

    while (temp && temp != root) {
        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "/%s%s", temp->name, path);
        strncpy(path, buffer, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
        temp = temp->parent;
    }
    printf("%s\n", path[0] ? path : "/");
}

// === Display ===
static const char* permission_str(int perm, int is_dir) {
    static char str[11];
    str[0] = is_dir ? 'd' : '-';
    int owner = perm / 100;
    int group = (perm / 10) % 10;
    int other = perm % 10;
    str[1] = (owner & 4) ? 'r' : '-';
    str[2] = (owner & 2) ? 'w' : '-';
    str[3] = (owner & 1) ? 'x' : '-';
    str[4] = (group & 4) ? 'r' : '-';
    str[5] = (group & 2) ? 'w' : '-';
    str[6] = (group & 1) ? 'x' : '-';
    str[7] = (other & 4) ? 'r' : '-';
    str[8] = (other & 2) ? 'w' : '-';
    str[9] = (other & 1) ? 'x' : '-';
    str[10] = '\0';
    return str;
}

void ls_vfs() {
    // Need read (and usually execute) on the dir to list
    int tdir = get_user_type(current_dir->owner, current_dir->group, current_user);
    if (!has_permission(current_dir->permission, 'r', tdir)) {
        printf("Permission denied.\n");
        return;
    }
    for (Directory* dir = current_dir->subdirs; dir; dir = dir->next) {
        printf("[D] %s\n", dir->name);
    }
    for (File* file = current_dir->files; file; file = file->next) {
        printf("[F] %s\n", file->name);
    }
}

void ls_l_vfs() {
    int tdir = get_user_type(current_dir->owner, current_dir->group, current_user);
    if (!has_permission(current_dir->permission, 'r', tdir)) {
        printf("Permission denied.\n");
        return;
    }
    for (Directory* dir = current_dir->subdirs; dir; dir = dir->next) {
        printf("%s  %s  %s  %s\n", permission_str(dir->permission, 1), dir->owner, dir->group, dir->name);
    }
    for (File* file = current_dir->files; file; file = file->next) {
        printf("%s  %s  %s  %s\n", permission_str(file->permission, 0), file->owner, file->group, file->name);
    }
}

static void tree_recursive(Directory* dir, int depth) {
    for (int i = 0; i < depth; ++i) printf("  ");
    printf("[D] %s\n", dir->name);
    for (File* f = dir->files; f; f = f->next) {
        for (int i = 0; i < depth + 1; ++i) printf("  ");
        printf("[F] %s\n", f->name);
    }
    for (Directory* sub = dir->subdirs; sub; sub = sub->next) {
        tree_recursive(sub, depth + 1);
    }
}

void tree() {
    int tdir = get_user_type(current_dir->owner, current_dir->group, current_user);
    if (!has_permission(current_dir->permission, 'r', tdir)) {
        printf("Permission denied.\n");
        return;
    }
    tree_recursive(current_dir, 0);
}

// === Save/Load ===
static void save_vfs_recursive(FILE* fp, Directory* dir, const char* path) {
    char full_path[1024];
    if (path[0] == '\0') {
        snprintf(full_path, sizeof(full_path), "/%s", dir->name);
    } else {
        snprintf(full_path, sizeof(full_path), "%s/%s", path, dir->name);
    }

    fprintf(fp, "DIR %s %s %s %d\n", full_path, dir->owner, dir->group, dir->permission);

    for (File* f = dir->files; f; f = f->next) {
        // NOTE: content with spaces will be split; keeping your original format
        fprintf(fp, "FILE %s/%s %s %s %d %s\n", full_path, f->name, f->owner, f->group, f->permission, f->content);
    }
    for (Directory* sub = dir->subdirs; sub; sub = sub->next) {
        save_vfs_recursive(fp, sub, full_path);
    }
}

void save_vfs() {
    FILE* fp = fopen("vfs.txt", "w"); // change to "a" if you want append
    if (!fp) {
        perror("Failed to open save file");
        return;
    }
    for (Directory* d = root->subdirs; d; d = d->next) {
        save_vfs_recursive(fp, d, "");
    }
    fclose(fp);
}

void load_vfs() {
    FILE* fp = fopen("vfs.txt", "r");
    if (!fp) return;

    char type[10], path[1024], owner[50], group[50], content[1024];
    int perm;

    while (fscanf(fp, "%9s", type) == 1) {
        if (strcmp(type, "DIR") == 0) {
            if (fscanf(fp, "%1023s %49s %49s %d", path, owner, group, &perm) != 4) break;

            char path_copy[1024];
            strncpy(path_copy, path, sizeof(path_copy) - 1);
            path_copy[sizeof(path_copy) - 1] = '\0';

            char* tok = strtok(path_copy, "/");
            Directory* dir = root;
            while (tok) {
                Directory* next = find_subdir(dir, tok);
                if (!next) {
                    next = (Directory*)malloc(sizeof(Directory));
                    strcpy(next->name, tok);
                    // Use provided meta only when creating the leaf; OK to keep for all levels in this simplified model
                    strcpy(next->owner, owner);
                    strcpy(next->group, group);
                    next->permission = perm;
                    next->parent = dir;
                    next->subdirs = NULL;
                    next->files = NULL;
                    next->next = dir->subdirs;
                    dir->subdirs = next;
                }
                dir = next;
                tok = strtok(NULL, "/");
            }
        } else if (strcmp(type, "FILE") == 0) {
            // Read the rest of the line after "FILE "
            char line[2048];
            if (!fgets(line, sizeof(line), fp)) continue;
            // Trim leading spaces
            char* s = line;
            while (*s == ' ' || *s == '\t') ++s;
            // Parse first 4 tokens: path owner group perm
            if (sscanf(s, "%1023s %49s %49s %d", path, owner, group, &perm) < 4) continue;

            // Move pointer past first 4 tokens to get content (may contain spaces)
            int tokens = 0;
            while (*s && tokens < 4) {
                if (*s == ' ') { ++tokens; while (*s == ' ') ++s; }
                else ++s;
            }
            // s now points somewhere mid; find start of content
            char* content_start = strchr(s, ' ');
            if (content_start) {
                while (*content_start == ' ') ++content_start;
                strncpy(content, content_start, sizeof(content) - 1);
                content[sizeof(content) - 1] = '\0';
                // strip trailing newline
                content[strcspn(content, "\r\n")] = 0;
            } else {
                content[0] = '\0';
            }

            char* base = strrchr(path, '/');
            if (!base) continue;
            *base = '\0';
            char* fname = base + 1;

            Directory* dir = root;
            char path_copy[1024];
            strncpy(path_copy, path, sizeof(path_copy) - 1);
            path_copy[sizeof(path_copy) - 1] = '\0';

            char* p = strtok(path_copy, "/");
            while (p) {
                Directory* next = find_subdir(dir, p);
                if (!next) { // shouldn't be missing if DIR lines were processed, but be robust
                    next = (Directory*)malloc(sizeof(Directory));
                    strcpy(next->name, p);
                    strcpy(next->owner, "X");
                    strcpy(next->group, "X");
                    next->permission = 755;
                    next->parent = dir;
                    next->subdirs = NULL;
                    next->files = NULL;
                    next->next = dir->subdirs;
                    dir->subdirs = next;
                }
                dir = next;
                p = strtok(NULL, "/");
            }

            File* f = (File*)malloc(sizeof(File));
            strcpy(f->name, fname);
            strcpy(f->owner, owner);
            strcpy(f->group, group);
            f->permission = perm;
            strcpy(f->content, content);
            f->next = dir->files;
            dir->files = f;
        } else {
            // Unknown line; skip rest of line
            int c;
            while ((c = fgetc(fp)) != '\n' && c != EOF) {}
        }
    }
    fclose(fp);
}

// === Remove ===
void rm_vfs(const char* name) {
    // POSIX semantics: need w+x on parent directory to unlink
    File* f = find_file(current_dir, name);
    if (!f) {
        printf("'%s' is not a file. Use -r to remove directory.\n", name);
        return;
    }

    int tparent = get_user_type(current_dir->owner, current_dir->group, current_user);
    if (!has_permission(current_dir->permission, 'w', tparent) ||
        !has_permission(current_dir->permission, 'x', tparent)) {
        printf("Permission denied.\n");
        return;
    }
    rm_file_vfs(name);
}

void rm_r_vfs(const char* name) {
    Directory* d = find_subdir(current_dir, name);
    if (!d) { printf("'%s' is not a directory.\n", name); return; }

    // Need w+x on parent to remove the entry (target perms irrelevant in classic DAC)
    int tparent = get_user_type(current_dir->owner, current_dir->group, current_user);
    if (!has_permission(current_dir->permission, 'w', tparent) ||
        !has_permission(current_dir->permission, 'x', tparent)) {
        printf("Permission denied.\n");
        return;
    }
    rm_dir_vfs(name);
}

static void rm_file_vfs(const char* name) {
    File** prev = &current_dir->files;
    while (*prev) {
        File* f = *prev;
        if (strcmp(f->name, name) == 0) {
            *prev = f->next;
            free(f);
            printf("File '%s' removed.\n", name);
            save_vfs(); // save after removal       
            return;
        }
        prev = &(*prev)->next;
    }
    printf("File not found.\n");
}

static void rm_dir_recursive(Directory* dir) {
    while (dir->files) {
        File* f = dir->files;
        dir->files = f->next;
        free(f);
    }
    while (dir->subdirs) {
        Directory* d = dir->subdirs;
        dir->subdirs = d->next;
        rm_dir_recursive(d);
    }
    free(dir);
}

static void rm_dir_vfs(const char* name) {
    Directory** prev = &current_dir->subdirs;
    while (*prev) {
        Directory* d = *prev;
        if (strcmp(d->name, name) == 0) {
            *prev = d->next;     // unlink from sibling list
            rm_dir_recursive(d); // free all children and dir itself
            printf("Directory '%s' removed.\n", name);
            save_vfs(); // save after removal
            return;
        }
        prev = &(*prev)->next;
    }
    printf("Directory not found.\n");
}

// === Ownership (kept as in your version, with minor safety) ===
void chown_vfs(const char* new_owner, const char* new_group, const char* name) {
    void* target = NULL;
    int is_dir = 0;

    Directory* d = find_subdir(current_dir, name);
    if (d) { target = d; is_dir = 1; }
    else {
        File* f = find_file(current_dir, name);
        if (f) { target = f; is_dir = 0; }
    }
    if (!target) {
        printf("chown: cannot access '%s': No such file or directory\n", name);
        return;
    }

    char* owner = is_dir ? ((Directory*)target)->owner : ((File*)target)->owner;
    char* group = is_dir ? ((Directory*)target)->group : ((File*)target)->group;

    // Owner change – root only
    if (new_owner && *new_owner) {
        if (strcmp(current_user, "root") != 0) {
            printf("chown: changing owner of '%s': Operation not permitted\n", name);
            return;
        }
        strcpy(owner, new_owner);
    }

    // Group change – root OR owner in target group
    if (new_group && *new_group) {
        if (strcmp(current_user, "root") != 0) {
            if (strcmp(owner, current_user) != 0 || !user_in_group(current_user, new_group)) {
                printf("chown: changing group of '%s': Operation not permitted\n", name);
                return;
            }
        }
        strcpy(group, new_group);
    }

    printf("Ownership of '%s' changed to %s:%s\n", name, owner, group);
}
// ===== CHMOD helpers =====
static void split_perm(int perm, int* u, int* g, int* o) {
    *u = perm / 100;
    *g = (perm / 10) % 10;
    *o = perm % 10;
}
static int join_perm(int u, int g, int o) {
    return (u % 8) * 100 + (g % 8) * 10 + (o % 8);
}
static int is_all_octal_digits(const char* s) {
    if (!s || !*s) return 0;
    for (const char* p = s; *p; ++p) if (*p < '0' || *p > '7') return 0;
    return 1;
}

// Apply one symbolic clause like "u+rwx" or "go-w" or "o=rx" to u/g/o digits
static void apply_symbolic_clause(const char* who, char op, const char* rwx, int* u, int* g, int* o) {
    int mask = 0;
    for (const char* p = rwx; *p; ++p) {
        if (*p == 'r') mask |= 4;
        else if (*p == 'w') mask |= 2;
        else if (*p == 'x') mask |= 1;
        // ignore s/t for this simplified FS
    }

    int apply_u = 0, apply_g = 0, apply_o = 0;
    if (!*who) { apply_u = apply_g = apply_o = 1; } // default to "a"
    else {
        for (const char* p = who; *p; ++p) {
            if (*p == 'u') apply_u = 1;
            else if (*p == 'g') apply_g = 1;
            else if (*p == 'o') apply_o = 1;
            else if (*p == 'a') apply_u = apply_g = apply_o = 1;
        }
    }

    if (op == '+') {
        if (apply_u) *u = (*u | mask) & 7;
        if (apply_g) *g = (*g | mask) & 7;
        if (apply_o) *o = (*o | mask) & 7;
    } else if (op == '-') {
        if (apply_u) *u = (*u & ~mask) & 7;
        if (apply_g) *g = (*g & ~mask) & 7;
        if (apply_o) *o = (*o & ~mask) & 7;
    } else if (op == '=') {
        if (apply_u) *u = mask & 7;
        if (apply_g) *g = mask & 7;
        if (apply_o) *o = mask & 7;
    }
}

// Parse a symbolic mode like "u+rwx,g-w,o=rx" and update the triplet
static int parse_symbolic_mode(const char* mode, int* u, int* g, int* o) {
    // We will destructively copy to tokenize by commas
    char buf[128];
    strncpy(buf, mode, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* clause = strtok(buf, ",");
    while (clause) {
        // clause pattern: [ugoa]* [+-=] [rwx]+
        // find op
        char op = 0;
        char* p = clause;
        while (*p) {
            if (*p == '+' || *p == '-' || *p == '=') { op = *p; break; }
            ++p;
        }
        if (!op) return 0; // invalid

        // split who/op/rwx
        // who: clause[0..p-1], rwx: p+1..end
        char who[8] = {0};
        if (p != clause) {
            size_t n = (size_t)(p - clause);
            if (n >= sizeof(who)) n = sizeof(who) - 1;
            memcpy(who, clause, n);
            who[n] = '\0';
        }
        const char* rwx = p + 1;
        if (!*rwx) return 0;

        apply_symbolic_clause(who, op, rwx, u, g, o);

        clause = strtok(NULL, ",");
    }
    return 1;
}

// === Public: chmod ===
// Only the owner or root can change mode. NAME is looked up in current_dir.
void chmod_vfs(const char* mode, const char* name) {
    if (!mode || !name || !*mode || !*name) {
        printf("chmod: missing operand\n");
        return;
    }

    // find target (file or dir) in current_dir
    Directory* d = NULL;
    File* f = NULL;
    for (Directory* it = current_dir->subdirs; it; it = it->next) {
        if (strcmp(it->name, name) == 0) { d = it; break; }
    }
    if (!d) {
        for (File* it = current_dir->files; it; it = it->next) {
            if (strcmp(it->name, name) == 0) { f = it; break; }
        }
    }
    if (!d && !f) {
        printf("chmod: cannot access '%s': No such file or directory\n", name);
        return;
    }

    // Ownership check: root or owner
    const char* owner = d ? d->owner : f->owner;
    if (strcmp(current_user, "root") != 0 && strcmp(owner, current_user) != 0) {
        printf("chmod: changing permissions of '%s': Operation not permitted\n", name);
        return;
    }

    // Work with triplet
    int u, g, o;
    int perm = d ? d->permission : f->permission;
    split_perm(perm, &u, &g, &o);

    // Numeric mode? (e.g., "755", "0644")
    if (is_all_octal_digits(mode)) {
        const char* s = mode;
        if (strlen(mode) == 4 && mode[0] == '0') s = mode + 1; // allow leading 0
        if (strlen(s) != 3) {
            printf("chmod: invalid mode: '%s'\n", mode);
            return;
        }
        int nu = s[0] - '0', ng = s[1] - '0', no = s[2] - '0';
        if (nu > 7 || ng > 7 || no > 7) {
            printf("chmod: invalid mode: '%s'\n", mode);
            return;
        }
        u = nu; g = ng; o = no;
    } else {
        // Symbolic mode
        if (!parse_symbolic_mode(mode, &u, &g, &o)) {
            printf("chmod: invalid mode: '%s'\n", mode);
            return;
        }
    }

    int newperm = join_perm(u, g, o);
    if (d) d->permission = newperm;
    else    f->permission = newperm;

    printf("mode of '%s' changed to %03d\n", name, newperm);
}