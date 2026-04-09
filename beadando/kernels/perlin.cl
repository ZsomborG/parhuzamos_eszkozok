__kernel void perlin_noise(__global float *output, int width) {
  int x = get_global_id(0);
  int y = get_global_id(1);

  output[y * width + x] = (float)x + (y * 10.0f);
}