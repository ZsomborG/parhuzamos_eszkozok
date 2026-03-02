#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <CL/cl.h>
#include "kernel_loader.h"

// Szabály: Két -1 nem kerülhet egymás mellé, és a szélső elemek sem lehetnek -1-ek.
void generate_gapped_input(int *data, int n)
{
    for (int i = 0; i < n; i++)
    {
        data[i] = i * 10;
    }

    for (int i = 1; i < n - 1; i++)
    {
        int should_remove = (rand() % 100) < 30;

        if (should_remove && data[i - 1] != -1)
        {
            data[i] = -1;
        }
    }
}

void print_array(const char *label, int *data, int n)
{
    printf("%s\n", label);
    for (int i = 0; i < n; i++)
    {
        if (data[i] == -1)
        {
            printf("  _  ");
        }
        else
        {
            printf("%4d ", data[i]);
        }
    }
    printf("\n\n");
}

int main(void)
{
    srand(time(NULL));

    const int N = 20;
    size_t size_bytes = N * sizeof(int);

    int *host_data = (int *)malloc(size_bytes);

    generate_gapped_input(host_data, N);
    print_array("--- Bemenet (Hianyzo elemek: '_') ---", host_data, N);

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
    char *kernel_source = load_kernel_source("kernels/fill_missing.cl", &load_err);
    if (load_err != 0)
    {
        printf("Nem talalhato a kernels/fill_missing.cl fajl!\n");
        return -1;
    }

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    cl_kernel kernel = clCreateKernel(program, "fill_missing", &err);
    if (err != CL_SUCCESS)
    {
        printf("Hiba a kernel letrehozasakor! Kod: %d\n", err);
        return -1;
    }

    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, size_bytes, host_data, &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    clSetKernelArg(kernel, 1, sizeof(int), &N);

    size_t global_work_size[1] = {N};

    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

    clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, size_bytes, host_data, 0, NULL, NULL);

    print_array("--- Kimenet (Potolt elemek) ---", host_data, N);

    free(kernel_source);
    free(host_data);
    clReleaseMemObject(buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}