__kernel void too_large_kernel(__global int *data) {
  __local int huge_array[100000000];

  huge_array[get_local_id(0)] = 1;
  data[get_global_id(0)] = huge_array[get_local_id(0)];
}