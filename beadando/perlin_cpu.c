#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef unsigned int uint;

uint hash_func(int x, int y, int seed)
{
    uint h = (uint)x * 374761393U + (uint)y * 668265263U + (uint)seed;
    h = (h ^ (h >> 13)) * 1274126177U;
    return h ^ (h >> 16);
}

float grad(uint hash, float x, float y)
{
    uint h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : 0.0f);
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

float fade(float t)
{
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

float get_noise(float x, float y, int seed)
{
    int X0 = (int)floor(x);
    int Y0 = (int)floor(y);
    int X1 = X0 + 1;
    int Y1 = Y0 + 1;
    float xf = x - floor(x);
    float yf = y - floor(y);
    float u = fade(xf);
    float v = fade(yf);
    float n00 = grad(hash_func(X0, Y0, seed), xf, yf);
    float n10 = grad(hash_func(X1, Y0, seed), xf - 1.0f, yf);
    float n01 = grad(hash_func(X0, Y1, seed), xf, yf - 1.0f);
    float n11 = grad(hash_func(X1, Y1, seed), xf - 1.0f, yf - 1.0f);
    float nx0 = lerp(n00, n10, u);
    float nx1 = lerp(n01, n11, u);
    return lerp(nx0, nx1, v);
}

#include <windows.h>
double get_time_sec()
{
    LARGE_INTEGER freq, counter;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / freq.QuadPart;
}

int main(void)
{
    const int width = 3840, height = 2160;
    float scale = 0.005f;
    int seed = (int)time(NULL);
    int total_pixels = width * height;
    float *output = malloc(total_pixels * sizeof(float));
    if (!output)
    {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    double t_start = get_time_sec();

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            float x = i * scale;
            float y = j * scale;
            float total = 0.0f, amplitude = 1.0f, frequency = 1.0f, max_value = 0.0f;
            for (int o = 0; o < 6; o++)
            {
                total += get_noise(x * frequency, y * frequency, seed) * amplitude;
                max_value += amplitude;
                amplitude *= 0.5f;
                frequency *= 2.0f;
            }
            output[j * width + i] = (total / max_value + 1.0f) * 0.5f;
        }
    }

    double t_end = get_time_sec();
    printf("CPU plain C time: %.3f ms\n", (t_end - t_start) * 1000.0);

    FILE *fp = fopen("perlin_cpu.ppm", "wb");
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    for (int i = 0; i < total_pixels; i++)
    {
        unsigned char val = (unsigned char)(output[i] * 255.0f);
        fputc(val, fp);
        fputc(val, fp);
        fputc(val, fp);
    }
    fclose(fp);

    free(output);
    return 0;
}