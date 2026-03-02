__kernel void write_indices(__global int *data) {
  int gid = get_global_id(0);
  data[gid] = gid;
}

__kernel void write_local_ids(__global int *data) {
  int gid = get_global_id(0);
  int lid = get_local_id(0);

  data[gid] = lid;
}

__kernel void write_reverse(__global int *data, int n) {
  int gid = get_global_id(0);

  if (gid < n) {
    data[gid] = n - 1 - gid;
  }
}

__kernel void swap_adjacent(__global int *data) {
  int gid = get_global_id(0);

  if (gid % 2 == 0) {
    data[gid] = gid + 1;
  } else {
    data[gid] = gid - 1;
  }
}

// Negyzetre emeles
__kernel void square_array(__global int *data, int scalar) {
  int gid = get_global_id(0);
  data[gid] = gid * scalar;
}