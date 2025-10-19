
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2024. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL
 * PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR
 * ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A
 * PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software")
 * have been modified by MediaTek Inc. All revisions are subject to any
 * receiver's
 * applicable license agreements with MediaTek Inc.
 */
#include <sys/stat.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "AHWBuffer/AndroidMemoryHelper.h"
#include "AHWBuffer/hardware_buffer.h"
// use mcv_shim for dlopen
#include "mcve/mcve_shim.h"

#define SEED_SIZE 256       // random seed array should be sizeof(uint32_t) * 64
#define MATRIX_SIZE 36  // homography matrix sizeof(float) * 9

#define CHECK_ERR(errCode, msg)       \
    do {                              \
        if (errCode != MCV_SUCCESS) { \
            printf("%s \n", msg);     \
            return -1;                \
        }                             \
    } while (0)

#define BUF_TYPE    MCV_BUF_TYPE_DMA

bool ReadData(uint8_t *buffer, uint32_t buffer_size, const char *data_file_path);
int LoadFile(std::vector<char> *data, const char *filename);

int main(int argc, char **argv) {
    // assign data path for example bit true comparing
    // 读取数据路径 用于调用示例比对

    // prevImg.bin, nextImg.bin → 光流输入图像
    const char *prev_img_path = "cve_data/prevImg.bin";
    const char *next_img_path = "cve_data/nextImg.bin";
    
    // prevPts.bin → 光流初始点
    const char *pts_path = "cve_data/prevPts.bin";

    // H.bin → 计算得到的单应矩阵输出
    const char *h_matrix_path = "cve_data/H.bin";
    // warp_output_bl_const.bin → 计算得到的图像变换输出
    const char *warp_golden_path = "cve_data/warp_output_bl_const.bin";

    // Load Custom Binary
    const char *binary_path = "CustomKernel.bin";
    std::vector<char> binary;
    if (LoadFile(&binary, binary_path) != 0) {
        std::cout << "Can't find Binary File. "
                  << "Use gen binary tool to generate binary or give correct Binary Path."
                  << std::endl;
        return -1;
    }

    // 设置参数
    // Shape info
    uint32_t img_height = 1080;
    uint32_t img_width = 1920;
    uint32_t img_stride = img_width * sizeof(uint8_t);
    int32_t points_num = 500;

    // Custom Algo param
    uint8_t max_level = 3;
    uint8_t win_height = 21;
    uint8_t win_width = 21;
    uint32_t criteria_cnt = 20;
    float criteria_eps = 0.01;
    int32_t max_iters = 2000;
    float threshold = 1;
    float confidence = 0.9;

    // Calculate buffer size for allocating
    int img_size = img_height * img_stride;
    int32_t lk_tmp_size = 0;
    uint32_t pyr_width = img_width;
    uint32_t pyr_height = img_height;

    // calculate LK tmp buffer size 用于分配内存
    for (int32_t i = 0; i <= (int32_t)max_level; i++) {
        pyr_width = (pyr_width + 1) >> 1;
        pyr_height = (pyr_height + 1) >> 1;
        lk_tmp_size += pyr_width * pyr_height;
    }

    lk_tmp_size = lk_tmp_size * 2;
    int pts_size = points_num * 2 * sizeof(float);
    int status_size = points_num * sizeof(uint8_t);

    // Create AHWBuffer for input/output (User may prepare before MCVE func)
    // 在 Host 端为 MCVE/OpenCL 算法准备输入输出内存，使用了 Android 的 AHardwareBuffer（硬件共享内存）
    // 和 AndroidMemoryHelper 封装

    // memHelper：封装的内存分配助手类，用于申请 AHardwareBuffer
    // AHardwareBuffer*：Android 硬件共享 buffer 的指针，允许跨 CPU/GPU/DSP 使用
    // fd_*：每个 buffer 对应的 file descriptor，用于导入到 MCVE 或其他硬件接口
    // *_ptr：CPU 可访问的指针，用于填充数据或读取结果

    AndroidMemoryHelper memHelper;
    AHardwareBuffer *ahwb_prev_img, *ahwb_next_img, *ahwb_matrix, *ahwb_warp;   // buffer
    int fd_prev_img, fd_next_img, fd_matrix, fd_warp;
    uint8_t *prev_img_ptr, *next_img_ptr, *matrix_ptr, *warp_ptr;
    // mem_alloc:size(字节数)，cacheable(是否可缓存)，buffer(输出的AHardwareBuffer指针)，
    // buf_share_fd(输出的buffer对应的fd)，buf_va(输出的CPU可访问指针)
    memHelper.mem_alloc(img_size, false, &ahwb_prev_img, &fd_prev_img,
                        reinterpret_cast<void**>(&prev_img_ptr));
    memHelper.mem_alloc(img_size, false, &ahwb_next_img, &fd_next_img,
                        reinterpret_cast<void**>(&next_img_ptr));
    memHelper.mem_alloc(MATRIX_SIZE, false, &ahwb_matrix, &fd_matrix,
                        reinterpret_cast<void**>(&matrix_ptr));
    memHelper.mem_alloc(img_size, false, &ahwb_warp, &fd_warp,
                        reinterpret_cast<void**>(&warp_ptr));
    
    // img数据填充到AHardwareBuffer中
    ReadData(prev_img_ptr, img_size, prev_img_path);
    ReadData(next_img_ptr, img_size, next_img_path);

    int err = 0;

    // Init MCVE Environment
    mcv_env_t *mcv_env;
    if (MCV_SUCCESS != mcvInitEnv(&mcv_env)) {
        return -1;
    }

    // Import input/output buffer
    // mcvMemImport:导入外部 buffer 到 MCVE 环境中使用。第三个参数为上面分配的硬件的 fd 、错误码、buffer别名
    mcv_buffer_t *prev_img = mcvMemImport(mcv_env, img_size, &fd_prev_img, &err, "prev_img");
    CHECK_ERR(err, "mcvMemImport prev_img fail.");

    mcv_buffer_t *next_img = mcvMemImport(mcv_env, img_size, &fd_next_img, &err, "next_img");
    CHECK_ERR(err, "mcvMemImport next_img fail.");

    mcv_buffer_t *h_matrix = mcvMemImport(mcv_env, MATRIX_SIZE, &fd_matrix, &err, "h_matrix");
    CHECK_ERR(err, "mcvMemImport h_matrix fail.");

    mcv_buffer_t *warp_result = mcvMemImport(mcv_env, img_size, &fd_warp, &err, "warp_result");
    CHECK_ERR(err, "mcvMemImport warp_result fail.");


    // 普通的数据类型
    mcv_buffer_t *prev_pts = mcvMemAlloc(mcv_env, pts_size, &err, "prev_pts", BUF_TYPE);
    CHECK_ERR(err, "mcvMemAlloc prev_pts fail.");

    mcv_buffer_t *pts_num = mcvMemAlloc(mcv_env, sizeof(int32_t), &err, "pts_num", BUF_TYPE);
    CHECK_ERR(err, "mcvMemAlloc pts_num fail.");

    mcv_buffer_t *next_pts = mcvMemAlloc(mcv_env, pts_size, &err, "next_pts", BUF_TYPE);
    CHECK_ERR(err, "mcvMemAlloc next_pts fail.");

    mcv_buffer_t *status_buf = mcvMemAlloc(mcv_env, status_size, &err, "status", BUF_TYPE);
    CHECK_ERR(err, "mcvMemAlloc status_buf fail.");

    mcv_buffer_t *tmp_buf_lk = mcvMemAlloc(mcv_env, lk_tmp_size, &err, "tmp_buf_lk", BUF_TYPE);
    CHECK_ERR(err, "mcvMemAlloc tmp_buf_lk fail.");

    mcv_buffer_t *random_seed = mcvMemAlloc(mcv_env, SEED_SIZE, &err, "random_seed", BUF_TYPE);
    CHECK_ERR(err, "mcvMemAlloc random_seed fail.");

    // assign value into buffer
    // getHostPtr(prev_pts)：获取 prev_pts buffer 的 CPU 可访问指针
    ReadData(reinterpret_cast<uint8_t *>(getHostPtr(prev_pts)), pts_size, pts_path);

    // reinterpret_cast<uint8_t *>：把指针类型转换为 uint8_t*，方便按字节读取文件
    reinterpret_cast<int32_t *>(getHostPtr(pts_num))[0] = points_num;

    // 初始化随机种子
    uint32_t *random_seed_ptr = reinterpret_cast<uint32_t *>(getHostPtr(random_seed));
    for (int i = 0; i < 64; ++i) {
        random_seed_ptr[i] = i;
    }

    // buffer list which order should be the same as kernel
    std::vector<mcv_buffer_t *> buffer_list;
    buffer_list.push_back(prev_img);
    buffer_list.push_back(next_img);
    buffer_list.push_back(prev_pts);
    buffer_list.push_back(next_pts);
    buffer_list.push_back(pts_num);
    buffer_list.push_back(status_buf);
    buffer_list.push_back(tmp_buf_lk);
    buffer_list.push_back(random_seed);
    buffer_list.push_back(h_matrix);
    buffer_list.push_back(warp_result);

    // param_list which order should be the same as kernel
    std::vector<void *> param_list;
    param_list.push_back(&img_stride);
    param_list.push_back(&img_width);
    param_list.push_back(&img_height);
    param_list.push_back(&win_width);
    param_list.push_back(&win_height);
    param_list.push_back(&max_level);
    param_list.push_back(&criteria_cnt);
    param_list.push_back(&criteria_eps);
    param_list.push_back(&max_iters);
    param_list.push_back(&threshold);
    param_list.push_back(&confidence);

    // Create Custom Algo from binary
    // mcvCreateCustomAlgo入参？？？
    /**
        mcv_algo_t *algo = mcvCreateCustomAlgo(
        mcv_env,                 // MCVE 环境指针
        binary.data(),           // kernel 二进制数据
        binary.size(),           // kernel 二进制大小
        reinterpret_cast<mcv_buffer_t **>(buffer_list.data()), // buffer 列表
        buffer_list.size(),      // buffer 数量
        reinterpret_cast<void **>(param_list.data()),          // 普通参数列表
        param_list.size(),       // 参数数量
        &err                     // 错误码输出
    );
    */

    // buffer_list 的顺序必须严格和 kernel 参数顺序一致。
    // param_list 也是一样，对应 kernel 的标量参数。
    mcv_algo_t *algo = mcvCreateCustomAlgo(
        mcv_env, binary.data(), binary.size(),
        reinterpret_cast<mcv_buffer_t **>(buffer_list.data()), buffer_list.size(),
        reinterpret_cast<void **>(param_list.data()), param_list.size(), &err);

    // Run Algo
    /**
        algo → 要执行的算法对象
        0、NULL → 用于设置运行配置或依赖事件，这里没用
        &e_custom → 输出事件句柄，用于后续等待执行完成
     */
    mcv_event_t e_custom;
    mcvRunAlgo(algo, 0, NULL, &e_custom);
    mcvWaitForEvents(1, &e_custom);     // 等待一个事件

    // Compare output with golden
    bool is_bit_true = true;    // 对比最终的数据是否正确的flag
    std::vector<float> golden_matrix(9, 0.0);
    ReadData(reinterpret_cast<uint8_t *>(golden_matrix.data()), MATRIX_SIZE, h_matrix_path);    // 读取golden数据

    float *h_matrix_output = reinterpret_cast<float *>(matrix_ptr);
    for (int i = 0; i < 9; i++) {
        std::cout << h_matrix_output[i] << " ";
        if (std::fabs(h_matrix_output[i] - golden_matrix[i]) > 0.0001) {
            std::cout << i << ". " << std::fixed << h_matrix_output[i] << " should be " <<
                        std::fixed << golden_matrix[i] << std::endl;
            is_bit_true = false;
        }
    }
    std::cout << std::endl;

    std::vector<uint8_t> golden_warp(img_size, 0.0);
    ReadData(reinterpret_cast<uint8_t *>(golden_warp.data()), img_size, warp_golden_path);
    int fail_count = 0;
    for (int i = 0; i < img_height; i++) {
        for (int j = 0; j < img_width; j++) {
            if (warp_ptr[i * img_stride + j] != golden_warp[i * img_stride + j]) {
                printf("[%d,%d]. %d should be %d\n", i, j,
                        warp_ptr[i * img_stride + j], golden_warp[i * img_stride + j]);
                is_bit_true = false;
                fail_count++;
            }
            if(fail_count > 30) break;
        }
        if(fail_count > 30) break;
    }
    std::cout << std::endl;

    if (is_bit_true)
        std::cout << "Test success!!!" << std::endl;
    else
        std::cout << "Test fail." << std::endl;

    // Release event
    mcvReleaseEvent(&e_custom);

    // Release Algo
    mcvReleaseAlgo(algo);

    // Release Buffer
    // mcvMemFree for buffer from mcvMemAlloc, mcvMemUnImport for buffer from mcvMemImport
    mcvMemUnImport(prev_img);
    mcvMemUnImport(next_img);
    mcvMemUnImport(prev_pts);
    mcvMemUnImport(warp_result);

    mcvMemFree(next_pts);
    mcvMemFree(pts_num);
    mcvMemFree(status_buf);
    mcvMemFree(tmp_buf_lk);
    mcvMemFree(random_seed);
    mcvMemFree(h_matrix);

    // Release MCVE Environment
    mcvDeinitEnv(mcv_env);
    return 0;
}

