#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include <iostream>
#include <vector>
#include "Foundation/Foundation.hpp"
#include "QuartzCore/QuartzCore.hpp"
#include "Metal/Metal.hpp"

int main() {

    const int count = 16;

    // 1. 创建设备
    MTL::Device* device = MTL::CreateSystemDefaultDevice();
    if (!device) {
        std::cerr << "No Metal device found.\n";
        return -1;
    }

    // 2. 加载库
    MTL::Library* library = device->newDefaultLibrary();
    
    if (!library) {
        std::cerr << "Failed to load default library!\n";
        std::cerr << "Make sure kernel.metal is added to your Xcode project.\n";
        return -1;
    }

    // 3. 获取 kernel 函数
    NS::String* functionName = NS::String::string("vector_add", NS::UTF8StringEncoding);
    MTL::Function* function = library->newFunction(functionName);
    
    if (!function) {
        std::cerr << "Failed to find function 'vector_add'.\n";
        
        // 调试
        std::cout << "Available functions in library:\n";
        NS::Array* functionNames = library->functionNames();
        for (size_t i = 0; i < functionNames->count(); ++i) {
            NS::String* name = reinterpret_cast<NS::String*>(functionNames->object(i));
            std::cout << "  - " << name->utf8String() << std::endl;
        }
        
        return -1;
    }

    // 4. 创建 pipeline
    NS::Error* error = nullptr;
    MTL::ComputePipelineState* pipeline = device->newComputePipelineState(function, &error);
    
    if (!pipeline) {
        std::cerr << "Failed to create pipeline: ";
        if (error) {
            std::cerr << error->localizedDescription()->utf8String();
        }
        std::cerr << std::endl;
        return -1;
    }

    // 5. 准备数据
    std::vector<float> a(count), b(count);
    for (int i = 0; i < count; i++) {
        a[i] = static_cast<float>(i);
        b[i] = static_cast<float>(i * 2);
    }

    // 6. 创建 buffers
    MTL::Buffer* bufA = device->newBuffer(a.data(),
                                          count * sizeof(float),
                                          MTL::ResourceStorageModeShared);
    MTL::Buffer* bufB = device->newBuffer(b.data(),
                                          count * sizeof(float),
                                          MTL::ResourceStorageModeShared);
    MTL::Buffer* bufOut = device->newBuffer(count * sizeof(float),
                                            MTL::ResourceStorageModeShared);

    // 7. 创建命令队列和缓冲区
    MTL::CommandQueue* queue = device->newCommandQueue();
    MTL::CommandBuffer* cmd = queue->commandBuffer();
    MTL::ComputeCommandEncoder* encoder = cmd->computeCommandEncoder();

    // 8. 设置计算参数
    encoder->setComputePipelineState(pipeline);
    encoder->setBuffer(bufA, 0, 0);
    encoder->setBuffer(bufB, 0, 1);
    encoder->setBuffer(bufOut, 0, 2);

    // 9. 计算线程组大小
    NS::UInteger maxThreads = pipeline->maxTotalThreadsPerThreadgroup();
    NS::UInteger threadsPerGroup = std::min(maxThreads, static_cast<NS::UInteger>(count));
    
    // 创建 MTL::Size 对象
    MTL::Size gridSize = MTL::Size::Make(count, 1, 1);
    MTL::Size groupSize = MTL::Size::Make(threadsPerGroup, 1, 1);

    // 10. 调度线程
    encoder->dispatchThreads(gridSize, groupSize);
    encoder->endEncoding();

    // 11. 提交并等待完成
    cmd->commit();
    cmd->waitUntilCompleted();

    // 12. 获取并打印结果
    float* result = reinterpret_cast<float*>(bufOut->contents());
    for (int i = 0; i < count; i++) {
        std::cout << a[i] << " + " << b[i] << " = " << result[i] << std::endl;
    }

    // 13. 清理资源
    encoder->release();
    cmd->release();
    queue->release();
    bufA->release();
    bufB->release();
    bufOut->release();
    pipeline->release();
    function->release();
    library->release();
    device->release();

    return 0;
}

