#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <CL/cl.h>

#include "error_handler.h"

int main(void)
{
    cl_int err;
    cl_uint num_platforms;

    printf("Helyes hivas tesztelese:\n");
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    printf("Eredmeny: %s\n\n", get_error_string(err));

    printf("Helytelen hivas tesztelese:\n");
    err = clGetPlatformIDs(0, NULL, NULL);

    check_error(err, "clGetPlatformIDs (szandekos hiba teszt)");

    return 0;
}