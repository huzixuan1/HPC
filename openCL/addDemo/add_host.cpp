#include <stdio.h>
#include <stdlib.h>
#include <OpenCL/opencl.h>

#define ARRAY_SIZE 1024

// 读取 OpenCL 内核源码
char *read_source(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
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

// 简单的错误检查宏
#define CHECK_ERROR(err, msg) \
    if (err != CL_SUCCESS) { \
        fprintf(stderr, "%s failed with error %d\n", msg, err); \
        exit(1); \
    }

int main()
{
    cl_int err;

    // 初始化输入输出数组
    float *A = (float *)malloc(sizeof(float) * ARRAY_SIZE);
    float *B = (float *)malloc(sizeof(float) * ARRAY_SIZE);
    float *C = (float *)malloc(sizeof(float) * ARRAY_SIZE);

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        A[i] = (float)i;
        B[i] = (float)(i * 2);
        C[i] = 0.0f;
    }

    // 1. 获取平台
    cl_platform_id platform;
    err = clGetPlatformIDs(1, &platform, NULL);
    CHECK_ERROR(err, "clGetPlatformIDs");

    // 2. 获取设备
    cl_device_id device;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, NULL);
    CHECK_ERROR(err, "clGetDeviceIDs");

    // 打印设备名称
    char device_name[128];
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
    CHECK_ERROR(err, "clGetDeviceInfo");
    printf("Using OpenCL device: %s\n", device_name);

    // 3. 创建上下文
    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    CHECK_ERROR(err, "clCreateContext");

    // 4. 创建命令队列
    cl_command_queue_properties props[] = {0}; // 空属性
    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &err);
    CHECK_ERROR(err, "clCreateCommandQueueWithPropertiesAPPLE");

    // 5. 创建缓冲区
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    sizeof(float) * ARRAY_SIZE, A, &err);
    CHECK_ERROR(err, "clCreateBuffer A");

    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    sizeof(float) * ARRAY_SIZE, B, &err);
    CHECK_ERROR(err, "clCreateBuffer B");

    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                    sizeof(float) * ARRAY_SIZE, NULL, &err);
    CHECK_ERROR(err, "clCreateBuffer C");

    // 6. 读取内核源码
    char *source = read_source("./add.cl");
    printf("Kernel Source Loaded:\n%s\n", source);

    // 7. 创建程序对象
    cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source, NULL, &err);
    CHECK_ERROR(err, "clCreateProgramWithSource");

    // 8. 编译内核程序
    err = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    if (err != CL_SUCCESS) {
        char log[4096];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
        fprintf(stderr, "clBuildProgram failed:\n%s\n", log);
        exit(1);
    }
    printf("Program built successfully.\n");

    // 9. 创建内核对象
    cl_kernel kernel = clCreateKernel(program, "vector_add", &err);
    CHECK_ERROR(err, "clCreateKernel");

    // 10. 设置内核参数
    err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    CHECK_ERROR(err, "clSetKernelArg 0");
    err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    CHECK_ERROR(err, "clSetKernelArg 1");
    err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);
    CHECK_ERROR(err, "clSetKernelArg 2");

    // 11. 执行内核
    size_t global_size = ARRAY_SIZE;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, NULL, 0, NULL, NULL);
    CHECK_ERROR(err, "clEnqueueNDRangeKernel");

    // 确保执行完成
    err = clFinish(queue);
    CHECK_ERROR(err, "clFinish");

    // 12. 读取结果
    err = clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, sizeof(float) * ARRAY_SIZE, C, 0, NULL, NULL);
    CHECK_ERROR(err, "clEnqueueReadBuffer");

    // 13. 打印结果检查
    for (int i = 0; i < 10; i++)
    {
        printf("Result[%d]: %.1f + %.1f = %.1f\n", i, A[i], B[i], C[i]);
    }

    // 14. 释放资源
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(source);
    free(A);
    free(B);
    free(C);

    return 0;
}

