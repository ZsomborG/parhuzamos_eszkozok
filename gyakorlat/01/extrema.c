#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CL/cl.h>
#include "kernel_loader.h"

int main(void)
{
    srand(time(NULL));

    const int N = 10;
    size_t size_bytes = N * sizeof(int);

    int *h_data = (int *)malloc(size_bytes);
    int h_result = 0;

    printf("--- Bemeneti tomb ---\n");
    for (int i = 0; i < N; i++)
    {
        h_data[i] = rand() % 100;
        printf("%d ", h_data[i]);
    }
    printf("\n\n");

    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_uint num;

    clGetPlatformIDs(1, &platform, &num);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &num);

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

    int load_err;
    char *kernel_source = load_kernel_source("kernels/extrema.cl", &load_err);
    if (load_err != 0)
    {
        printf("Hiba: Nem talalhato a kernels/extrema.cl fajl!\n");
        return -1;
    }

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    cl_kernel k_init = clCreateKernel(program, "init_candidates", &err);
    cl_kernel k_comp = clCreateKernel(program, "compare_triangle", &err);
    cl_kernel k_res = clCreateKernel(program, "get_result", &err);

    cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size_bytes, h_data, &err);
    cl_mem d_B = clCreateBuffer(context, CL_MEM_READ_WRITE, size_bytes, NULL, &err);
    cl_mem d_result = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int), NULL, &err);

    clSetKernelArg(k_init, 0, sizeof(cl_mem), &d_B);
    clSetKernelArg(k_init, 1, sizeof(int), &N);
    size_t global_1D[1] = {N};
    clEnqueueNDRangeKernel(queue, k_init, 1, NULL, global_1D, NULL, 0, NULL, NULL);

    clSetKernelArg(k_comp, 0, sizeof(cl_mem), &d_data);
    clSetKernelArg(k_comp, 1, sizeof(cl_mem), &d_B);
    clSetKernelArg(k_comp, 2, sizeof(int), &N);
    size_t global_2D[2] = {N, N};
    clEnqueueNDRangeKernel(queue, k_comp, 2, NULL, global_2D, NULL, 0, NULL, NULL);

    clSetKernelArg(k_res, 0, sizeof(cl_mem), &d_data);
    clSetKernelArg(k_res, 1, sizeof(cl_mem), &d_B);
    clSetKernelArg(k_res, 2, sizeof(cl_mem), &d_result);
    clSetKernelArg(k_res, 3, sizeof(int), &N);
    clEnqueueNDRangeKernel(queue, k_res, 1, NULL, global_1D, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(queue, d_result, CL_TRUE, 0, sizeof(int), &h_result, 0, NULL, NULL);

    printf("A tomb MAXIMALIS eleme: %d\n", h_result);

    free(kernel_source);
    free(h_data);
    clReleaseMemObject(d_data);
    clReleaseMemObject(d_B);
    clReleaseMemObject(d_result);
    clReleaseKernel(k_init);
    clReleaseKernel(k_comp);
    clReleaseKernel(k_res);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}