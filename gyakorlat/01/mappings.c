#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include "kernel_loader.h"

void print_result(const char *label, int *data, int size)
{
    printf("\n--- %s ---\n", label);
    for (int i = 0; i < size; i++)
    {
        printf("[%d]: %d  ", i, data[i]);
    }
    printf("\n");
}

int main(void)
{
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_uint num_platforms, num_devices;

    if (clGetPlatformIDs(1, &platform, &num_platforms) != CL_SUCCESS)
    {
        printf("Hiba: Nincs OpenCL platform.\n");
        return -1;
    }
    if (clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, &num_devices) != CL_SUCCESS)
    {
        printf("Hiba: Nincs OpenCL eszkoz.\n");
        return -1;
    }

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

    int load_err;
    char *kernel_source = load_kernel_source("kernels/mapping.cl", &load_err);
    if (load_err != 0)
    {
        printf("Hiba a kernel fajl betoltesekor!\n");
        return -1;
    }

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    if (err != CL_SUCCESS)
    {
        char build_log[16384];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(build_log), build_log, NULL);
        printf("Build Log:\n%s\n", build_log);
        return -1;
    }

    const int DATA_SIZE = 10;
    size_t datasize_bytes = sizeof(int) * DATA_SIZE;
    int *host_data = (int *)malloc(datasize_bytes);

    cl_mem buffer = clCreateBuffer(context, CL_MEM_READ_WRITE, datasize_bytes, NULL, &err);

    size_t global_work_size[1] = {DATA_SIZE};

    // 1. FELADAT: Indexek kiírása (write_indices)
    printf("1. Kernel futtatasa: write_indices...\n");
    cl_kernel k_indices = clCreateKernel(program, "write_indices", &err);

    clSetKernelArg(k_indices, 0, sizeof(cl_mem), &buffer);

    clEnqueueNDRangeKernel(queue, k_indices, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, datasize_bytes, host_data, 0, NULL, NULL);

    print_result("Indexek (Identity)", host_data, DATA_SIZE);
    clReleaseKernel(k_indices);

    // 2. FELADAT: Fordított sorrend (write_reverse)
    printf("2. Kernel futtatasa: write_reverse...\n");
    cl_kernel k_reverse = clCreateKernel(program, "write_reverse", &err);

    clSetKernelArg(k_reverse, 0, sizeof(cl_mem), &buffer);
    clSetKernelArg(k_reverse, 1, sizeof(int), &DATA_SIZE);

    clEnqueueNDRangeKernel(queue, k_reverse, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, datasize_bytes, host_data, 0, NULL, NULL);

    print_result("Forditott sorrend", host_data, DATA_SIZE);
    clReleaseKernel(k_reverse);

    // 3. FELADAT: Szomszédok cseréje (swap_adjacent)
    printf("3. Kernel futtatasa: swap_adjacent...\n");
    cl_kernel k_swap = clCreateKernel(program, "swap_adjacent", &err);

    clSetKernelArg(k_swap, 0, sizeof(cl_mem), &buffer);

    clEnqueueNDRangeKernel(queue, k_swap, 1, NULL, global_work_size, NULL, 0, NULL, NULL);
    clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, datasize_bytes, host_data, 0, NULL, NULL);

    print_result("Szomszedcsere (Swap)", host_data, DATA_SIZE);
    clReleaseKernel(k_swap);

    free(kernel_source);
    free(host_data);
    clReleaseMemObject(buffer);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}