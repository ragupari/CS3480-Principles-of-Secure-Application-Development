#ifndef USERMOD_H
#define USERMOD_H

// Create a user and assign them to multiple groups
void usermod_append_group(const char *username, const char *groupname);
#endif // USERMOD_H
