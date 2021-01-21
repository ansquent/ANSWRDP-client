#include <cstdio>
#include <iostream>
#include <cstdarg>

void info(const char *format, ...)
{
    char buf[500]; int i;
    va_list vlist;
    va_start(vlist,format);
    vsprintf(buf, format, vlist);
    va_end(vlist);
    std::cerr << " INFO: " << buf << std::endl;
}
