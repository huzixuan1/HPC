__kernel void box_filter_3x3(
    __global const uchar* src_img,
    __global uchar* dst_img,
    const int width,
    const int height
) {
    int x = get_global_id(0);
    int y = get_global_id(1);

    if (x >= width || y >= height) return;

    int sum = 0;
    for (int dy = -1; dy <= 1; dy++) {
        int yy = clamp(y + dy, 0, height - 1);
        for (int dx = -1; dx <= 1; dx++) {
            int xx = clamp(x + dx, 0, width - 1);
            sum += src_img[yy * width + xx];
        }
    }

    dst_img[y * width + x] = sum / 9;
}

