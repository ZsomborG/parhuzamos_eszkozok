float hash(int x, int y) {
  float dot_product = x * 12.9898f + y * 78.233f;
  float s = sin(dot_product) * 43758.5453f;
  return s - floor(s);
}

__kernel void perlin_noise(__global float *output, int width, int height) {
  int x = get_global_id(0);
  int y = get_global_id(1);

  if (x >= width || y >= height)
    return;

  output[y * width + x] = hash(x, y);
}