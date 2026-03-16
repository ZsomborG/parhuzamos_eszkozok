__kernel void div_by_zero_kernel(__global int *data, int divisor) {
  int gid = get_global_id(0);
  data[gid] = 100 / divisor;
}