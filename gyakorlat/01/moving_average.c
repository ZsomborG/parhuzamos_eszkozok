#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

#include "kernel_loader.h"

int main(void)
{
    const int N = 15;
    const int RADIUS = 2;
    size_t size_bytes = N * sizeof(float);

    float *h_input = (float *)malloc(size_bytes);
    float *h_output = (float *)malloc(size_bytes);

    for (int i = 0; i < N; i++)
    {
        h_input[i] = 10.0f;
    }
    h_input[7] = 100.0f;

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
    char *kernel_source = load_kernel_source("kernels/moving_average.cl", &load_err);
    if (load_err != 0)
    {
        printf("Hiba: Nem talalhato a kernels/moving_average.cl fajl!\n");
        return -1;
    }

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "moving_average", &err);

    cl_mem d_input = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, size_bytes, h_input, &err);
    cl_mem d_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, size_bytes, NULL, &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &d_input);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &d_output);
    clSetKernelArg(kernel, 2, sizeof(int), &N);
    clSetKernelArg(kernel, 3, sizeof(int), &RADIUS);

    size_t global_work_size[1] = {N};
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(queue, d_output, CL_TRUE, 0, size_bytes, h_output, 0, NULL, NULL);

    printf("Cszusoatlag sugar (radius): %d\n\n", RADIUS);
    printf("Index | Eredeti Ertek | Szurt Ertek (Atlag)\n");
    printf("-------------------------------------------\n");
    for (int i = 0; i < N; i++)
    {
        printf("[%2d] | %13.1f | %19.1f\n", i, h_input[i], h_output[i]);
    }

    free(kernel_source);
    free(h_input);
    free(h_output);
    clReleaseMemObject(d_input);
    clReleaseMemObject(d_output);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}