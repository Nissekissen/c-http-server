#include "utils.h"

char *strdup(const char *s)
{
    size_t len = strlen(s) + 1;
    char *p = (char*) malloc(len);
    if (p) {
        memcpy(p, s, len);
    }
    return p;
}
