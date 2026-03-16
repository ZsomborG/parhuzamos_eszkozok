#ifndef KERNEL_LOADER_H
#define KERNEL_LOADER_H

#include <CL/cl.h>

char *load_kernel_source(const char *filename, int *error_code);
cl_program build_program(cl_context context, cl_device_id device, const char *source_code);

#endif