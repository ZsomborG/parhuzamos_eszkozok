#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#include "error_handler.h"
#include "kernel_loader.h"

int main(void)
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_uint num;

    clGetPlatformIDs(1, &platform, &num);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &num);
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

    char *source;
    cl_program program;

    // ==========================================================
    // 1. TESZT: Túl nagy méretű kernel
    // ==========================================================
    printf("--- TESZT 1: Tul nagy lokalis memoria ---\n");
    source = load_kernel_source("kernels/error_memory.cl", &err);
    program = build_program(context, device, source);

    if (program == NULL)
    {
        printf("Sikeresen elkaptuk a memoria limit tullepest!\n\n");
    }
    free(source);

    // ==========================================================
    // 2. TESZT: OpenCL C limitáció (Rekurzió)
    // ==========================================================
    printf("--- TESZT 2: Rekurzio ---\n");
    source = load_kernel_source("kernels/error_recursion.cl", &err);
    program = build_program(context, device, source);

    if (program == NULL)
    {
        printf("Sikeresen elkaptuk a rekurzio okozta forditasi hibat!\n\n");
    }
    free(source);

    // ==========================================================
    // 3. TESZT: Nullával osztás (Futási idejű)
    // ==========================================================
    printf("--- TESZT 3: Nullaval osztas ---\n");
    source = load_kernel_source("kernels/error_divzero.cl", &err);
    program = build_program(context, device, source);

    if (program != NULL)
    {
        printf("A program lefordult (Mivel a fordito nem tudja, hogy nullaval fogunk osztani).\n");

        cl_kernel kernel = clCreateKernel(program, "div_by_zero_kernel", &err);

        int host_data[1] = {0};
        cl_mem d_data = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int), NULL, &err);

        int divisor = 0;

        clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_data);
        clSetKernelArg(kernel, 1, sizeof(int), &divisor);

        size_t global_size = 1;
        printf("Futtatas inditasa 0 osztoval...\n");

        err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);

        clEnqueueReadBuffer(queue, d_data, CL_TRUE, 0, sizeof(int), host_data, 0, NULL, NULL);

        printf("Futtatas kesz. Az eredmeny erteke: %d\n", host_data[0]);

        clReleaseKernel(kernel);
        clReleaseMemObject(d_data);
        clReleaseProgram(program);
    }
    free(source);

    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}