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
    int *h_counts = (int *)malloc(size_bytes);

    printf("--- Bemeneti tomb ---\n");
    for (int i = 0; i < N; i++)
    {
        h_data[i] = rand() % 20;
        printf("%d ", h_data[i]);
    }
    printf("\n\n");

    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_uint num;

    if (clGetPlatformIDs(1, &platform, &num) != CL_SUCCESS)
        return -1;
    if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &num) != CL_SUCCESS)
        return -1;

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

    int load_err;
    char *kernel_source = load_kernel_source("kernels/count_occurrences.cl", &load_err);
    if (load_err != 0)
    {
        printf("Hiba: Nem talalhato a kernels/count_occurrences.cl fajl!\n");
        return -1;
    }

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "count_occurrences", &err);

    cl_mem d_data = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size_bytes, h_data, &err);
    cl_mem d_counts = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size_bytes, NULL, &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_data);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_counts);
    clSetKernelArg(kernel, 2, sizeof(int), &N);

    size_t global_work_size[1] = {N};
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(queue, d_counts, CL_TRUE, 0, size_bytes, h_counts, 0, NULL, NULL);

    printf("--- Eredmeny (Ertek -> Elofordulas) ---\n");

    int is_unique = 1;

    for (int i = 0; i < N; i++)
    {
        printf("[%d] Ertek: %2d -> Szerepel: %d alkalommal\n", i, h_data[i], h_counts[i]);

        if (h_counts[i] > 1)
        {
            is_unique = 0;
        }
    }

    printf("\n--- Konkluzio ---\n");
    if (is_unique)
    {
        printf("A tomb minden eleme EGYEDI.\n");
    }
    else
    {
        printf("A tombben VANNAK DUPLIKATUMOK.\n");
    }

    free(kernel_source);
    free(h_data);
    free(h_counts);
    clReleaseMemObject(d_data);
    clReleaseMemObject(d_counts);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}