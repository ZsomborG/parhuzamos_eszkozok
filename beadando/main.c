#include "kernel_loader.h"
#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    const int width = 512;
    const int height = 512;
    size_t buffer_size = width * height * sizeof(float);

    cl_int err;
    int error_code;

    // 1. Setup
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, NULL, &err);

    // 2. Load and Build
    const char *source = load_kernel_source("kernels/perlin.cl", &error_code);
    cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "perlin_noise", NULL);

    // 3. Buffers
    cl_mem device_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffer_size, NULL, NULL);
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &device_output);
    clSetKernelArg(kernel, 1, sizeof(int), &width);
    clSetKernelArg(kernel, 2, sizeof(int), &height);

    // 4. Run
    size_t global_size[2] = {width, height};
    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, NULL, 0, NULL, NULL);

    // 5. Read Back
    float *host_output = (float *)malloc(buffer_size);
    clEnqueueReadBuffer(queue, device_output, CL_TRUE, 0, buffer_size, host_output, 0, NULL, NULL);

    // 6. Save to .ppm
    FILE *fp = fopen("perlin_noise.ppm", "wb");
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    for (int i = 0; i < width * height; i++)
    {
        unsigned char val = (unsigned char)(host_output[i] * 255.0f);
        fputc(val, fp); // R
        fputc(val, fp); // G
        fputc(val, fp); // B
    }
    fclose(fp);

    // Cleanup
    clReleaseMemObject(device_output);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(host_output);
    return 0;
}