#include <cstdio>
#include <cstdarg>

void info(const char *format, ...) {
    char buf[500];
    va_list vlist;
    va_start(vlist, format);
    vsprintf(buf, format, vlist);
    va_end(vlist);
    fprintf(stderr, "INFO: %s\n", buf);
}

