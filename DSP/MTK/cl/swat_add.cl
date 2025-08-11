__kernel void vector_add(__global const int* a,
                         __global const int* b,
                         __global int* c,
                         int len) {
    int id = get_global_id(0);
    if (id < len) {
        c[id] = a[id] + b[id];
    }
}
