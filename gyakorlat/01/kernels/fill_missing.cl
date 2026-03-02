__kernel void fill_missing(__global int *data, int n) {
  int gid = get_global_id(0);
  if (gid > 0 && gid < n - 1) {
    if (data[gid] == -1) {
      int prev = data[gid - 1];
      int next = data[gid + 1];

      data[gid] = (prev + next) / 2;
    }
  }
}