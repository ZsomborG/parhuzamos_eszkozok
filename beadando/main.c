#define CL_TARGET_OPENCL_VERSION 220
#ifndef GL_RGBA32F
#define GL_RGBA32F 0x8814
#endif

#include <windows.h>
#include <GL/gl.h>
#include <CL/cl.h>
#include <CL/cl_gl.h>
#include "kernel_loader.h"
#include <stdio.h>

const int width = 1024;
const int height = 1024;

cl_context context;
cl_command_queue queue;
cl_kernel kernel;
cl_mem cl_image;

float cam_x = 0.0f;
float cam_y = 0.0f;
float cam_speed = 0.0005f;

void init_opencl(HWND hwnd)
{
    cl_int err;
    cl_platform_id platform;
    clGetPlatformIDs(1, &platform, NULL);

    cl_device_id device;
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    cl_context_properties props[] = {
        CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
        CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
        CL_CONTEXT_PLATFORM, (cl_context_properties)platform,
        0};
    context = clCreateContext(props, 1, &device, NULL, NULL, &err);

    cl_queue_properties q_props[] = {CL_QUEUE_PROPERTIES, CL_QUEUE_PROFILING_ENABLE, 0};
    queue = clCreateCommandQueueWithProperties(context, device, q_props, &err);

    int error_code;
    const char *source = load_kernel_source("kernels/perlin.cl", &error_code);
    cl_program program = clCreateProgramWithSource(context, 1, &source, NULL, NULL);
    clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    kernel = clCreateKernel(program, "perlin_noise", NULL);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    cl_image = clCreateFromGLTexture(context, CL_MEM_WRITE_ONLY, GL_TEXTURE_2D, 0, tex, &err);
}

void render(HWND hwnd)
{
    if (GetAsyncKeyState('W') & 0x8000)
        cam_y += cam_speed;
    if (GetAsyncKeyState('S') & 0x8000)
        cam_y -= cam_speed;
    if (GetAsyncKeyState('A') & 0x8000)
        cam_x -= cam_speed;
    if (GetAsyncKeyState('D') & 0x8000)
        cam_x += cam_speed;

    float scale = 0.004f;
    int seed = 123;

    glFinish();
    clEnqueueAcquireGLObjects(queue, 1, &cl_image, 0, NULL, NULL);

    clSetKernelArg(kernel, 0, sizeof(cl_mem), &cl_image);
    clSetKernelArg(kernel, 1, sizeof(float), &scale);
    clSetKernelArg(kernel, 2, sizeof(float), &cam_x);
    clSetKernelArg(kernel, 3, sizeof(float), &cam_y);
    clSetKernelArg(kernel, 4, sizeof(int), &seed);

    size_t global[2] = {width, height};
    cl_event event;

    clEnqueueNDRangeKernel(queue, kernel, 2, NULL, global, NULL, 0, NULL, &event);
    clEnqueueReleaseGLObjects(queue, 1, &cl_image, 0, NULL, NULL);

    clWaitForEvents(1, &event);

    static DWORD last_time = 0;
    static int frames = 0;
    DWORD current_time = GetTickCount();
    frames++;

    if (current_time - last_time >= 500)
    {
        cl_ulong start_time, end_time;
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(start_time), &start_time, NULL);
        clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(end_time), &end_time, NULL);
        double gpu_time_ms = (double)(end_time - start_time) / 1000000.0;
        float fps = frames * 1000.0f / (current_time - last_time);

        char title[256];
        sprintf(title, "OpenCL Terepgenerator | FPS: %.0f | GPU Ido: %.2f ms", fps, gpu_time_ms);
        SetWindowTextA(hwnd, title);

        frames = 0;
        last_time = current_time;
    }
    clReleaseEvent(event);
    clFinish(queue);

    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(-1, -1);
    glTexCoord2f(1, 0);
    glVertex2f(1, -1);
    glTexCoord2f(1, 1);
    glVertex2f(1, 1);
    glTexCoord2f(0, 1);
    glVertex2f(-1, 1);
    glEnd();
}

LRESULT CALLBACK WndProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    if (m == WM_DESTROY)
        PostQuitMessage(0);
    return DefWindowProc(h, m, w, l);
}

int main()
{
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.lpszClassName = "CLGL";
    RegisterClass(&wc);

    HWND hwnd = CreateWindow("CLGL", "OpenCL Terepgenerator", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 100, 100, width, height, 0, 0, 0, 0);

    HDC hdc = GetDC(hwnd);
    PIXELFORMATDESCRIPTOR pfd = {sizeof(pfd), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32};
    SetPixelFormat(hdc, ChoosePixelFormat(hdc, &pfd), &pfd);
    wglMakeCurrent(hdc, wglCreateContext(hdc));

    init_opencl(hwnd);

    MSG msg;
    while (1)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            DispatchMessage(&msg);
        }
        render(hwnd);
        SwapBuffers(hdc);
    }

    return 0;
}