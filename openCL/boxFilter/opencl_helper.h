//
// Created by zixhu on 2025/7/26.
//

#ifndef COMPUTERVISION_OPENCL_HELPER_H
#define COMPUTERVISION_OPENCL_HELPER_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <OpenCL/opencl.h>

#define CHECK_ERROR(err, msg) \
    if (err != CL_SUCCESS) { \
        fprintf(stderr, "%s failed with error %d\n", msg, err); \
        exit(1); \
    }

typedef struct {
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
} OpenCLObjects;

// 加载内核源码
char *read_source(const char *filename) {
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open kernel file");
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    rewind(fp);
    char *source = (char *)malloc(size + 1);
    fread(source, 1, size, fp);
    source[size] = '\0';
    fclose(fp);
    return source;
}

// 初始化 OpenCL，并构建 kernel
OpenCLObjects init_opencl(const char *source_file, const char *kernel_name) {
    OpenCLObjects ocl;
    cl_int err;

    err = clGetPlatformIDs(1, &ocl.platform, NULL);
    CHECK_ERROR(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(ocl.platform, CL_DEVICE_TYPE_DEFAULT, 1, &ocl.device, NULL);
    CHECK_ERROR(err, "clGetDeviceIDs");

    ocl.context = clCreateContext(NULL, 1, &ocl.device, NULL, NULL, &err);
    CHECK_ERROR(err, "clCreateContext");

    ocl.queue = clCreateCommandQueue(ocl.context, ocl.device, 0, &err);
    CHECK_ERROR(err, "clCreateCommandQueue");

    char *source = read_source(source_file);
    ocl.program = clCreateProgramWithSource(ocl.context, 1, (const char **)&source, NULL, &err);
    CHECK_ERROR(err, "clCreateProgramWithSource");

    err = clBuildProgram(ocl.program, 1, &ocl.device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        char log[4096];
        clGetProgramBuildInfo(ocl.program, ocl.device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
        fprintf(stderr, "Build Error:\n%s\n", log);
        exit(1);
    }

    ocl.kernel = clCreateKernel(ocl.program, kernel_name, &err);
    CHECK_ERROR(err, "clCreateKernel");

    free(source);
    return ocl;
}

void release_opencl(OpenCLObjects *ocl) {
    clReleaseKernel(ocl->kernel);
    clReleaseProgram(ocl->program);
    clReleaseCommandQueue(ocl->queue);
    clReleaseContext(ocl->context);
}

#endif //COMPUTERVISION_OPENCL_HELPER_H
