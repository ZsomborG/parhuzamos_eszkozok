#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include "kernel_loader.h"

char *load_kernel_source(const char *filename, int *error_code)
{
    FILE *f = fopen(filename, "r");
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

cl_program build_program(cl_context context, cl_device_id device, const char *source_code)
{
    cl_int err;
    cl_program program = clCreateProgramWithSource(context, 1, &source_code, NULL, &err);
    if (err != CL_SUCCESS)
    {
        printf("[HIBA] Nem sikerult a program letrehozasa a forrasbol.\n");
        return NULL;
    }

    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    if (err != CL_SUCCESS)
    {
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        char *build_log = (char *)malloc(log_size);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);

        printf("\n================ FORDITASI HIBA ================\n");
        printf("%s\n", build_log);
        printf("================================================\n\n");

        free(build_log);
        clReleaseProgram(program);
        return NULL;
    }

    return program;
}