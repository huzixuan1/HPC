#include "AndroidMemoryHelper.h"
#include "hardware_buffer.h"
#include <sys/mman.h>
#include <iostream>
#include <android/log.h>
#include <dlfcn.h>

using namespace std;

#define INVALID_ID -1

// 来自Linux内核的dma-buf同步结构和宏定义
struct dma_buf_sync {
  __u64 flags;
};
#define DMA_BUF_BASE 'b'
#define DMA_BUF_SYNC_END (1 << 2)
#define DMA_BUF_SYNC_START (0 << 2)
#define DMA_BUF_SYNC_READ (1 << 0)
#define DMA_BUF_SYNC_WRITE (2 << 0)
#define DMA_BUF_SYNC_RW (DMA_BUF_SYNC_READ | DMA_BUF_SYNC_WRITE)
#define DMA_BUF_IOCTL_SYNC _IOW(DMA_BUF_BASE, 0, struct dma_buf_sync)

#define LOGTAG "neuron-test-demo"
#define LOGE(...)  // __android_log_print(ANDROID_LOG_ERROR, LOGTAG, __VA_ARGS__);
#define LOGW(...)  // __android_log_print(ANDROID_LOG_WARN, LOGTAG, __VA_ARGS__);
#define LOGI(...)  // __android_log_print(ANDROID_LOG_INFO, LOGTAG, __VA_ARGS__);
#define LOGD(...)  // __android_log_print(ANDROID_LOG_DEBUG, LOGTAG, __VA_ARGS__);
#define LOGV(...)  // __android_log_print(ANDROID_LOG_VERBOSE, LOGTAG, __VA_ARGS__);

typedef int (*gralloc_extra_query_func)(buffer_handle_t, GRALLOC_EXTRA_ATTRIBUTE_QUERY, void*);
static void* dlhandler = nullptr;

// 获取AHardwareBuffer对应的ion fd
AndroidMemoryHelper::AndroidMemoryHelper() {
    dlhandler = dlopen("libgralloc_extra_sys.so",RTLD_LAZY | RTLD_LOCAL);
    if (dlhandler == nullptr) {
        dlhandler = dlopen("libgralloc_extra.so",RTLD_LAZY | RTLD_LOCAL);
        if (dlhandler == nullptr) {
            char * error = nullptr;
            if ((error = dlerror()) != nullptr) {
                LOGE("getFdFromAHardwareBufferInternal libgralloc_extra dlopen error: %s\n", error);
            }
        }
    }
}

// 析构函数，关闭动态库句柄
AndroidMemoryHelper::~AndroidMemoryHelper() {
    if (dlhandler != nullptr) {
        dlclose(dlhandler);
        dlhandler = nullptr;
    }
}

// 从AHardwareBuffer获取一个ion fd
int getFdFromAHardwareBuffer(const AHardwareBuffer* hardwareBuffer){
    int32_t ionFD = INVALID_ID;
    const native_handle_t *handle = AHardwareBuffer_getNativeHandle(hardwareBuffer);

    if (nullptr == dlhandler || nullptr == handle) {
       return ionFD;
    }
    gralloc_extra_query_func grallocExtraQuery = (gralloc_extra_query_func)dlsym(dlhandler, "gralloc_extra_query");
    if (grallocExtraQuery == nullptr) {
        char * error = nullptr;
        if ((error = dlerror()) != nullptr) {
            LOGE("getFdFromAHardwareBufferInternal libgralloc_extra dlopen error: %s\n", error);
        }
        dlclose(dlhandler);
        return INVALID_ID;
    }
    int err = grallocExtraQuery(handle, GRALLOC_EXTRA_GET_ION_FD, &ionFD);
    if (GRALLOC_EXTRA_OK != err){
        LOGE("getFdFromAHardwareBufferInternal: %d", -err);
    }
    return ionFD;
}

//  分配AHardwareBuffer内存
bool AndroidMemoryHelper::mem_alloc(unsigned int length, bool cacheable, AHardwareBuffer** buffer, int *buf_share_fd, void **buf_va){
    uint64_t* id;
     uint64_t usage = 0;
    if (cacheable) {
        usage = AHARDWAREBUFFER_USAGE_CPU_READ_OFTEN | AHARDWAREBUFFER_USAGE_CPU_WRITE_OFTEN;
    } else {
        usage=AHARDWAREBUFFER_USAGE_CPU_READ_RARELY | AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY;
    }
    memset(&desc, 0, sizeof(AHardwareBuffer_Desc));
    desc.width = length;
    desc.format = AHARDWAREBUFFER_FORMAT_BLOB;
    desc.height = 1;
    desc.layers = 1;
    desc.usage = usage;
    desc.stride = length;
    AHardwareBuffer_allocate(&desc, buffer);
    if (buffer == nullptr) {
        LOGE("Can't acquire  AHardwareBuffer");
        return false;
    }

    *buf_share_fd = getFdFromAHardwareBuffer(*buffer);
    if (INVALID_ID == *buf_share_fd ) {
        AHardwareBuffer_release(*buffer);
        return false;
    }

    int status = AHardwareBuffer_lock(*buffer, desc.usage, INVALID_ID, nullptr, buf_va);
    if (status != 0) {
        LOGE("Can't lock the AHardwareBuffer %d", status);
        AHardwareBuffer_release(*buffer);
        return false;
    }
    int32_t fence = -1;
    AHardwareBuffer_unlock(*buffer, &fence);
    return true;
}

//  获取同步AHardwareBuffer缓存
bool AndroidMemoryHelper::mem_cache_sync(AHardwareBuffer* buffer){
    // bool status = false;
    // if (buffer == nullptr) {
    //     LOGE("AHardwareBuffer is nullptr");
    //     return status;
    // }
    // status = MtkGraphWrapper_syncAHardwareBuffer(buffer);
    // if (!status){
    //     LOGE("sync AHardwareBuffer error");
    //     return status;
    // }
   return false;
}

// 释放AHardwareBuffer内存
void AndroidMemoryHelper::mem_free(AHardwareBuffer* buffer) {
    AHardwareBuffer_release(buffer);
    return;
}