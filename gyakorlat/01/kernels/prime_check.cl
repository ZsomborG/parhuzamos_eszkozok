// 1. Feladatrész: Minden szál csak EGYETLEN osztót vizsgál
__kernel void check_single(unsigned int n, __global int *is_prime) {
  unsigned int divisor = get_global_id(0) + 2;

  if (divisor * divisor <= n) {
    if (n % divisor == 0) {
      *is_prime = 0;
    }
  }
}

// 2. Feladatrész: Minden szál egy TARTOMÁNYT (több osztót) vizsgál
__kernel void check_range(unsigned int n, __global int *is_prime,
                          unsigned int range_size) {
  unsigned int gid = get_global_id(0);

  unsigned int start = 2 + (gid * range_size);
  unsigned int end = start + range_size;

  for (unsigned int d = start; d < end; d++) {
    if (d * d > n)
      break;

    if (n % d == 0) {
      *is_prime = 0;
      break;
    }
  }
}

// 3. Feladatrész: Előre kiosztott PRÍMEK vizsgálata
__kernel void check_precalculated(unsigned int n, __global int *is_prime,
                                  __global const unsigned int *primes,
                                  unsigned int num_primes) {
  unsigned int gid = get_global_id(0);

  if (gid < num_primes) {
    unsigned int p = primes[gid];

    if (p * p <= n) {
      if (n % p == 0) {
        *is_prime = 0;
      }
    }
  }
}