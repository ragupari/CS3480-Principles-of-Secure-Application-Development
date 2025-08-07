#ifndef USER_H
#define USER_H

#include <stdbool.h>

// Create a user and assign them to their own primary group
void adduser(const char *arg);

// Delete user from users.txt and remove them from all groups in groups.txt
void deluser(const char *username);

// Check if a user exists
int user_present(const char* username);

bool user_value_exists(const char* filename, const char* value);

// Check if a user belongs to a specific group
bool user_in_group(const char* username, const char* groupname);

// Get all groups for a user, returns count, fills the provided array
int get_user_groups(const char* username, char groups[][50]);

int get_user_type(const char* file_owner, const char* file_group, const char* username);

int has_permission(int perm, char mode, int user_type);

#endif // USER_H

