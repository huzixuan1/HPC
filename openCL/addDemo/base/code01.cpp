#include <stdio.h>
#include <stdlib.h>
#include <OpenCL/opencl.h>

#define N 1024

// 读取 kernel 源码
char* read_source(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Cannot open kernel");
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    char* source = (char*)malloc(size + 1);
    fread(source, 1, size, fp);
    source[size] = '\0';
    fclose(fp);
    return source;
}

int main() {
    float* A = (float*)malloc(sizeof(float) * N);
    float* B = (float*)malloc(sizeof(float) * N);
    float* C = (float*)malloc(sizeof(float) * N);

    for (int i = 0; i < N; i++) {
        A[i] = i * 1.0f;
        B[i] = i * 2.0f;
    }

    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;

    clGetPlatformIDs(1, &platform, NULL);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    context = clCreateContext(NULL, 1, &device, NULL, NULL, NULL);
    queue = clCreateCommandQueueWithPropertiesAPPLE(context, device, 0, NULL);

    // 分配缓冲区
    cl_mem bufferA = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    sizeof(float) * N, A, NULL);
    cl_mem bufferB = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                    sizeof(float) * N, B, NULL);
    cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY,
                                    sizeof(float) * N, NULL, NULL);

    // 读取并编译内核
    char* source = read_source("add.cl");
    program = clCreateProgramWithSource(context, 1, (const char**)&source, NULL, NULL);
    cl_int build_status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);

    if (build_status != CL_SUCCESS) {
        char log[2048];
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(log), log, NULL);
        printf("Build failed:\n%s\n", log);
        return 1;
    }

    kernel = clCreateKernel(program, "vector_add", NULL);

    // 设置参数
    clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufferA);
    clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufferB);
    clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufferC);

    // 执行内核
    size_t global_work_size = N;
    clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_work_size, NULL, 0, NULL, NULL);
    clFinish(queue);

    // 读取结果
    clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, sizeof(float) * N, C, 0, NULL, NULL);

    // 打印前10项
    for (int i = 0; i < 10; i++) {
        printf("Result[%d]: %.1f + %.1f = %.1f\n", i, A[i], B[i], C[i]);
    }

    // 清理资源
    clReleaseMemObject(bufferA);
    clReleaseMemObject(bufferB);
    clReleaseMemObject(bufferC);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
    free(A); free(B); free(C); free(source);

    return 0;
}

