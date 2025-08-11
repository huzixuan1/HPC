#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <cstring>
#include "mcve/MCVEAPI.h"

#define CHECK_ERR(err, msg) \
    if ((err) != 0) { \
        std::cerr << "[ERROR] " << msg << " (code: " << err << ")" << std::endl; \
        return -1; \
    }

int main() {
    std::cout << "start simulator vector_add" << std::endl;
    int err = 0;
    const int len = 8;
    size_t buffer_size = len * sizeof(int);

    // Init Env
    mcv_env_t* env;
    CHECK_ERR(mcvInitEnv(&env), "Init MCVE env failed");

    // Allocate buffers
    mcv_buffer_t* a = mcvMemAlloc(env, buffer_size, &err, "a", MCV_BUF_TYPE_DEFAULT);
    CHECK_ERR(err, "Alloc buffer a failed");
    mcv_buffer_t* b = mcvMemAlloc(env, buffer_size, &err, "b", MCV_BUF_TYPE_DEFAULT);
    CHECK_ERR(err, "Alloc buffer b failed");
    mcv_buffer_t* c = mcvMemAlloc(env, buffer_size, &err, "c", MCV_BUF_TYPE_DEFAULT);
    CHECK_ERR(err, "Alloc buffer c failed");

    // Init input data
    int* a_ptr = reinterpret_cast<int*>(getHostPtr(a));
    int* b_ptr = reinterpret_cast<int*>(getHostPtr(b));
    for (int i = 0; i < len; ++i) {
        a_ptr[i] = i;
        b_ptr[i] = i * 10;
    }

    // Sync to device
    mcvSyncBufferHostToDevice(env, a);
    mcvSyncBufferHostToDevice(env, b);

    // Load binary
    std::ifstream fin("vector_add.bin", std::ios::binary);
    if (!fin) {
        std::cerr << "Cannot open vector_add.bin" << std::endl;
        return -1;
    }
    std::vector<char> binary((std::istreambuf_iterator<char>(fin)), {});

    // Prepare buffers (must follow kernel param order: a, b, c)
    std::vector<mcv_buffer_t*> buffer_list = { a, b, c };

    // Prepare params
    std::vector<void*> param_list = { (void*)&len };

    // Create algo
    mcv_algo_t* algo = mcvCreateCustomAlgo(env,
                                           binary.data(),
                                           binary.size(),
                                           buffer_list.data(),
                                           buffer_list.size(),
                                           param_list.data(),
                                           param_list.size(),
                                           &err);
    CHECK_ERR(err, "Create custom algo failed");

    // Launch
    mcv_event_t evt = nullptr;
    CHECK_ERR(mcvRunAlgo(algo, 0, nullptr, &evt), "Run algo failed");

    //  wait until kernel finishes
    CHECK_ERR(mcvWaitForEvents(1, &evt), "Wait for event failed");

    //  Sync result back to host
    mcvSyncBufferDeviceToHost(env, c);
    int* out_ptr = reinterpret_cast<int*>(getHostPtr(c));

    // Compare
    bool pass = true;
    for (int i = 0; i < len; ++i) {
        int expected = a_ptr[i] + b_ptr[i];
        if (out_ptr[i] != expected) {
            std::cout << "Mismatch at " << i << ": got " << out_ptr[i]
                      << ", expected " << expected << std::endl;
            pass = false;
        }
    }

    std::cout << (pass ? "Test Passed!" : "Test Failed!") << std::endl;

    // Cleanup
    mcvReleaseEvent(&evt);
    mcvReleaseAlgo(algo);
    mcvMemFree(a);
    mcvMemFree(b);
    mcvMemFree(c);
    mcvDeinitEnv(env);

    return pass ? 0 : -1;
}
