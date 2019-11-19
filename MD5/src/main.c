#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "md5.h"
#include "binary.h"

void error(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(2);
}

void usage(int argc, char *argv[])
{
    fprintf(stderr, "MD5 file hash\n");
    fprintf(stderr, "Usage: %s [file]\n\n", argv[0]);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        usage(argc, argv);
        return 1;
    }
    

    FILE *infile = fopen(argv[1], "rb");
    if (!infile)
        error("Unable to open input file %s", argv[3]);

    struct md5_stream stream;
    md5_stream_begin(&stream);

    uint8_t rbuf[1024];
    size_t rlen, wlen;
    while ((rlen = fread(rbuf, sizeof(uint8_t), 1024, infile)))
    {
        md5_stream_data(&stream, rbuf, rlen);
    }

    uint8_t result[16];
    md5_stream_end(&stream, result);
    fclose(infile);

    for (int i = 0; i < 16; ++i) printf("%02x", result[i]);

    printf("  %s\n", argv[1]);

    return 1;
}