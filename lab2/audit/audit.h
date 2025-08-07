#ifndef AUDIT_H
#define AUDIT_H

void log_event(const char* user, const char* action, const char* target, const char* result);

#endif