// filename会被打开读取，文件内容会被存入data vector中
int LoadFile(std::vector<char> *data, const char *filename) {

    struct stat statbuf;    // 用于存储文件信息（大小 权限等）
    FILE *fh;   // 文件指针
    size_t file_len;    // 文件长度

    fh = fopen(filename, "rb"); // 以二进制只读方式打开文件
    if (fh == 0) {
        printf("Fail to Read : %s\n", filename);
        return -1;
    }
    printf("Load File: %s\n", filename);

    stat(filename, &statbuf);   // stat获取文件信息，存入statbuf结构体
    size_t file_data_len = static_cast<size_t>(statbuf.st_size);

    // 读取文件内容到data vector中
    char *file_data = reinterpret_cast<char *>(malloc(file_data_len + 1));
    fread(file_data, file_data_len, 1, fh);
    file_data[file_data_len] = '\0';
    fclose(fh);

    //  将读取的数据存入data vector
    (*data).insert((*data).end(), file_data, file_data + file_data_len);
    free(file_data);
    return 0;
}

//  buffer：用户提供的内存指针，用于存放文件内容
//  buffer_size：buffer 的大小（字节数）
//  data_file_path：待读取的文件路径
bool ReadData(uint8_t *buffer, uint32_t buffer_size, const char *data_file_path) {
    //  打开文件
    std::ifstream in;
    in.open(data_file_path, std::ios::in | std::ios::binary);
    if (!in.good()) {
        printf("Fail to read file %s\n", data_file_path);
        return false;
    }

    //  获取文件大小并与 buffer_size 比较
    in.seekg(0, in.end);    // 把读取指针移动到文件末尾
    int fileSize = in.tellg();  // 获取当前位置的偏移量，即文件大小
    if (fileSize != buffer_size) {
        printf("fileSize(%d) and input BufferSize(%d) are not match.\n", fileSize, buffer_size);
        return false;
    }

    in.seekg(0, in.beg);    // 将读取指针移回文件开头
    in.read(reinterpret_cast<char *>(buffer), buffer_size); // 把文件内容读入 buffer 中
    in.close();
    return true;
}
