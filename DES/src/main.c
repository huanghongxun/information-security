#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "des.h"
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

void generate_key(int argc, char *argv[])
{
    FILE *file = fopen(argv[2], "wb");
    uint64_t key = des_generate_key();
    char raw_key[8];
    split_uint64(key, raw_key);
    if (!file || fwrite(raw_key, 1, DES_KEY_SIZE, file) != DES_KEY_SIZE)
    {
        error("Unable to write key to given file %s", argv[2]);
    }
    fclose(file);
}

void encrypt(int argc, char *argv[])
{
    FILE *keyfile = fopen(argv[2], "rb");
    char raw_key[8];
    if (!keyfile || fread(raw_key, sizeof(unsigned char), DES_KEY_SIZE, keyfile) != DES_KEY_SIZE)
    {
        error("Unable to read key from file %s", argv[2]);
    }
    fclose(keyfile);
    uint64_t key = join_uint64(raw_key);
    if (!des_verify_key(key))
        error("Key in not valid");

    FILE *infile = fopen(argv[3], "rb");
    if (!infile)
        error("Unable to open input file %s", argv[3]);
    FILE *outfile = fopen(argv[4], "wb");
    if (!outfile)
        error("Unable to open output file %s", argv[4]);

    struct des_stream *stream = des_open(DES_ENCRYPT);

    char rbuf[1024], wbuf[1024];
    size_t rlen, wlen;
    while ((rlen = fread(rbuf, sizeof(char), 1024, infile)))
    {
        wlen = des(stream, rbuf, rlen, key, wbuf, 1024);
        fwrite(wbuf, sizeof(char), wlen, outfile);
    }

    if ((wlen = des_close(stream, key, wbuf, 1024)))
    {
        fwrite(wbuf, sizeof(char), wlen, outfile);
    }

    fclose(infile);
    fclose(outfile);
}

void decrypt(int argc, char *argv[])
{
    FILE *keyfile = fopen(argv[2], "rb");
    char raw_key[8];
    if (!keyfile || fread(raw_key, sizeof(unsigned char), DES_KEY_SIZE, keyfile) != DES_KEY_SIZE)
    {
        error("Unable to read key from file %s", argv[2]);
    }
    fclose(keyfile);
    uint64_t key = join_uint64(raw_key);
    if (!des_verify_key(key))
        error("Key in not valid");

    FILE *infile = fopen(argv[3], "rb");
    if (!infile)
        error("Unable to open input file %s", argv[3]);
    FILE *outfile = fopen(argv[4], "wb");
    if (!outfile)
        error("Unable to open output file %s", argv[4]);

    struct des_stream *stream = des_open(DES_DECRYPT);

    char rbuf[1024], wbuf[1024];
    size_t rlen, wlen;
    while ((rlen = fread(rbuf, sizeof(char), 1024, infile)))
    {
        wlen = des(stream, rbuf, rlen, key, wbuf, 1024);
        fwrite(wbuf, sizeof(char), wlen, outfile);
    }

    if ((wlen = des_close(stream, key, wbuf, 1024)))
    {
        fwrite(wbuf, sizeof(char), wlen, outfile);
    }

    fclose(infile);
    fclose(outfile);
}

struct optaction
{
    const char *opt;
    int optlen;
    const char *help;
    void (*action)(int, char *[]);
};

struct optaction actions[] = {
    {"generate-key", 1, "[key file]: generate a vaild DES key to given file", generate_key},
    {"encrypt", 3, "[key file] [plain file] [cipher file]: encrypt given input file by DES key", encrypt},
    {"decrypt", 3, "[key file] [cipher file] [decrypted file]: decrypte given file by DES key", decrypt},
    {NULL, 0, NULL, NULL}};

void usage(int argc, char *argv[])
{
    fprintf(stderr, "DES encryption and decryption\n");
    fprintf(stderr, "Usage: %s [command] [options...]\n\n", argv[0]);
    fprintf(stderr, "Commands:\n");
    for (struct optaction *p = actions; p->opt; p++)
    {
        fprintf(stderr, "  %s %s\n", p->opt, p->help);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        usage(argc, argv);
        return 1;
    }

    for (struct optaction *p = actions; p->opt; p++)
    {
        if (strcmp(argv[1], p->opt) == 0)
        {
            if (argc < p->optlen + 2)
            {
                fprintf(stderr, "Options: %s", p->help);
                return 1;
            }
            p->action(argc, argv);
            return 0;
        }
    }

    usage(argc, argv);
    return 1;
}