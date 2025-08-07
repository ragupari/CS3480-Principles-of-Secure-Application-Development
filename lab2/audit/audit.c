#include <stdio.h>
#include <time.h>
#include "audit.h"

void log_event(const char* user, const char* action, const char* target, const char* result) {
    FILE* fp = fopen("audit.log", "a");
    if (!fp) return;

    time_t now = time(NULL);
    char ts[64];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(fp, "%s | user=%s | action=%s | target=%s | result=%s\n",
            ts,
            user && *user ? user : "(none)",
            action && *action ? action : "(none)",
            target && *target ? target : "(none)",
            result && *result ? result : "(none)");

    fclose(fp);
}
