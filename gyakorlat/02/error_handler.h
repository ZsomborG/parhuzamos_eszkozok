#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H

#include <CL/cl.h>

const char *get_error_string(cl_int error);

void check_error(cl_int error, const char *operation);

#endif