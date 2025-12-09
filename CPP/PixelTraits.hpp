#ifndef __PIEXLTRAITS_HPP__
#define __PIEXLTRAITS_HPP__

#include <cstdint>
#include "half.hpp"

/***************************************************************
 *  Pixel Traits
 ***************************************************************/
template <typename Tin, typename Tout>
struct PixelTraits;

// -----------------------------
//  Tin = float, Tout = float
// -----------------------------
template <>
struct PixelTraits<float, float>
{
    static inline float toScalar(float v) { return v; }
    static inline float fromScalar(float v) { return v; }
};

// -----------------------------
//  Tin = float, Tout = uint16_t
// -----------------------------
template <>
struct PixelTraits<float, uint16_t>
{
    static inline float toScalar(float v) { return v; }
    static inline uint16_t fromScalar(double v) { return static_cast<uint16_t>(v); }
};

// -----------------------------
//  Tin = uint16_t, Tout = uint16_t
// -----------------------------
template <>
struct PixelTraits<uint16_t, uint16_t>
{
    static inline float toScalar(uint16_t v) { return static_cast<float>(v); }
    static inline uint16_t fromScalar(double v) { return static_cast<uint16_t>(v); }
};

// -----------------------------
//  Tin = half, Tout = half
// -----------------------------
using half = half_float::half;

template <>
struct PixelTraits<half, half>
{
    static inline float toScalar(half v) { return half_float::half_cast<float>(v); }
    static inline half fromScalar(float v) { return half_float::half_cast<half>(v); }
};

// -----------------------------
//  Tin = half, Tout = uint16_t
// -----------------------------
template <>
struct PixelTraits<half, uint16_t>
{
    static inline float toScalar(half v) { return half_float::half_cast<float>(v); }
    static inline uint16_t fromScalar(float v) { return static_cast<uint16_t>(v * 65535.0f); }
};

// -----------------------------
//  Tin = uint16_t, Tout = half
// -----------------------------
template <>
struct PixelTraits<uint16_t, half>
{
    static inline float toScalar(uint16_t v) { return static_cast<float>(v / 65535.0f); }
    static inline half fromScalar(float v) { return half_float::half_cast<half>(v); }
};

#endif  // __PIEXLTRAITS_HPP__


 /***************************************************************
     *  AWB HighlightKeep Implementation
     ***************************************************************/
    template <typename Tin, typename Tout>
    void applyAwbHighlightKeep(const VImage* src, VImage* dst, const VAwbInfo* awbInfo, float f32ScaleRatio,
                               float f32MaxValue, const VRect& roi)
    {
        const size_t top = roi.top;
        const size_t left = roi.left;
        const size_t bottom = roi.bottom;
        const size_t right = roi.right;

        for (size_t row = top; row < bottom; ++row)
        {
            const Tin* lineIn = reinterpret_cast<const Tin*>(src->data[0] + row * src->stride[0]);
            Tout* lineOut = reinterpret_cast<Tout*>(dst->data[0] + row * dst->stride[0]);

            for (size_t col = left; col < right; ++col)
            {
                // clip input RGB to [0, f32MaxValue]
                float r = SYS_CLIP(lineIn[0], 0, f32MaxValue);
                float g = SYS_CLIP(lineIn[1], 0, f32MaxValue);
                float b = SYS_CLIP(lineIn[2], 0, f32MaxValue);

                // apply per-channel AWB gain
                float r_awb = SYS_CLIP(r * awbInfo->redGain / f32ScaleRatio, 0, f32MaxValue);
                float g_awb = SYS_CLIP(g * awbInfo->greenGain / f32ScaleRatio, 0, f32MaxValue);
                float b_awb = SYS_CLIP(b * awbInfo->blueGain / f32ScaleRatio, 0, f32MaxValue);

                float gBlendingValue = 0.0f;
                float rbBlendingValue = 0.0f;
                // compute blending weight for green channel highlight
                LinearMappingPositiveFloat(&gBlendingValue, g, 0.90f * f32MaxValue, 0.98f * f32MaxValue, 0.0f, 1);
                // compute RB blending based on dominant gain
                rbBlendingValue = (awbInfo->redGain > awbInfo->blueGain)
                                      ? (LinearMappingPositiveFloat(&rbBlendingValue, b, 0.90f * f32MaxValue,
                                                                    0.98f * f32MaxValue, 0.0f, 1),
                                         rbBlendingValue)
                                      : (LinearMappingPositiveFloat(&rbBlendingValue, r, 0.90f * f32MaxValue,
                                                                    0.98f * f32MaxValue, 0.0f, 1),
                                         rbBlendingValue);
                // intermediate RB blending values for highlight correction
                float rbInterValueR =
                    rbBlendingValue * rbBlendingValue * b_awb + (1 - rbBlendingValue * rbBlendingValue) * r_awb;
                float rbInterValueB =
                    rbBlendingValue * rbBlendingValue * r_awb + (1 - rbBlendingValue * rbBlendingValue) * b_awb;

                // adjust R and B channels using blending
                r_awb = (awbInfo->redGain > awbInfo->blueGain) ? r_awb : SYS_MAX(rbInterValueR, r_awb);
                b_awb = (awbInfo->redGain > awbInfo->blueGain) ? SYS_MAX(rbInterValueB, b_awb) : b_awb;

                // blend G channel toward mixed RB value for highlight consistency
                float interValue = (r_awb + b_awb) / 2;
                float gInterValue =
                    gBlendingValue * gBlendingValue * interValue + (1 - gBlendingValue * gBlendingValue) * g_awb;
                g_awb = SYS_MAX(gInterValue, g_awb);

                // write output RGB with digital gain and scale ratio
                lineOut[0] = r_awb * f32ScaleRatio * awbInfo->digitalGain;
                lineOut[1] = g_awb * f32ScaleRatio * awbInfo->digitalGain;
                lineOut[2] = b_awb * f32ScaleRatio * awbInfo->digitalGain;

                lineIn += 3;
                lineOut += 3;
            }
        }
    }

    /***************************************************************
     *  AWB Implementation
     ***************************************************************/
    template <typename Tin, typename Tout>
    void applyAwb(const VImage* src, VImage* dst, const VAwbInfo* awbInfo, const VRect& roi)
    {

        const size_t top = roi.top;
        const size_t left = roi.left;
        const size_t bottom = roi.bottom;
        const size_t right = roi.right;

        for (size_t row = top; row < bottom; ++row)
        {
            const Tin* lineIn = reinterpret_cast<const Tin*>(src->data[0] + row * src->stride[0]);
            Tout* lineOut = reinterpret_cast<Tout*>(dst->data[0] + row * dst->stride[0]);

            for (size_t col = left; col < right; ++col)
            {
                float r = lineIn[0];
                float g = lineIn[1];
                float b = lineIn[2];

                float r_awb = r * awbInfo->redGain * awbInfo->digitalGain;
                float g_awb = g * awbInfo->greenGain * awbInfo->digitalGain;
                float b_awb = b * awbInfo->blueGain * awbInfo->digitalGain;

                lineOut[0] = r_awb;
                lineOut[1] = g_awb;
                lineOut[2] = b_awb;

                lineIn += 3;
                lineOut += 3;
            }
        }
    }
	
	
