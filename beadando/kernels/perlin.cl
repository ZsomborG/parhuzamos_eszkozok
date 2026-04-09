// Jobb hash függvény a rácspontokhoz (bit-mixer)
uint hash_func(int x, int y, int seed) {
  uint h = (uint)x * 374761393U + (uint)y * 668265263U + (uint)seed;
  h = (h ^ (h >> 13)) * 1274126177U;
  return h ^ (h >> 16);
}

// Gradiens függvény: egységvektorokat rendel a rácspontokhoz
float grad(uint hash, float x, float y) {
  uint h = hash & 7;
  float u = h < 4 ? x : y;
  float v = h < 4 ? y : x;
  return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f * v : 2.0f * v);
}

// Quintic (ötödfokú) simító függvény: 6t^5 - 15t^4 + 10t^3
float fade(float t) { return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f); }

float lerp(float a, float b, float t) { return a + t * (b - a); }

__kernel void perlin_noise(__global float *output, int width, int height,
                           float scale, int seed) {
  int i = get_global_id(0);
  int j = get_global_id(1);

  if (i >= width || j >= height)
    return;

  // Koordináták skálázása
  float x = (float)i * scale;
  float y = (float)j * scale;

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
  float result = lerp(nx0, nx1, v);

  // Normalizálás [0, 1] tartományba
  output[j * width + i] = (result + 1.0f) * 0.5f;
}