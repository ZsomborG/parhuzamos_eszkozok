#define CL_TARGET_OPENCL_VERSION 220
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

char *load_kernel_source(const char *filename, int *error_code);

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
    char *kernel_source = load_kernel_source("kernels/hello_kernel.cl", &load_err);
    if (load_err != 0)
    {
        printf("Hiba a kernel betoltesekor!\n");
        return -1;
    }

    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernel_source, NULL, &err);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    cl_kernel kernel = clCreateKernel(program, "hello_kernel", &err);
    if (err != CL_SUCCESS)
    {
        printf("Hiba a kernel letrehozasakor! Hibakod: %d\n", err);
        return -1;
    }

    const int DATA_SIZE = 10;
    size_t datasize_bytes = sizeof(int) * DATA_SIZE;

    int *host_data = (int *)malloc(datasize_bytes);

    for (int i = 0; i < DATA_SIZE; i++)
        host_data[i] = 0;

    cl_mem buffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, datasize_bytes, NULL, &err);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);

    size_t global_work_size[1] = {DATA_SIZE};
    size_t local_work_size[1] = {1};

    printf("Kernel futtatasa %d elemen...\n", DATA_SIZE);

    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Hiba a futtatasnal: %d\n", err);
        return -1;
    }

    clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, datasize_bytes, host_data, 0, NULL, NULL);

    printf("\n--- Eredmeny ---\n");
    for (int i = 0; i < DATA_SIZE; i++)
    {
        printf("Index [%d] -> Ertek: %d\n", i, host_data[i]);
    }
    printf("----------------\n");

    free(kernel_source);
    free(host_data);
    clReleaseMemObject(buffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}

char *load_kernel_source(const char *filename, int *error_code)
{
    FILE *f = fopen(filename, "rb");
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
    fread(source, 1, len, f);
    source[len] = '\0';
    fclose(f);
    if (error_code)
        *error_code = 0;
    return source;
}