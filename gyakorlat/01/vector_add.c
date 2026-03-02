#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>
#include "kernel_loader.h"

int compute_vector_add_opencl(float *h_a, float *h_b, float *h_c, int n)
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_uint num;

    if (clGetPlatformIDs(1, &platform, &num) != CL_SUCCESS)
        return -1;
    if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &num) != CL_SUCCESS)
        return -2;

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

    int load_err;
    char *source = load_kernel_source("kernels/vector_add.cl", &load_err);
    if (load_err != 0)
        return -3;

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "vector_add", &err);

    size_t bytes = sizeof(float) * n;
    cl_mem d_a = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    cl_mem d_b = clCreateBuffer(context, CL_MEM_READ_ONLY, bytes, NULL, NULL);
    cl_mem d_c = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bytes, NULL, NULL);

    clEnqueueWriteBuffer(queue, d_a, CL_TRUE, 0, bytes, h_a, 0, NULL, NULL);
    clEnqueueWriteBuffer(queue, d_b, CL_TRUE, 0, bytes, h_b, 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_a);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_b);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &d_c);
    clSetKernelArg(kernel, 3, sizeof(int), &n);

    size_t global_size = n;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(queue, d_c, CL_TRUE, 0, bytes, h_c, 0, NULL, NULL);

    clReleaseMemObject(d_a);
    clReleaseMemObject(d_b);
    clReleaseMemObject(d_c);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(source);

    return 0;
}

int verify_results(float *A, float *B, float *C, int n)
{
    int errors = 0;
    const float EPSILON = 0.0001f;

    for (int i = 0; i < n; i++)
    {
        float expected = A[i] + B[i];
        if (fabs(C[i] - expected) > EPSILON)
        {
            errors++;
            if (errors < 5)
            {
                printf("[HIBA] Index %d: GPU=%.4f, CPU=%.4f\n", i, C[i], expected);
            }
        }
    }
    return errors;
}

int main(void)
{
    const int N = 10000;
    size_t size_bytes = N * sizeof(float);

    float *h_a = (float *)malloc(size_bytes);
    float *h_b = (float *)malloc(size_bytes);
    float *h_c = (float *)malloc(size_bytes);

    for (int i = 0; i < N; i++)
    {
        h_a[i] = (float)i;
        h_b[i] = (float)(N - i);
        h_c[i] = 0.0f;
    }

    printf("Vektor osszeadas inditasa %d elemen...\n", N);

    int status = compute_vector_add_opencl(h_a, h_b, h_c, N);

    if (status != 0)
    {
        printf("Hiba tortent az OpenCL futtatasa kozben! Kod: %d\n", status);
        return -1;
    }

    printf("OpenCL szamitas kesz.\n");

    printf("Eredmenyek ellenorzese...\n");
    int errors = verify_results(h_a, h_b, h_c, N);

    if (errors == 0)
    {
        printf("SIKER! A GPU es a CPU eredmenye megegyezik.\n");
        printf("Minta: %0.1f + %0.1f = %0.1f\n", h_a[0], h_b[0], h_c[0]);
        printf("Minta: %0.1f + %0.1f = %0.1f\n", h_a[N - 1], h_b[N - 1], h_c[N - 1]);
    }
    else
    {
        printf("HIBA! Osszesen %d elteres talalhato.\n", errors);
    }

    free(h_a);
    free(h_b);
    free(h_c);

    return 0;
}