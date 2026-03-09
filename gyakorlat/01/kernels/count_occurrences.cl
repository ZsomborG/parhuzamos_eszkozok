__kernel void count_occurrences(__global const int *data, __global int *counts,
                                int n) {
  int gid = get_global_id(0);

  if (gid < n) {
    int my_value = data[gid];
    int count = 0;

    for (int i = 0; i < n; i++) {
      if (data[i] == my_value) {
        count++;
      }
    }

    counts[gid] = count;
  }
}