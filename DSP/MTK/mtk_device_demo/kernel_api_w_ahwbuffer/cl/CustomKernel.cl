/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2023. All rights reserved.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#include "mtk_cv_calibration_3d.h"
#include "mtk_cv_imgproc.h"
#include "mtk_cv_video.h"

__kernel void CustomKernel(
    __global uchar *prev_img,
    __global uchar *next_img,
    __global float *prev_pts,
    __global float *next_pts,
    __global int *pts_num,
    __global uchar *status,
    __global uchar *tmp_buf_lk,
    __global uint *random_seed,
    __global float *homography_matrix,
    __global uchar *warp_result,
    __fast_global uchar *tmp_buf,
    uint ui_tmp_buf_size,
    uint stride_img,
    uint width_img,
    uint height_img,
    uchar win_width,
    uchar win_height,
    uchar max_level,
    uint criteria_cnt,
    float criteria_eps,
    int max_iters,
    float threshold,
    float confidence)
{
    // CalcOpticalFlow
    mtkapi_calcopticalflowpyrlk_u8f32_with_search_range(
        prev_img,
        next_img,
        prev_pts,
        pts_num,
        next_pts,
        status,
        tmp_buf_lk,
        tmp_buf,
        ui_tmp_buf_size,
        stride_img,
        stride_img,
        width_img,
        height_img,
        win_width,
        win_height,
        max_level,
        criteria_cnt,
        criteria_eps,
        0);

    // FindHomography
    mtkapi_findhomography_ransac_f32(
        prev_pts,
        next_pts,
        pts_num,
        homography_matrix,
        tmp_buf,
        random_seed,
        threshold,
        max_iters,
        confidence);

    // Warpperspective
    mtkapi_warpperspective_bl_u8(
        next_img,
        homography_matrix,
        warp_result,
        tmp_buf,
        ui_tmp_buf_size,
        stride_img,
        stride_img,
        width_img,
        height_img,
        width_img,
        height_img,
        0,  // constant
        0);
}

