//
// Created by zixhu on 2025/9/1.
//

#ifndef COMPUTERVISION_BOXMAIN_H
#define COMPUTERVISION_BOXMAIN_H
#include <vector>
#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "../helper/opencl_helper.h"

int boxMain() {
    // 读取灰度图
    cv::Mat image = cv::imread("../src/opencl/sources/img.png", cv::IMREAD_GRAYSCALE);
    if (image.empty()) {
        printf("Image load failed!\n");
        return -1;
    }

    int width = image.cols;
    int height = image.rows;
    size_t imgSize = width * height;

    // 初始化 OpenCL，加载 kernel
    OpenCLObjects ocl = init_opencl("box_filter.cl", "box_filter_3x3");

    cl_int err;
    // 创建输入输出 buffer
    cl_mem buf_input = clCreateBuffer(ocl.context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                      imgSize, image.data, &err);
    CHECK_ERROR(err, "clCreateBuffer input");

    cl_mem buf_output = clCreateBuffer(ocl.context, CL_MEM_WRITE_ONLY, imgSize, NULL, &err);
    CHECK_ERROR(err, "clCreateBuffer output");

    // 设置 kernel 参数
    clSetKernelArg(ocl.kernel, 0, sizeof(cl_mem), &buf_input);
    clSetKernelArg(ocl.kernel, 1, sizeof(cl_mem), &buf_output);
    clSetKernelArg(ocl.kernel, 2, sizeof(int), &width);
    clSetKernelArg(ocl.kernel, 3, sizeof(int), &height);

    // 执行 kernel
    size_t gsize[2] = { (size_t)width, (size_t)height };
    err = clEnqueueNDRangeKernel(ocl.queue, ocl.kernel, 2, NULL, gsize, NULL, 0, NULL, NULL);
    CHECK_ERROR(err, "clEnqueueNDRangeKernel");
    clFinish(ocl.queue);

    // 读回结果
    std::vector<uchar> result(imgSize);
    clEnqueueReadBuffer(ocl.queue, buf_output, CL_TRUE, 0, imgSize, result.data(), 0, NULL, NULL);

    // 显示结果
    cv::Mat output(height, width, CV_8UC1, result.data());
    cv::imshow("Original", image);
    cv::imshow("Box Filter 3x3", output);
    cv::waitKey(0);

    // 释放资源
    release_opencl(&ocl);
    clReleaseMemObject(buf_input);
    clReleaseMemObject(buf_output);

    return 0;
}


#endif //COMPUTERVISION_BOXMAIN_H

