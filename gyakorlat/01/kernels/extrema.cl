__kernel void init_candidates(__global int *B, int n) {
  int i = get_global_id(0);
  if (i < n)
    B[i] = 1;
}

__kernel void compare_triangle(__global const int *A, __global int *B, int n) {
  int i = get_global_id(0); // X koordináta
  int j = get_global_id(1); // Y koordináta

  if (i < j && i < n && j < n) {
    if (A[i] < A[j]) {
      B[i] = 0;
    } else if (A[i] > A[j]) {
      B[j] = 0;
    } else {
      B[j] = 0;
    }
  }
}

__kernel void get_result(__global const int *A, __global const int *B,
                         __global int *result, int n) {
  int i = get_global_id(0);
  if (i < n) {
    if (B[i] == 1) {
      *result = A[i];
    }
  }
}