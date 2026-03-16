int factorial(int n) {
  if (n <= 1)
    return 1;
  return n * factorial(n - 1);
}

__kernel void recursion_kernel(__global int *data) {
  data[get_global_id(0)] = factorial(5);
}