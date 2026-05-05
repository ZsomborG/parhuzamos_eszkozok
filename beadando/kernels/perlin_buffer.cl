uint hash_func(int x, int y, int seed) {
  uint h = (uint)x * 374761393U + (uint)y * 668265263U + (uint)seed;
  h = (h ^ (h >> 13)) * 1274126177U;
  return h ^ (h >> 16);
}

float grad(uint hash, float x, float y) {
  uint h = hash & 15;
  float u = h < 8 ? x : y;
  float v = h < 4 ? y : (h == 12 || h == 14 ? x : 0.0f);
  return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// Quintic (ötödfokú) simító függvény: 6t^5 - 15t^4 + 10t^3
float fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

float lerp(float a, float b, float t) { return a + t * (b - a); }

float get_noise(float x, float y, int seed) {
  // Rácscella indexek
  int X0 = (int)floor(x);
  int Y0 = (int)floor(y);
  int X1 = X0 + 1;
  int Y1 = Y0 + 1;

  // Cellán belüli relatív koordináták
  float xf = x - floor(x);
  float yf = y - floor(y);

  // Simítási faktorok
  float u = fade(xf);
  float v = fade(yf);

  // Gradiens értékek a 4 sarokponton
  float n00 = grad(hash_func(X0, Y0, seed), xf, yf);
  float n10 = grad(hash_func(X1, Y0, seed), xf - 1.0f, yf);
  float n01 = grad(hash_func(X0, Y1, seed), xf, yf - 1.0f);
  float n11 = grad(hash_func(X1, Y1, seed), xf - 1.0f, yf - 1.0f);

  // Bilineáris interpoláció
  float nx0 = lerp(n00, n10, u);
  float nx1 = lerp(n01, n11, u);
  return lerp(nx0, nx1, v);
}

__kernel void perlin_noise(__global float *output, int width, int height,
                           float scale, int seed) {
  int i = get_global_id(0);
  int j = get_global_id(1);
  if (i >= width || j >= height)
    return;

  float x = (float)i * scale;
  float y = (float)j * scale;

  float total = 0.0f;
  float amplitude = 1.0f;
  float frequency = 1.0f;
  float max_value = 0.0f;

  for (int o = 0; o < 6; o++) {
    total += get_noise(x * frequency, y * frequency, seed) * amplitude;
    max_value += amplitude;
    amplitude *= 0.5f;
    frequency *= 2.0f;
  }

  // Normalizálás [0, 1] tartományba
  output[j * width + i] = (total / max_value + 1.0f) * 0.5f;
}