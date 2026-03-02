__kernel void vector_add(__global const float *A, __global const float *B,
                         __global float *C, int n) {
  int gid = get_global_id(0);

  if (gid < n) {
    C[gid] = A[gid] + B[gid];
  }
}