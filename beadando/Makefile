all: map benchmark

map:
	gcc main.c src/kernel_loader.c -o map.exe -Iinclude -lOpenCL -lopengl32 -lgdi32

benchmark:
	gcc perlin_gpu.c src/kernel_loader.c -o benchmark.exe -Iinclude -lOpenCL