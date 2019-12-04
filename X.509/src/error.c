#include "error.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

void error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, ": %s\n", strerror(errno));
    va_end(ap);
    exit(2);
}