/**
 * \file
 * Types.h
 * ---
 * Common type definitions.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

typedef struct native_handle
{
    int version;        /* sizeof(native_handle_t) */
    int numFds;         /* number of file-descriptors at &data[0] */
    int numInts;        /* number of ints at &data[numFds] */
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-length-array"
#endif
    int data[0];        /* numFds + numInts ints */
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
} native_handle_t;

typedef const native_handle_t* buffer_handle_t;

enum {
    GRALLOC_EXTRA_OK,
    GRALLOC_EXTRA_UNKNOWN,
    GRALLOC_EXTRA_NOT_SUPPORTED,
    GRALLOC_EXTRA_NO_IMPLEMENTATION,
    GRALLOC_EXTRA_NOT_INIT,
    GRALLOC_EXTRA_INVALIDE_PARAMS,
    GRALLOC_EXTRA_INVALIDE_OPERATION,
    GRALLOC_EXTRA_ERROR,
};

/* enum for query() */
typedef enum {
    GRALLOC_EXTRA_GET_ION_FD = 1,    /* int */
    GRALLOC_EXTRA_GET_FB_MVA,        /* uintptr_t, deprecated */
    GRALLOC_EXTRA_GET_SECURE_HANDLE, /* uint32_t */

    /* output: int */
    GRALLOC_EXTRA_GET_WIDTH = 10,
    GRALLOC_EXTRA_GET_HEIGHT,
    GRALLOC_EXTRA_GET_STRIDE,
    GRALLOC_EXTRA_GET_VERTICAL_STRIDE,
    GRALLOC_EXTRA_GET_ALLOC_SIZE,
    GRALLOC_EXTRA_GET_FORMAT,
    GRALLOC_EXTRA_GET_REQ_FORMAT,
    GRALLOC_EXTRA_GET_USAGE,
    GRALLOC_EXTRA_GET_VERTICAL_2ND_STRIDE,
    GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE,
    GRALLOC_EXTRA_GET_PRIV_FLAGS,

    /* output: uint64_t */
    GRALLOC_EXTRA_GET_ID = 50,
    GRALLOC_EXTRA_GET_CONSUMER_USAGE,
    GRALLOC_EXTRA_GET_PRODUCER_USAGE,
    GRALLOC_EXTRA_GET_ALLOC_FORMAT,

    /* output: gralloc_extra_sf_info_t */
    GRALLOC_EXTRA_GET_SF_INFO = 100,
    GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO = GRALLOC_EXTRA_GET_SF_INFO, /* deprecated */

    /* output: gralloc_extra_smvr_info_t */
    GRALLOC_EXTRA_GET_SMVR_INFO,

    /* output: gralloc_extra_buf_debug_t */
    GRALLOC_EXTRA_GET_IOCTL_ION_DEBUG,

    /* output: uint32_t */
    GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC,

    /* output: ge_rotate_info_t */
    GRALLOC_EXTRA_GET_ROTATE_INFO,

    /* output: gralloc_gpu_compression_info_t */
    GRALLOC_EXTRA_GET_GPU_COMPRESSION_INFO,

    /* output: gralloc_gpu_yuyv rotation: int32_t */
    GRALLOC_EXTRA_GET_ORIENTATION,

    /* output: ge_hdr_info_t */
    GRALLOC_EXTRA_GET_HDR_INFO,

    /* output: ge_hdr10p_dynamic_metadata_t */
    GRALLOC_EXTRA_GET_HDR10P_INFO,

    /* output: ge_video_info_t */
    GRALLOC_EXTRA_GET_VIDEO_INFO,

    /* output: ge_hwc_info_t */
    GRALLOC_EXTRA_GET_HWC_INFO,

    /* output: buffer timestmp: uint64_t */
    GRALLOC_EXTRA_GET_TIMESTAMP,

    /* output: ge_timestmp_info_t */
    GRALLOC_EXTRA_GET_TIMESTAMP_INFO,

    /* output: ge_pq__scltm_info_t */
    GRALLOC_EXTRA_GET_PQ_SCLTM_INFO,

    /* output: ge_pq_mira_vision_info_t */
    GRALLOC_EXTRA_GET_PQ_MIRA_VISION_INFO,

    /* output: ge_pq_qp_sum_info_t */
    GRALLOC_EXTRA_GET_PQ_QP_SUM_INFO,

    /* output: ge_hdr_f_info_t */
    GRALLOC_EXTRA_GET_HDR_F_INFO,

    /* output: ge_hdr_prop_t */
    GRALLOC_EXTRA_GET_HDR_PROP,

    /* output: ge_ai_pq_info_t */
    GRALLOC_EXTRA_GET_AI_PQ_INFO,

    /* output: ge_ai_pq_model_path_t */
    GRALLOC_EXTRA_GET_AI_PQ_MODEL_PATH,
} GRALLOC_EXTRA_ATTRIBUTE_QUERY;

__END_DECLS

