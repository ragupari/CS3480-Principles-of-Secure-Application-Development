#ifndef VFS_H
#define VFS_H

#include <stdio.h>

typedef struct File {
    char name[100];
    char owner[50];
    char group[50];        // ✅ NEW
    int permission;        // like 754
    char content[1024];
    struct File* next;
} File;

typedef struct Directory {
    char name[100];
    char owner[50];
    char group[50];        // ✅ NEW
    int permission;
    struct Directory* parent;
    struct Directory* subdirs;
    struct Directory* next;
    struct File* files;
} Directory;


// Global variables
extern Directory* root;
extern Directory* current_dir;
extern char current_user[50];

// Core FS functions
void init_fs();
void go_to_home_directory();

// Basic FS operations
void mkdir_vfs(const char* name);
void touch_vfs(const char* name);
void ls_vfs();
void ls_l_vfs();
void cd_vfs(const char* name);
void pwd_vfs();
void write_vfs(const char* name, const char* content);
void read_vfs(const char* name);

// Persistence
void save_vfs();
void load_vfs();

// Tree view
void tree();

// File operations
void rm_vfs(const char* name);
void rm_r_vfs(const char* name);

void chown_vfs(const char* new_owner, const char* new_group, const char* name);
// vfs.h
void chmod_vfs(const char* mode, const char* name);


#endif // VFS_H

