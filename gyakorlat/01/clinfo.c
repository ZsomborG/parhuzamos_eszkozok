#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>

int main(void)
{
    cl_int err;
    cl_uint n_platforms;

    err = clGetPlatformIDs(0, NULL, &n_platforms);
    if (err != CL_SUCCESS || n_platforms == 0)
    {
        printf("[ERROR] Nem talalhato OpenCL platform. Hiba: %d\n", err);
        return -1;
    }
    printf("Eszlelt OpenCL platformok szama: %d\n\n", n_platforms);

    cl_platform_id *platforms = (cl_platform_id *)malloc(sizeof(cl_platform_id) * n_platforms);
    clGetPlatformIDs(n_platforms, platforms, NULL);

    for (cl_uint i = 0; i < n_platforms; i++)
    {
        char platform_name[1000];
        char platform_version[1000];

        clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
        clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, sizeof(platform_version), platform_version, NULL);

        printf("=============================\n");
        printf("Platform [%d]: %s\n", i, platform_name);
        printf("Platform verzio: %s\n", platform_version);
        printf("=============================\n\n");

        cl_uint n_devices;
        err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &n_devices);

        if (err != CL_SUCCESS || n_devices == 0)
        {
            printf("  Nincs elerheto eszkoz ezen a platformon (pl. driver hiba vagy ures platform).\n\n");
            continue;
        }

        printf("  Eszkozok szama a platformon: %d\n\n", n_devices);

        cl_device_id *devices = (cl_device_id *)malloc(sizeof(cl_device_id) * n_devices);
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, n_devices, devices, NULL);

        for (cl_uint j = 0; j < n_devices; j++)
        {
            char device_name[1000];
            char device_vendor[1000];
            cl_uint compute_units;
            cl_uint clock_freq;
            cl_ulong global_mem_size;
            cl_ulong local_mem_size;
            size_t max_work_group_size;

            clGetDeviceInfo(devices[j], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
            clGetDeviceInfo(devices[j], CL_DEVICE_VENDOR, sizeof(device_vendor), device_vendor, NULL);
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_freq), &clock_freq, NULL);
            clGetDeviceInfo(devices[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(global_mem_size), &global_mem_size, NULL);
            clGetDeviceInfo(devices[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(local_mem_size), &local_mem_size, NULL);
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(max_work_group_size), &max_work_group_size, NULL);

            printf("  --- Eszkoz [%d] ---\n", j);
            printf("  Nev:                    %s\n", device_name);
            printf("  Gyarto:                 %s\n", device_vendor);
            printf("  CU:                     %u\n", compute_units);
            printf("  Orajel:                 %u MHz\n", clock_freq);
            printf("  Globalis memoria:       %llu MB\n", (unsigned long long)global_mem_size / (1024 * 1024));
            printf("  Lokalis memora:         %llu KB\n", (unsigned long long)local_mem_size / 1024);
            printf("  Max Work-Group meret:   %zu\n", max_work_group_size);
            printf("  -------------------\n");
        }

        free(devices);
    }
    free(platforms);
    return 0;
}