#include <stdio.h>
#include <stdlib.h>
#include "kernel_loader.h"

char *load_kernel_source(const char *filename, int *error_code)
{
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        if (error_code)
            *error_code = -1;
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    rewind(f);

    char *source = (char *)malloc(len + 1);
    if (!source)
    {
        fclose(f);
        if (error_code)
            *error_code = -2;
        return NULL;
    }

    size_t read_size = fread(source, 1, len, f);
    source[read_size] = '\0';

    fclose(f);
    if (error_code)
        *error_code = 0;

    return source;
}