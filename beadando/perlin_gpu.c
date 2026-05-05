#include "kernel_loader.h"
#define CL_TARGET_OPENCL_VERSION 220
#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    int test_res[][2] = {
        {960, 544},
        {1280, 720},
        {1600, 900},
        {1920, 1080},
        {2560, 1440},
        {3200, 1800},
        {3840, 2160}};

    int num_resolutions = sizeof(test_res) / sizeof(test_res[0]);
    int runs_per_res = 3;

    float scale = 0.005f;
    int seed = (int)time(NULL);
    cl_int err;
    int error_code;

    // 1. Setup
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);
    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    cl_queue_properties props[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    cl_command_queue queue = clCreateCommandQueueWithProperties(context, device, props, &err);

    // 2. Load and Build
    const char *source = load_kernel_source("kernels/perlin_buffer.cl", &error_code);
    cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    cl_kernel kernel = clCreateKernel(program, "perlin_noise", NULL);

    for (int r = 0; r < num_resolutions; r++)
    {
        int width = test_res[r][0];
        int height = test_res[r][1];
        size_t buffer_size = width * height * sizeof(float);

        printf("--- Resolution: %dx%d ---\n", width, height);

        for (int i = 0; i < runs_per_res; i++)
        {
            cl_mem device_output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, buffer_size, NULL, NULL);
            float *host_output = (float *)malloc(buffer_size);

            clSetKernelArg(kernel, 0, sizeof(cl_mem), &device_output);
            clSetKernelArg(kernel, 1, sizeof(int), &width);
            clSetKernelArg(kernel, 2, sizeof(int), &height);
            clSetKernelArg(kernel, 3, sizeof(float), &scale);
            clSetKernelArg(kernel, 4, sizeof(int), &seed);

            size_t global_size[2] = {(size_t)width, (size_t)height};
            // size_t local_size[2] = {width, height};

            cl_event kernel_event, read_event;
            clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global_size, NULL, 0, NULL, &kernel_event);
            clEnqueueReadBuffer(queue, device_output, CL_TRUE, 0, buffer_size, host_output, 0, NULL, &read_event);

            cl_ulong start, end, r_start, r_end;
            clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
            clGetEventProfilingInfo(kernel_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
            clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &r_start, NULL);
            clGetEventProfilingInfo(read_event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &r_end, NULL);

            printf("Kernel: %.4f ms | Readback: %.4f ms\n", (end - start) * 1e-6, (r_end - r_start) * 1e-6);

            if (i == runs_per_res - 1)
            {
                char filename[64];
                sprintf(filename, "perlin_%dx%d.ppm", width, height);
                FILE *fp = fopen(filename, "wb");
                fprintf(fp, "P6\n%d %d\n255\n", width, height);
                for (int j = 0; j < width * height; j++)
                {
                    unsigned char val = (unsigned char)(host_output[j] * 255.0f);
                    fputc(val, fp);
                    fputc(val, fp);
                    fputc(val, fp);
                }
                fclose(fp);
                printf("Saved output to %s\n", filename);
            }
            printf("\n");

            clReleaseEvent(kernel_event);
            clReleaseEvent(read_event);
            clReleaseMemObject(device_output);
            free(host_output);
        }
    }

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    return 0;
}