__kernel void hello_kernel(__global int* data) {
    int gid = get_global_id(0);
    data[gid] = gid * 10;
}