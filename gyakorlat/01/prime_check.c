#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>

#include "kernel_loader.h"

unsigned int generate_primes(unsigned int max_val, unsigned int **out_primes)
{
    char *is_p = (char *)malloc(max_val + 1);
    for (unsigned int i = 0; i <= max_val; i++)
        is_p[i] = 1;

    for (unsigned int p = 2; p * p <= max_val; p++)
    {
        if (is_p[p])
        {
            for (unsigned int i = p * p; i <= max_val; i += p)
                is_p[i] = 0;
        }
    }

    unsigned int count = 0;
    for (unsigned int p = 2; p <= max_val; p++)
    {
        if (is_p[p])
            count++;
    }

    *out_primes = (unsigned int *)malloc(count * sizeof(unsigned int));
    unsigned int idx = 0;
    for (unsigned int p = 2; p <= max_val; p++)
    {
        if (is_p[p])
            (*out_primes)[idx++] = p;
    }

    free(is_p);
    return count;
}

int main(void)
{
    // Vizsgálandó szám: a Mersenne prím (2^31 - 1)
    unsigned int N = 2147483647;
    unsigned int limit = (unsigned int)sqrt(N);

    printf("Vizsgalando szam: %u (gyoke kb. %u)\n\n", N, limit);

    int host_is_prime;
    int true_val = 1;

    unsigned int *cpu_primes = NULL;
    unsigned int num_primes = generate_primes(limit, &cpu_primes);
    printf("A CPU legeneralta a primeket %u-ig. (Osszesen: %u db)\n\n", limit, num_primes);

    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_uint num;

    clGetPlatformIDs(1, &platform, &num);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &num);
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

    int load_err;
    char *kernel_source = load_kernel_source("kernels/prime_check.cl", &load_err);
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    cl_kernel k_single = clCreateKernel(program, "check_single", &err);
    cl_kernel k_range = clCreateKernel(program, "check_range", &err);
    cl_kernel k_precalc = clCreateKernel(program, "check_precalculated", &err);

    cl_mem d_is_prime = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int), NULL, &err);
    cl_mem d_primes = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                     num_primes * sizeof(unsigned int), cpu_primes, &err);

    // ====================================================
    // MÓDSZER 1: Egy szál - Egy osztó
    // ====================================================
    clEnqueueWriteBuffer(queue, d_is_prime, CL_TRUE, 0, sizeof(int), &true_val, 0, NULL, NULL);

    clSetKernelArg(k_single, 0, sizeof(unsigned int), &N);
    clSetKernelArg(k_single, 1, sizeof(cl_mem), &d_is_prime);

    size_t global_size_1[1] = {limit}; // kb. 46340 szál
    clEnqueueNDRangeKernel(queue, k_single, 1, NULL, global_size_1, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, d_is_prime, CL_TRUE, 0, sizeof(int), &host_is_prime, 0, NULL, NULL);

    printf("1. Modszer (Egy oszto/szal):  Eredmeny: %s\n", host_is_prime ? "PRIM" : "NEM PRIM");

    // ====================================================
    // MÓDSZER 2: Egy szál - Tartomány
    // ====================================================
    clEnqueueWriteBuffer(queue, d_is_prime, CL_TRUE, 0, sizeof(int), &true_val, 0, NULL, NULL);

    unsigned int range_size = 100;
    unsigned int needed_threads = (limit / range_size) + 1; // kb. 464 szál

    clSetKernelArg(k_range, 0, sizeof(unsigned int), &N);
    clSetKernelArg(k_range, 1, sizeof(cl_mem), &d_is_prime);
    clSetKernelArg(k_range, 2, sizeof(unsigned int), &range_size);

    size_t global_size_2[1] = {needed_threads};
    clEnqueueNDRangeKernel(queue, k_range, 1, NULL, global_size_2, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, d_is_prime, CL_TRUE, 0, sizeof(int), &host_is_prime, 0, NULL, NULL);

    printf("2. Modszer (Tartomany/szal):  Eredmeny: %s\n", host_is_prime ? "PRIM" : "NEM PRIM");

    // ====================================================
    // MÓDSZER 3: Előre kiosztott prímek
    // ====================================================
    clEnqueueWriteBuffer(queue, d_is_prime, CL_TRUE, 0, sizeof(int), &true_val, 0, NULL, NULL);

    clSetKernelArg(k_precalc, 0, sizeof(unsigned int), &N);
    clSetKernelArg(k_precalc, 1, sizeof(cl_mem), &d_is_prime);
    clSetKernelArg(k_precalc, 2, sizeof(cl_mem), &d_primes);
    clSetKernelArg(k_precalc, 3, sizeof(unsigned int), &num_primes);

    size_t global_size_3[1] = {num_primes}; // kb. 4792 szál
    clEnqueueNDRangeKernel(queue, k_precalc, 1, NULL, global_size_3, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, d_is_prime, CL_TRUE, 0, sizeof(int), &host_is_prime, 0, NULL, NULL);

    printf("3. Modszer (Csak primekkel):  Eredmeny: %s\n", host_is_prime ? "PRIM" : "NEM PRIM");

    free(kernel_source);
    free(cpu_primes);
    clReleaseMemObject(d_is_prime);
    clReleaseMemObject(d_primes);
    clReleaseKernel(k_single);
    clReleaseKernel(k_range);
    clReleaseKernel(k_precalc);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}