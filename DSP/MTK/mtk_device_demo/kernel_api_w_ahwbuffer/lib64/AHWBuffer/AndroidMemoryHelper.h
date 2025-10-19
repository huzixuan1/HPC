#ifndef ANDROIDMEMORYHELPER_H_
#define ANDROIDMEMORYHELPER_H_
#include <android/hardware_buffer.h>
class AndroidMemoryHelper
{
public:
    AndroidMemoryHelper();
    virtual ~AndroidMemoryHelper();
    bool mem_alloc(unsigned int length, bool cacheable, AHardwareBuffer** buffer, int *buf_share_fd, void **buf_va);
    void mem_free(AHardwareBuffer* buffer);
    bool mem_cache_sync(AHardwareBuffer* buffer);

    int getHandleId();
private:
    AHardwareBuffer_Desc desc;
};
#endif
