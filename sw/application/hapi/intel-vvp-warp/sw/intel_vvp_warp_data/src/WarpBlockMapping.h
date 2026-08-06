/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <cstdint>
#include "WarpDataContext.h"
#include "WarpMesh.h"
#include "WarpDataUtils.h"

#ifndef DONT_USE_SIMD
#if defined (__ARM_NEON)
#include "arm_neon.h"
#elif defined (__SSE__)
#include <emmintrin.h>
#include <smmintrin.h>
#elif defined (__AVX2__)
#include <immintrin.h>
#endif /* __ARM_NEON / __SSE__ / __AVX2__ */
#endif /* DONT_USE_SIMD */


namespace intel_vvp_warp
{

// Max allowed number of input blocks
// an output block can use (defined by RTL filter grid)
static constexpr uint32_t BLOCK_GRID_WIDTH      = 4;
static constexpr uint32_t BLOCK_GRID_HEIGHT     = 6;
static constexpr uint32_t MAX_BLOCK_SPAN        = BLOCK_GRID_HEIGHT * BLOCK_GRID_WIDTH;


// Raster scan implementation
struct block_ref_t
{
    int16_t _v;
    int16_t _h;		
    uint8_t _ref_begin;
    uint8_t _ref_end;
    uint8_t _in_cache;
};

using block_list_t = block_ref_t[MAX_BLOCK_SPAN];

struct block_descriptor_t
{
    block_list_t _block_list;
    int16_t _min_v;
    int16_t _min_h;
    int16_t _max_v;
    int16_t _max_h;
    uint8_t _block_list_size;
    uint8_t _block_coef[2];		// [h, v]
    uint8_t _mm_level;			// mipmap level	
};


template<uint32_t N>
inline std::pair<float, float> EstimateBlockScale(const mesh_node_t* row0, const mesh_node_t* row1, const uint32_t num_streams, const int32_t block_width, const int32_t block_height)
{
	auto get_line_scale_squared = [](const mesh_node_t& p0, const mesh_node_t& p1, const uint32_t length_ref)->float
	{
		const int32_t dx = p1._x - p0._x;
		const int32_t dy = p1._y - p0._y;
		const float length_squared = static_cast<float>(dx * dx + dy * dy);
		const float scale_squared = length_squared / static_cast<float>(length_ref * length_ref);
		return scale_squared;
	};

	const uint32_t offset = num_streams * (N-1);
	const float sh0 = get_line_scale_squared(row0[0], row0[offset], block_width);
	const float sh1 = get_line_scale_squared(row1[0], row1[offset], block_width);
	const float sh = sqrtf(std::max(sh0, sh1));
	const float sv0 = get_line_scale_squared(row0[0], row1[0], block_height);
	const float sv1 = get_line_scale_squared(row0[offset], row1[offset], block_height);
	const float sv = sqrtf(std::max(sv0, sv1));

	return std::make_pair(sh, sv);
}


inline uint32_t GetMipmapLevel(const float fscale)
{
    static constexpr uint32_t MAX_MIPMAP_SCALE		= 128;	// Max downscale ratio currently supported: 128:1
    const uint32_t uscale = std::clamp(static_cast<uint32_t>(floorf(fscale)), static_cast<uint32_t>(1), MAX_MIPMAP_SCALE);
    const uint32_t mm_scale = get_prev_pwr_two(uscale);
    return mm_scale_to_level(mm_scale);
}


inline bool CheckCompressionLimit(const float scale)
{
    static constexpr float MAX_COMPRESSION_RATE = 2.012f;
    return (scale < MAX_COMPRESSION_RATE);
}


inline bool AddInputBlock(const WarpHwContext* const hw, uint32_t osv, int32_t x, int32_t y, block_descriptor_t& bd)
{
	const int32_t block_height = static_cast<int32_t>(hw->_block_height);
	const int32_t block_width = static_cast<int32_t>(hw->_block_width);

#ifdef DONT_USE_DIV
	const int16_t bv = static_cast<int16_t>(y >> 3);
	const int16_t bh = static_cast<int16_t>(x >> 4);
#else
	// Negative values are rounded towards negative infinity
	const int16_t bv = ((y < 0) ? (y - block_height + 1) : y) / block_height;
	const int16_t bh = ((x < 0) ? (x - block_width + 1) : x) / block_width;
#endif /* DONT_USE_DIV */

	auto add_block = [](const uint32_t osv, const int16_t bv, const int16_t bh, block_descriptor_t& bd)->bool
	{
		bool ret = (bd._block_list_size < MAX_BLOCK_SPAN);

		if(ret)
		{
            // Place the block being searched for at the back of the list
			block_ref_t* pe = bd._block_list + bd._block_list_size;
			pe->_v = bv;
			pe->_h = bh;

			block_ref_t* pb = bd._block_list;

            // The loop below is the hottest spot of the warp data generator
            // The condition: ((pb->_v != bv) || (pb->_h != bh))
            // compiles into 3 instructions on Arm64 gcc with -O3

            // Look for the block
            while((pb->_v != bv) || (pb->_h != bh))
				++pb;

            // We are going to find either pre-existing block
            // or the one we've just placed at the back
			pb->_ref_end = osv + 1;

			if(pb == pe) // If it's a new block
			{
				pb->_ref_begin = osv;

                bd._min_v = std::min(bd._min_v, bv);
                bd._min_h = std::min(bd._min_h, bh);
                bd._max_v = std::max(bd._max_v, bv);
                bd._max_h = std::max(bd._max_h, bh);

				bd._block_list_size++;
			}		
		}

		return ret;
	};    

	bool ret = add_block(osv, bv, bh, bd);

#ifdef DONT_USE_DIV
    const int32_t y_rm = y & (block_height - 1);
    const int32_t x_rm = x & (block_width - 1);	
#else
    const int32_t y_rm = y % block_height;
    const int32_t x_rm = x % block_width;
#endif /* DONT_USE_DIV */

    const int32_t vmax = (y < 0) ? - 2 : (block_height - 2);
    const int32_t hmax = (x < 0) ? - 2 : (block_width - 2);
    const int32_t extra_v = (y_rm == 0) ? -1 : (int32_t)(y_rm >= vmax);
    const int32_t extra_h = (x_rm == 0) ? -1 : (int32_t)(x_rm >= hmax);

    if(extra_h) ret = ret && add_block(osv, bv, bh + extra_h, bd);
    if(extra_v) ret = ret && add_block(osv, bv + extra_v, bh, bd);
    if(extra_v && extra_h ) ret = ret && add_block(osv, bv + extra_v, bh + extra_h, bd);

	return ret;
}

template<bool MIPMAP_ENABLE>
inline bool OtoI_2x3_Sb(const WarpDataContext& ctx, const mesh_node_t* r0, const mesh_node_t* r1, const uint32_t num_streams, const uint32_t frac_prec, block_descriptor_t& bd)
{
	bool ret = true;

	// Bring to the mesh format (N fraction bits)
	const int32_t block_height = static_cast<int32_t>(ctx._hw->_block_height << frac_prec);
	const int32_t block_width = static_cast<int32_t>(ctx._hw->_block_width << frac_prec);
    // Extra pixels for bicubic filter
    const int32_t extra = 2 << frac_prec;
	const int32_t width = ctx._width_in << frac_prec;
    const int32_t height = ctx._height_in << frac_prec;

	const WarpHwContext* const hw = ctx._hw.get();

    //Pre-initialize compression factors to zero
    float scale_h = 1.0f;
    float scale_v = 1.0f;
	uint32_t mipmap_level = 0;

    // Pre-initialize block's min and max values
    bd._min_v = bd._min_h = std::numeric_limits<int16_t>::max();
	bd._max_v = bd._max_h = std::numeric_limits<int16_t>::min();    

    // Quick check if entire block is outside screen
    for(uint32_t s = 0; s < num_streams; ++s)
    {
        const uint32_t _0 = s;
        const uint32_t _1 = s + num_streams;
        const uint32_t _2 = s + num_streams + num_streams;

        const auto [minh, maxh] = std::minmax({r0[_0]._x, r0[_1]._x, r0[_2]._x, r1[_0]._x, r1[_1]._x, r1[_2]._x});
        const auto [minv, maxv] = std::minmax({r0[_0]._y, r0[_1]._y, r0[_2]._y, r1[_0]._y, r1[_1]._y, r1[_2]._y});

		const int32_t block_minh = minh - (1 << frac_prec);
		const int32_t block_minv = minv - (1 << frac_prec);
		const int32_t block_maxh = maxh + extra;
		const int32_t block_maxv = maxv + extra;

		// Do not process the block if it is entirely outside the screen
		if((block_minh >= width) || (block_maxh < 0) || (block_minv >= height) || (block_maxv < 0))
			continue;		

        // Estimate local compression
        const auto [scale_h_tmp, scale_v_tmp] = EstimateBlockScale<3>(r0 + s, r1 + s, num_streams, block_width, block_height);
        scale_h = std::max(scale_h, scale_h_tmp);
        scale_v = std::max(scale_v, scale_v_tmp);
        
        if constexpr (MIPMAP_ENABLE)
            mipmap_level = GetMipmapLevel(std::max(scale_h_tmp, scale_v_tmp));

#if defined (__ARM_NEON) && ! defined (DONT_USE_SIMD)

        // Linearly interpolated top X,Y
        int32x4_t tx0, tx1, tx2, tx3;
        int32x4_t ty0, ty1, ty2, ty3;

        // Linearly interpolated bottom X,Y
        int32x4_t bx0, bx1, bx2, bx3;
        int32x4_t by0, by1, by2, by3;

        // Top 3x points
        const int32x4_t x0 = vdupq_n_s32(r0[_0]._x);
        const int32x4_t y0 = vdupq_n_s32(r0[_0]._y);
        const int32x4_t x1 = vdupq_n_s32(r0[_1]._x);
        const int32x4_t y1 = vdupq_n_s32(r0[_1]._y);
        const int32x4_t x2 = vdupq_n_s32(r0[_2]._x);
        const int32x4_t y2 = vdupq_n_s32(r0[_2]._y);

        // Bottom 3x points
        const int32x4_t x3 = vdupq_n_s32(r1[_0]._x);
        const int32x4_t y3 = vdupq_n_s32(r1[_0]._y);
        const int32x4_t x4 = vdupq_n_s32(r1[_1]._x);
        const int32x4_t y4 = vdupq_n_s32(r1[_1]._y);
        const int32x4_t x5 = vdupq_n_s32(r1[_2]._x);
        const int32x4_t y5 = vdupq_n_s32(r1[_2]._y);

        const  int32x4_t v8 = vdupq_n_s32(8);

        {
            int32x4_t k = { 0, 1, 2, 3};
            int32x4_t kr = vsubq_s32(v8, k);

            {
                // Left top and bottom
                const int32x4_t px0 = vmulq_s32(x0, kr);
                const int32x4_t py0 = vmulq_s32(y0, kr);
                const int32x4_t px2 = vmulq_s32(x3, kr);
                const int32x4_t py2 = vmulq_s32(y3, kr);

                // Right top and bottom
                const int32x4_t px1 = vmulq_s32(x1, k);
                const int32x4_t py1 = vmulq_s32(y1, k);
                const int32x4_t px3 = vmulq_s32(x4, k);
                const int32x4_t py3 = vmulq_s32(y4, k);

                tx0 = vaddq_s32(px0, px1);
                ty0 = vaddq_s32(py0, py1);
                bx0 = vaddq_s32(px2, px3);
                by0 = vaddq_s32(py2, py3);
            }

            {
                // Left top and bottom
                const int32x4_t px0 = vmulq_s32(x1, kr);
                const int32x4_t py0 = vmulq_s32(y1, kr);
                const int32x4_t px2 = vmulq_s32(x4, kr);
                const int32x4_t py2 = vmulq_s32(y4, kr);

                // Right top and bottom
                const int32x4_t px1 = vmulq_s32(x2, k);
                const int32x4_t py1 = vmulq_s32(y2, k);
                const int32x4_t px3 = vmulq_s32(x5, k);
                const int32x4_t py3 = vmulq_s32(y5, k);

                tx2 = vaddq_s32(px0, px1);
                ty2 = vaddq_s32(py0, py1);
                bx2 = vaddq_s32(px2, px3);
                by2 = vaddq_s32(py2, py3);
            }            
        }

        {
            int32x4_t k = {4, 5, 6, 7};
            int32x4_t kr = vsubq_s32(v8, k);

            {
                // Left top and bottom
                const int32x4_t px0 = vmulq_s32(x0, kr);
                const int32x4_t py0 = vmulq_s32(y0, kr);
                const int32x4_t px2 = vmulq_s32(x3, kr);
                const int32x4_t py2 = vmulq_s32(y3, kr);

                // Right top and bottom
                const int32x4_t px1 = vmulq_s32(x1, k);
                const int32x4_t py1 = vmulq_s32(y1, k);
                const int32x4_t px3 = vmulq_s32(x4, k);
                const int32x4_t py3 = vmulq_s32(y4, k);

                tx1 = vaddq_s32(px0, px1);
                ty1 = vaddq_s32(py0, py1);
                bx1 = vaddq_s32(px2, px3);
                by1 = vaddq_s32(py2, py3);
            }

            {
                // Left top and bottom
                const int32x4_t px0 = vmulq_s32(x1, kr);
                const int32x4_t py0 = vmulq_s32(y1, kr);
                const int32x4_t px2 = vmulq_s32(x4, kr);
                const int32x4_t py2 = vmulq_s32(y4, kr);

                // Right top and bottom
                const int32x4_t px1 = vmulq_s32(x2, k);
                const int32x4_t py1 = vmulq_s32(y2, k);
                const int32x4_t px3 = vmulq_s32(x5, k);
                const int32x4_t py3 = vmulq_s32(y5, k);

                tx3 = vaddq_s32(px0, px1);
                ty3 = vaddq_s32(py0, py1);
                bx3 = vaddq_s32(px2, px3);
                by3 = vaddq_s32(py2, py3);
            }            
        }

        const int32x4_t rounding_offset = vdupq_n_s32((1 << mipmap_level) >> 1);

		for( int32_t sv = 0; sv < 8; ++sv )
		{
            const int32x4_t k = vdupq_n_s32(sv);
            const int32x4_t kr = vsubq_s32(v8, k);

            auto interpolate_blocks = [&](const int32x4_t& tx, const int32x4_t& ty, const int32x4_t& bx, const int32x4_t& by){

                const int32x4_t ptx = vmulq_s32(tx, kr);
                const int32x4_t pty = vmulq_s32(ty, kr);
                const int32x4_t pbx = vmulq_s32(bx, k);
                const int32x4_t pby = vmulq_s32(by, k);

                const int32x4_t vx = vshrq_n_s32(vaddq_s32(vshrq_n_s32(vaddq_s32(ptx, pbx), 10), rounding_offset), mipmap_level);
                const int32x4_t vy = vshrq_n_s32(vaddq_s32(vshrq_n_s32(vaddq_s32(pty, pby), 10), rounding_offset), mipmap_level);

                int32_t x[4];
                int32_t y[4];

                vst1q_s32(x, vx);
                vst1q_s32(y, vy);

                ret = ret && AddInputBlock(hw, sv, x[0], y[0], bd);
                ret = ret && AddInputBlock(hw, sv, x[1], y[1], bd);
                ret = ret && AddInputBlock(hw, sv, x[2], y[2], bd);
                ret = ret && AddInputBlock(hw, sv, x[3], y[3], bd);
            };

            interpolate_blocks(tx0, ty0, bx0, by0);
            interpolate_blocks(tx1, ty1, bx1, by1);
            interpolate_blocks(tx2, ty2, bx2, by2);
            interpolate_blocks(tx3, ty3, bx3, by3);         
        }        

#elif defined (__SSE__) && !defined (DONT_USE_SIMD)

		// Linearly interpolated top X,Y
        __m128i tx0, tx1, tx2, tx3;
        __m128i ty0, ty1, ty2, ty3;

        // Linearly interpolated bottom X,Y
        __m128i bx0, bx1, bx2, bx3;
        __m128i by0, by1, by2, by3;

		// Load top 3x points
		const __m128i x0 = _mm_set1_epi32(r0[_0]._x);
		const __m128i y0 = _mm_set1_epi32(r0[_0]._y);
		const __m128i x1 = _mm_set1_epi32(r0[_1]._x);
		const __m128i y1 = _mm_set1_epi32(r0[_1]._y);
		const __m128i x2 = _mm_set1_epi32(r0[_2]._x);
		const __m128i y2 = _mm_set1_epi32(r0[_2]._y);

		// Load bottom 3x points
		const __m128i x3 = _mm_set1_epi32(r1[_0]._x);
		const __m128i y3 = _mm_set1_epi32(r1[_0]._y);
		const __m128i x4 = _mm_set1_epi32(r1[_1]._x);
		const __m128i y4 = _mm_set1_epi32(r1[_1]._y);
		const __m128i x5 = _mm_set1_epi32(r1[_2]._x);
		const __m128i y5 = _mm_set1_epi32(r1[_2]._y);

		const __m128i v8 = _mm_set1_epi32(8);

		{
			const __m128i k = _mm_set_epi32( 0 , 1,  2,  3);
			const __m128i kr = _mm_sub_epi32(v8, k);

			{
				// Left top and bottom
				const __m128i px0 = _mm_mullo_epi32(x0, kr);
				const __m128i py0 = _mm_mullo_epi32(y0, kr);
				const __m128i px2 = _mm_mullo_epi32(x3, kr);
				const __m128i py2 = _mm_mullo_epi32(y3, kr);

				// Right top and bottom
				const __m128i px1 = _mm_mullo_epi32(x1, k);
				const __m128i py1 = _mm_mullo_epi32(y1, k);
				const __m128i px3 = _mm_mullo_epi32(x4, k);
				const __m128i py3 = _mm_mullo_epi32(y4, k);

				tx0 = _mm_add_epi32(px0, px1);
				ty0 = _mm_add_epi32(py0, py1);
				bx0 = _mm_add_epi32(px2, px3);
				by0 = _mm_add_epi32(py2, py3);
			}

			{
				// Left top and bottom
				const __m128i px0 = _mm_mullo_epi32(x1, kr);
				const __m128i py0 = _mm_mullo_epi32(y1, kr);
				const __m128i px2 = _mm_mullo_epi32(x4, kr);
				const __m128i py2 = _mm_mullo_epi32(y4, kr);

				// Right top and bottom
				const __m128i px1 = _mm_mullo_epi32(x2, k);
				const __m128i py1 = _mm_mullo_epi32(y2, k);
				const __m128i px3 = _mm_mullo_epi32(x5, k);
				const __m128i py3 = _mm_mullo_epi32(y5, k);

				tx2 = _mm_add_epi32(px0, px1);
				ty2 = _mm_add_epi32(py0, py1);
				bx2 = _mm_add_epi32(px2, px3);
				by2 = _mm_add_epi32(py2, py3);
			}
		}

		{
			const __m128i k = _mm_set_epi32( 4 , 5,  6,  7);
			const __m128i kr = _mm_sub_epi32(v8, k);

			{
				// Left top and bottom
				const __m128i px0 = _mm_mullo_epi32(x0, kr);
				const __m128i py0 = _mm_mullo_epi32(y0, kr);
				const __m128i px2 = _mm_mullo_epi32(x3, kr);
				const __m128i py2 = _mm_mullo_epi32(y3, kr);

				// Right top and bottom
				const __m128i px1 = _mm_mullo_epi32(x1, k);
				const __m128i py1 = _mm_mullo_epi32(y1, k);
				const __m128i px3 = _mm_mullo_epi32(x4, k);
				const __m128i py3 = _mm_mullo_epi32(y4, k);

				tx1 = _mm_add_epi32(px0, px1);
				ty1 = _mm_add_epi32(py0, py1);
				bx1 = _mm_add_epi32(px2, px3);
				by1 = _mm_add_epi32(py2, py3);
			}

			{
				// Left top and bottom
				const __m128i px0 = _mm_mullo_epi32(x1, kr);
				const __m128i py0 = _mm_mullo_epi32(y1, kr);
				const __m128i px2 = _mm_mullo_epi32(x4, kr);
				const __m128i py2 = _mm_mullo_epi32(y4, kr);

				// Right top and bottom
				const __m128i px1 = _mm_mullo_epi32(x2, k);
				const __m128i py1 = _mm_mullo_epi32(y2, k);
				const __m128i px3 = _mm_mullo_epi32(x5, k);
				const __m128i py3 = _mm_mullo_epi32(y5, k);

				tx3 = _mm_add_epi32(px0, px1);
				ty3 = _mm_add_epi32(py0, py1);
				bx3 = _mm_add_epi32(px2, px3);
				by3 = _mm_add_epi32(py2, py3);
			}
		}

		const __m128i rounding_offset = _mm_set1_epi32((1 << mipmap_level) >> 1);
		const __m128i v_mipmap_level = _mm_set_epi32(0, 0, 0, mipmap_level);

		for( int32_t sv = 0; sv < 8; ++sv )
		{
            const __m128i k = _mm_set1_epi32(sv);
            const __m128i kr = _mm_sub_epi32(v8, k);

            auto interpolate_pixels = [&](const __m128i& tx, const __m128i& ty, const __m128i& bx, const __m128i& by){

                const __m128i ptx = _mm_mullo_epi32(tx, kr);
                const __m128i pty = _mm_mullo_epi32(ty, kr);
                const __m128i pbx = _mm_mullo_epi32(bx, k);
                const __m128i pby = _mm_mullo_epi32(by, k);

                const __m128i vx = _mm_sra_epi32(_mm_add_epi32(_mm_srai_epi32(_mm_add_epi32(ptx, pbx), 10), rounding_offset), v_mipmap_level);
                const __m128i vy = _mm_sra_epi32(_mm_add_epi32(_mm_srai_epi32(_mm_add_epi32(pty, pby), 10), rounding_offset), v_mipmap_level);

                int32_t x[4];
                int32_t y[4];

				_mm_storeu_si128((__m128i*)(x), vx);
				_mm_storeu_si128((__m128i*)(y), vy);

                ret = ret && AddInputBlock(hw, sv, x[0], y[0], bd);
                ret = ret && AddInputBlock(hw, sv, x[1], y[1], bd);
                ret = ret && AddInputBlock(hw, sv, x[2], y[2], bd);
                ret = ret && AddInputBlock(hw, sv, x[3], y[3], bd);
            };

			interpolate_pixels(tx3, ty3, bx3, by3);
            interpolate_pixels(tx2, ty2, bx2, by2);
			interpolate_pixels(tx1, ty1, bx1, by1);
			interpolate_pixels(tx0, ty0, bx0, by0);
        }		

#else
        mm_value_scaler scale_value{static_cast<int32_t>(mipmap_level)};

		// Generate output to input block mapping using bilinear interpolation on the mesh
		for( int32_t sv = 0; sv < 8; ++sv )
		{
			const int32_t fy1 = sv;
			const int32_t fy2 = 8 - fy1;

			for( int32_t sh = 0; sh < 8; ++sh )
			{
				const int32_t fx1 = sh;
				const int32_t fx2 = 8 - fx1;

				const int32_t w[4] = {(fx2 * fy2), (fx1 * fy2), (fx2 * fy1), (fx1 * fy1)};

				// Left half
				{
					const int32_t x = scale_value(r0[0]._x * w[0] + r0[1]._x * w[1] + r1[0]._x * w[2] + r1[1]._x * w[3]) >> 10;
					const int32_t y = scale_value(r0[0]._y * w[0] + r0[1]._y * w[1] + r1[0]._y * w[2] + r1[1]._y * w[3]) >> 10;

					ret = ret && AddInputBlock(hw, sv, x, y, bd);
				}

				// Right half
				{
					const int32_t x = scale_value(r0[1]._x * w[0] + r0[2]._x * w[1] + r1[1]._x * w[2] + r1[2]._x * w[3]) >> 10;
					const int32_t y = scale_value(r0[1]._y * w[0] + r0[2]._y * w[1] + r1[1]._y * w[2] + r1[2]._y * w[3]) >> 10;

					ret = ret && AddInputBlock(hw, sv, x, y, bd);
				}
			}		
		}
#endif /* __SSE__ */			
    }

	// If the block is not empty, check the filter grid constraint
	if(bd._block_list_size)
	{
		const uint32_t block_v_range = bd._max_v - bd._min_v + 1;
		const uint32_t block_h_range = bd._max_h - bd._min_h + 1;

		bool out_of_range = (block_h_range > BLOCK_GRID_WIDTH) || (block_v_range > BLOCK_GRID_HEIGHT);

		const float local_scale_h = scale_h / static_cast<float>(1 << mipmap_level);
		const float local_scale_v = scale_v / static_cast<float>(1 << mipmap_level);		

		bd._block_coef[0] = get_lpf_coef(local_scale_h);
		bd._block_coef[1] = get_lpf_coef(local_scale_v);

		bd._mm_level = mipmap_level;

		ret = ret && (!out_of_range) && CheckCompressionLimit(std::max(local_scale_h, local_scale_v));
	}	

	return ret;
}


// Simple output to input block translation
// finds min and max input coordinates used by a block
// and uses all input blocks within this range
// This sometimes will result in extra input blocks loaded
// into the cache. However these blocks are likely to
// be required for neighbouring output areas anyway.
// This algorithm is significantly faster than interpolation
// and translation of individual pixels of output image.
template<bool MIPMAP_ENABLE>
inline bool OtoI_2x3_Db(const WarpDataContext& ctx, const mesh_node_t* r0, const mesh_node_t* r1, const uint32_t num_streams, const uint32_t frac_prec, block_descriptor_t& bd)
{
    bool ret = true;

    // Bring to the mesh format (N fraction bits)
	const int32_t block_height = static_cast<int32_t>(ctx._hw->_block_height << frac_prec);
	const int32_t block_width = static_cast<int32_t>(ctx._hw->_block_width << frac_prec);
    // Extra pixels for bicubic filter
    const int32_t extra = (2 << frac_prec);

    // Pre-initialize to max and min values
    int32_t block_minh = std::numeric_limits<int32_t>::max();
    int32_t block_maxh = std::numeric_limits<int32_t>::min();
    int32_t block_minv = std::numeric_limits<int32_t>::max();
    int32_t block_maxv = std::numeric_limits<int32_t>::min();

    // Pre-initialize compression factors to zero
    float scale_h = 1.0f;
    float scale_v = 1.0f;

    // Quick check if entire block is outside screen
    for(uint32_t s = 0; s < num_streams; ++s)
    {
        const uint32_t _0 = s;
        const uint32_t _1 = s + num_streams;
        const uint32_t _2 = s + num_streams + num_streams;

        const auto [minh, maxh] = std::minmax({r0[_0]._x, r0[_1]._x, r0[_2]._x, r1[_0]._x, r1[_1]._x, r1[_2]._x});
        const auto [minv, maxv] = std::minmax({r0[_0]._y, r0[_1]._y, r0[_2]._y, r1[_0]._y, r1[_1]._y, r1[_2]._y});

        block_minh = std::min(minh, block_minh);
        block_maxh = std::max(maxh, block_maxh);
        block_minv = std::min(minv, block_minv);
        block_maxv = std::max(maxv, block_maxv);	

        // Estimate local compression
        const auto [scale_h_tmp, scale_v_tmp] = EstimateBlockScale<3>(r0 + s, r1 + s, num_streams, block_width, block_height);
        scale_h = std::max(scale_h, scale_h_tmp);
        scale_v = std::max(scale_v, scale_v_tmp);
    }

    block_minh -= (1 << frac_prec);
    block_minv -= (1 << frac_prec);
    block_maxh += extra;
    block_maxv += extra;

    const int32_t width = ctx._width_in << frac_prec;
    const int32_t height = ctx._height_in << frac_prec;

    // Do not process the block if it is entirely outside the screen
    if ((block_minh >= width) || (block_maxh < 0) || (block_minv >= height) || (block_maxv < 0))
        return ret;            

    uint32_t mipmap_level = 0;

    if constexpr (MIPMAP_ENABLE)
        mipmap_level = GetMipmapLevel(std::max(scale_h, scale_v));

    mm_value_scaler scale_value{static_cast<int32_t>(mipmap_level)};

    // Pre-initialize to max and min values
    block_minh = std::numeric_limits<int32_t>::max();
    block_maxh = std::numeric_limits<int32_t>::min();
    block_minv = std::numeric_limits<int32_t>::max();
    block_maxv = std::numeric_limits<int32_t>::min();            

    for(uint32_t s = 0; s < num_streams; ++s)
    {    
        const uint32_t _0 = s;
        const uint32_t _1 = s + num_streams;
        const uint32_t _2 = s + num_streams + num_streams;

        // Perform bilinear interpolation on the mesh nodes (to mimic RTL)
        // This is important otherwise not all necessary input blocks are fetched into the cache
        const mesh_node_t r0s[] = {
            {scale_value(r0[_0]._x << 6) >> 6, scale_value(r0[_0]._y << 6) >> 6},
            {scale_value(r0[_1]._x << 6) >> 6, scale_value(r0[_1]._y << 6) >> 6},
            {scale_value(r0[_2]._x << 6) >> 6, scale_value(r0[_2]._y << 6) >> 6}
        };

        const mesh_node_t r1s[] = {
            {scale_value(r1[_0]._x << 6) >> 6, scale_value(r1[_0]._y << 6) >> 6},
            {scale_value(r1[_1]._x << 6) >> 6, scale_value(r1[_1]._y << 6) >> 6},
            {scale_value(r1[_2]._x << 6) >> 6, scale_value(r1[_2]._y << 6) >> 6}
        };

        const auto [minh, maxh] = std::minmax({r0s[0]._x, r0s[1]._x, r0s[2]._x, r1s[0]._x, r1s[1]._x, r1s[2]._x});
        const auto [minv, maxv] = std::minmax({r0s[0]._y, r0s[1]._y, r0s[2]._y, r1s[0]._y, r1s[1]._y, r1s[2]._y});

        block_minh = std::min(minh, block_minh);
        block_maxh = std::max(maxh, block_maxh);
        block_minv = std::min(minv, block_minv);
        block_maxv = std::max(maxv, block_maxv);
    }

    block_minh -= (1 << frac_prec);
    block_minv -= (1 << frac_prec);	
    block_maxh += extra;
    block_maxv += extra;

    bd._min_h = block_minh / block_width - (block_minh < 0 ? 1 : 0);
    bd._max_h = block_maxh / block_width + 1;
    bd._min_v = block_minv / block_height - (block_minv < 0 ? 1 : 0);
    bd._max_v = block_maxv / block_height + 1;			

    const uint32_t block_v_range = bd._max_v - bd._min_v;
    const uint32_t block_h_range = bd._max_h - bd._min_h;

    bool out_of_range = ((block_h_range > BLOCK_GRID_WIDTH) || (block_v_range > BLOCK_GRID_HEIGHT));

    const float local_scale_h = scale_h / static_cast<float>(1 << mipmap_level);
    const float local_scale_v = scale_v / static_cast<float>(1 << mipmap_level);

    bd._block_list_size = block_v_range * block_h_range;
    bd._block_coef[0] = get_lpf_coef(local_scale_h);
    bd._block_coef[1] = get_lpf_coef(local_scale_v);	
    bd._mm_level = mipmap_level;			

    ret = (!out_of_range && CheckCompressionLimit(std::max(local_scale_h, local_scale_v)));

    return ret;
}


template<bool MIPMAP_ENABLE>
inline bool OtoI_2x2_Sb(const WarpDataContext& ctx, const mesh_node_t* r0, const mesh_node_t* r1, const uint32_t num_streams, const uint32_t frac_prec, block_descriptor_t& bd, bool top_half)
{
	bool ret = true;

	// Bring to the mesh format (N fraction bits)
	const int32_t block_width = static_cast<int32_t>(ctx._hw->_block_width << frac_prec);
    // Extra pixels for bicubic filter
    const int32_t extra = 2 << frac_prec;
	const int32_t width = ctx._width_in << frac_prec;
    const int32_t height = ctx._height_in << frac_prec;

	const WarpHwContext* const hw = ctx._hw.get();

    //Pre-initialize compression factors to zero
    float scale_h = 1.0f;
    float scale_v = 1.0f;
	uint32_t mipmap_level = 0;

    const int32_t v_offset = top_half ? 0 : 8;

    // Pre-initialize block's min and max values
    bd._min_v = bd._min_h = std::numeric_limits<int16_t>::max();
	bd._max_v = bd._max_h = std::numeric_limits<int16_t>::min();

    // Quick check if entire mesh section is outside screen
    for(uint32_t s = 0; s < num_streams; ++s)
    {
        const uint32_t _0 = s;
        const uint32_t _1 = s + num_streams;

        const auto [minh, maxh] = std::minmax({r0[_0]._x, r0[_1]._x, r1[_0]._x, r1[_1]._x});
        const auto [minv, maxv] = std::minmax({r0[_0]._y, r0[_1]._y, r1[_0]._y, r1[_1]._y});

		const int32_t block_minh = minh - (1 << frac_prec);
		const int32_t block_minv = minv - (1 << frac_prec);
		const int32_t block_maxh = maxh + extra;
		const int32_t block_maxv = maxv + extra;

		// Do not process if the mesh section is entirely outside the screen
		if((block_minh >= width) || (block_maxh < 0) || (block_minv >= height) || (block_maxv < 0))
			continue;		

        // Estimate local compression
		// Instead of block_width really need mesh step here
		const auto [scale_h_tmp, scale_v_tmp] = EstimateBlockScale<2>(r0 + s, r1 + s, num_streams, block_width, block_width);
        scale_h = std::max(scale_h, scale_h_tmp);
        scale_v = std::max(scale_v, scale_v_tmp);

        if constexpr (MIPMAP_ENABLE)
            mipmap_level = GetMipmapLevel(std::max(scale_h_tmp, scale_v_tmp));

#if defined (__ARM_NEON) && ! defined (DONT_USE_SIMD)

        // Linearly interpolated top X,Y
        int32x4_t tx0, tx1, tx2, tx3;
        int32x4_t ty0, ty1, ty2, ty3;

        // Linearly interpolated bottom X,Y
        int32x4_t bx0, bx1, bx2, bx3;
        int32x4_t by0, by1, by2, by3;

        // Top 2x points
        const int32x4_t x0 = vdupq_n_s32(r0[_0]._x);
        const int32x4_t x1 = vdupq_n_s32(r0[_1]._x);
        const int32x4_t y0 = vdupq_n_s32(r0[_0]._y);
        const int32x4_t y1 = vdupq_n_s32(r0[_1]._y);

        // Bottom 2x points
        const int32x4_t x2 = vdupq_n_s32(r1[_0]._x);
        const int32x4_t x3 = vdupq_n_s32(r1[_1]._x);
        const int32x4_t y2 = vdupq_n_s32(r1[_0]._y);
        const int32x4_t y3 = vdupq_n_s32(r1[_1]._y);

        const  int32x4_t v16 = vdupq_n_s32(16);

        auto interpolate_top_bottom_row = [&](const int32x4_t& k, int32x4_t& tx, int32x4_t& ty, int32x4_t& bx, int32x4_t& by){
            int32x4_t kr = vsubq_s32(v16, k);

            // Left top and bottom
            const int32x4_t px0 = vmulq_s32(x0, kr);
            const int32x4_t py0 = vmulq_s32(y0, kr);
            const int32x4_t px2 = vmulq_s32(x2, kr);
            const int32x4_t py2 = vmulq_s32(y2, kr);

            // Right top and bottom
            const int32x4_t px1 = vmulq_s32(x1, k);
            const int32x4_t py1 = vmulq_s32(y1, k);
            const int32x4_t px3 = vmulq_s32(x3, k);
            const int32x4_t py3 = vmulq_s32(y3, k);

            tx = vaddq_s32(px0, px1);
            ty = vaddq_s32(py0, py1);
            bx = vaddq_s32(px2, px3);
            by = vaddq_s32(py2, py3);
        };

        interpolate_top_bottom_row(int32x4_t{ 0, 1, 2, 3}, tx0, ty0, bx0, by0);
        interpolate_top_bottom_row(int32x4_t{ 4, 5, 6, 7}, tx1, ty1, bx1, by1);
        interpolate_top_bottom_row(int32x4_t{ 8, 9,10,11}, tx2, ty2, bx2, by2);
        interpolate_top_bottom_row(int32x4_t{12,13,14,15}, tx3, ty3, bx3, by3);

        const int32x4_t rounding_offset = vdupq_n_s32((1 << mipmap_level) >> 1);

		for( int32_t sv = 0 + v_offset; sv < (8 + v_offset); ++sv )
		{
            const uint32_t osv = sv - v_offset;

            const int32x4_t k = vdupq_n_s32(sv);
            const int32x4_t kr = vsubq_s32(v16, k);

            auto interpolate_blocks = [&](const int32x4_t& tx, const int32x4_t& ty, const int32x4_t& bx, const int32x4_t& by){

                const int32x4_t ptx = vmulq_s32(tx, kr);
                const int32x4_t pty = vmulq_s32(ty, kr);
                const int32x4_t pbx = vmulq_s32(bx, k);
                const int32x4_t pby = vmulq_s32(by, k);

                const int32x4_t vx = vshrq_n_s32(vaddq_s32(vshrq_n_s32(vaddq_s32(ptx, pbx), 11), rounding_offset), mipmap_level);
                const int32x4_t vy = vshrq_n_s32(vaddq_s32(vshrq_n_s32(vaddq_s32(pty, pby), 11), rounding_offset), mipmap_level);

                int32_t x[4];
                int32_t y[4];

                vst1q_s32(x, vx);
                vst1q_s32(y, vy);

                ret = ret && AddInputBlock(hw, osv, x[0], y[0], bd);
                ret = ret && AddInputBlock(hw, osv, x[1], y[1], bd);
                ret = ret && AddInputBlock(hw, osv, x[2], y[2], bd);
                ret = ret && AddInputBlock(hw, osv, x[3], y[3], bd);
            };

            interpolate_blocks(tx0, ty0, bx0, by0);
            interpolate_blocks(tx1, ty1, bx1, by1);
            interpolate_blocks(tx2, ty2, bx2, by2);
            interpolate_blocks(tx3, ty3, bx3, by3);         
        }
#elif defined (__SSE__) && !defined (DONT_USE_SIMD)
        
		// Linearly interpolated top X,Y
        __m128i tx0, tx1, tx2, tx3;
        __m128i ty0, ty1, ty2, ty3;

        // Linearly interpolated bottom X,Y
        __m128i bx0, bx1, bx2, bx3;
        __m128i by0, by1, by2, by3;

		// Load top 2x points
		const __m128i x0 = _mm_set1_epi32(r0[_0]._x);
		const __m128i y0 = _mm_set1_epi32(r0[_0]._y);
		const __m128i x1 = _mm_set1_epi32(r0[_1]._x);
		const __m128i y1 = _mm_set1_epi32(r0[_1]._y);

		// Load bottom 2x points
		const __m128i x2 = _mm_set1_epi32(r1[_0]._x);
		const __m128i y2 = _mm_set1_epi32(r1[_0]._y);
		const __m128i x3 = _mm_set1_epi32(r1[_1]._x);
		const __m128i y3 = _mm_set1_epi32(r1[_1]._y);

		const __m128i v16 = _mm_set1_epi32(16);

		auto interpolate_top_bottom = [&](const __m128i& k, __m128i& tx,  __m128i& ty,  __m128i& bx,  __m128i& by){
            const __m128i kr = _mm_sub_epi32(v16, k);

            // Left top and bottom
            const __m128i px0 = _mm_mullo_epi32(x0, kr);
            const __m128i py0 = _mm_mullo_epi32(y0, kr);
            const __m128i px2 = _mm_mullo_epi32(x2, kr);
            const __m128i py2 = _mm_mullo_epi32(y2, kr);

            // Right top and bottom
            const __m128i px1 = _mm_mullo_epi32(x1, k);
            const __m128i py1 = _mm_mullo_epi32(y1, k);
            const __m128i px3 = _mm_mullo_epi32(x3, k);
            const __m128i py3 = _mm_mullo_epi32(y3, k);

            tx = _mm_add_epi32(px0, px1);
            ty = _mm_add_epi32(py0, py1);
            bx = _mm_add_epi32(px2, px3);
            by = _mm_add_epi32(py2, py3);
		};

		interpolate_top_bottom(_mm_set_epi32( 0 , 1,  2,  3), tx0, ty0, bx0, by0);
		interpolate_top_bottom(_mm_set_epi32( 4 , 5,  6,  7), tx1, ty1, bx1, by1);
		interpolate_top_bottom(_mm_set_epi32( 8  ,9, 10, 11), tx2, ty2, bx2, by2);
		interpolate_top_bottom(_mm_set_epi32(12 ,13, 14, 15), tx3, ty3, bx3, by3);

		const __m128i rounding_offset = _mm_set1_epi32((1 << mipmap_level) >> 1);
		const __m128i v_mipmap_level = _mm_set_epi32(0, 0, 0, mipmap_level);

		for( int32_t sv = 0 + v_offset; sv < (8 + v_offset); ++sv )
		{
            const uint32_t osv = sv - v_offset;

            const __m128i k = _mm_set1_epi32(sv);
            const __m128i kr = _mm_sub_epi32(v16, k);

            auto interpolate_pixels = [&](const __m128i& tx, const __m128i& ty, const __m128i& bx, const __m128i& by){

                const __m128i ptx = _mm_mullo_epi32(tx, kr);
                const __m128i pty = _mm_mullo_epi32(ty, kr);
                const __m128i pbx = _mm_mullo_epi32(bx, k);
                const __m128i pby = _mm_mullo_epi32(by, k);

                const __m128i vx = _mm_sra_epi32(_mm_add_epi32(_mm_srai_epi32(_mm_add_epi32(ptx, pbx), 11), rounding_offset), v_mipmap_level);
                const __m128i vy = _mm_sra_epi32(_mm_add_epi32(_mm_srai_epi32(_mm_add_epi32(pty, pby), 11), rounding_offset), v_mipmap_level);

                int32_t x[4];
                int32_t y[4];

				_mm_storeu_si128((__m128i*)(x), vx);
				_mm_storeu_si128((__m128i*)(y), vy);

                ret = ret && AddInputBlock(hw, osv, x[0], y[0], bd);
                ret = ret && AddInputBlock(hw, osv, x[1], y[1], bd);
                ret = ret && AddInputBlock(hw, osv, x[2], y[2], bd);
                ret = ret && AddInputBlock(hw, osv, x[3], y[3], bd);
            };

            interpolate_pixels(tx0, ty0, bx0, by0);
            interpolate_pixels(tx1, ty1, bx1, by1);
            interpolate_pixels(tx2, ty2, bx2, by2);
            interpolate_pixels(tx3, ty3, bx3, by3);         
        }        
#elif defined (__AVX2__) && !defined (DONT_USE_SIMD)
		// Linearly interpolated top X,Y
        __m256i tx0, tx1, ty0, ty1;

        // Linearly interpolated bottom X,Y
        __m256i bx0, bx1, by0, by1;

		// Load top 2x points
		const __m256i x0 = _mm256_set1_epi32(r0[_0]._x);
		const __m256i x1 = _mm256_set1_epi32(r0[_1]._x);
		const __m256i y0 = _mm256_set1_epi32(r0[_0]._y);
		const __m256i y1 = _mm256_set1_epi32(r0[_1]._y);

		// Load bottom 2x points
		const __m256i x2 = _mm256_set1_epi32(r1[_0]._x);
		const __m256i x3 = _mm256_set1_epi32(r1[_1]._x);
		const __m256i y2 = _mm256_set1_epi32(r1[_0]._y);
		const __m256i y3 = _mm256_set1_epi32(r1[_1]._y);

		const __m256i v16 = _mm256_set1_epi32(16);

		auto interpolate_top_bottom = [&](const __m256i& k, __m256i& tx,  __m256i& ty,  __m256i& bx,  __m256i& by){
            const __m256i kr = _mm256_sub_epi32(v16, k);

            // Left top and bottom
            const __m256i px0 = _mm256_mullo_epi32(x0, kr);
            const __m256i py0 = _mm256_mullo_epi32(y0, kr);
            const __m256i px2 = _mm256_mullo_epi32(x2, kr);
            const __m256i py2 = _mm256_mullo_epi32(y2, kr);

            // Right top and bottom
            const __m256i px1 = _mm256_mullo_epi32(x1, k);
            const __m256i py1 = _mm256_mullo_epi32(y1, k);
            const __m256i px3 = _mm256_mullo_epi32(x3, k);
            const __m256i py3 = _mm256_mullo_epi32(y3, k);

            tx = _mm256_add_epi32(px0, px1);
            ty = _mm256_add_epi32(py0, py1);
            bx = _mm256_add_epi32(px2, px3);
            by = _mm256_add_epi32(py2, py3);
		};

		interpolate_top_bottom(_mm256_set_epi32(0 ,1,  2,  3,  4,  5,  6,  7), tx0, ty0, bx0, by0);
		interpolate_top_bottom(_mm256_set_epi32(8 ,9, 10, 11, 12, 13, 14, 15), tx1, ty1, bx1, by1);

		const __m256i rounding_offset = _mm256_set1_epi32((1 << mipmap_level) >> 1);
		const __m128i v_mipmap_level = _mm_set_epi32(0, 0, 0, mipmap_level);

		for( int32_t sv = 0 + v_offset; sv < (8 + v_offset); ++sv )
		{
            const uint32_t osv = sv - v_offset;

            const __m256i k = _mm256_set1_epi32(sv);
            const __m256i kr = _mm256_sub_epi32(v16, k);

            auto interpolate_pixels = [&](const __m256i& tx, const __m256i& ty, const __m256i& bx, const __m256i& by){

                const __m256i ptx = _mm256_mullo_epi32(tx, kr);
                const __m256i pty = _mm256_mullo_epi32(ty, kr);
                const __m256i pbx = _mm256_mullo_epi32(bx, k);
                const __m256i pby = _mm256_mullo_epi32(by, k);

                const __m256i vx = _mm256_sra_epi32(_mm256_add_epi32(_mm256_srai_epi32(_mm256_add_epi32(ptx, pbx), 11), rounding_offset), v_mipmap_level);
                const __m256i vy = _mm256_sra_epi32(_mm256_add_epi32(_mm256_srai_epi32(_mm256_add_epi32(pty, pby), 11), rounding_offset), v_mipmap_level);

                int32_t x[8];
                int32_t y[8];

				_mm256_storeu_si256((__m256i*)(x), vx);
				_mm256_storeu_si256((__m256i*)(y), vy);

                ret = ret && AddInputBlock(hw, osv, x[0], y[0], bd);
                ret = ret && AddInputBlock(hw, osv, x[1], y[1], bd);
                ret = ret && AddInputBlock(hw, osv, x[2], y[2], bd);
                ret = ret && AddInputBlock(hw, osv, x[3], y[3], bd);
				ret = ret && AddInputBlock(hw, osv, x[4], y[4], bd);
				ret = ret && AddInputBlock(hw, osv, x[5], y[5], bd);
				ret = ret && AddInputBlock(hw, osv, x[6], y[6], bd);
				ret = ret && AddInputBlock(hw, osv, x[7], y[7], bd);
            };

            interpolate_pixels(tx0, ty0, bx0, by0);
            interpolate_pixels(tx1, ty1, bx1, by1);
        }
#else
        mm_value_scaler scale_value{static_cast<int32_t>(mipmap_level)};

		// Generate output to input block mapping using bilinear interpolation on the mesh
		for( int32_t sv = 0 + v_offset; sv < (8 + v_offset); ++sv )
		{
			const int32_t fy1 = sv;
			const int32_t fy2 = 16 - fy1;
            const uint32_t osv = sv - v_offset;

			for( int32_t sh = 0; sh < 16; ++sh )
			{
				const int32_t fx1 = sh;
				const int32_t fx2 = 16 - fx1;

				const int32_t w[4] = {(fx2 * fy2), (fx1 * fy2), (fx2 * fy1), (fx1 * fy1)};

                const int32_t x = scale_value(r0[_0]._x * w[0] + r0[_1]._x * w[1] + r1[_0]._x * w[2] + r1[_1]._x * w[3]) >> 11;
                const int32_t y = scale_value(r0[_0]._y * w[0] + r0[_1]._y * w[1] + r1[_0]._y * w[2] + r1[_1]._y * w[3]) >> 11;

                ret = ret && AddInputBlock(hw, osv, x, y, bd);
			}		
		}
#endif /* __ARM_NEON__ || SSE */
    }

	// If the block is not empty, check the filter grid constraint
	if(bd._block_list_size)
	{
		const uint32_t block_v_range = bd._max_v - bd._min_v + 1;
		const uint32_t block_h_range = bd._max_h - bd._min_h + 1;

		bool out_of_range = (block_h_range > BLOCK_GRID_WIDTH) || (block_v_range > BLOCK_GRID_HEIGHT);

		const float local_scale_h = scale_h / static_cast<float>(1 << mipmap_level);
		const float local_scale_v = scale_v / static_cast<float>(1 << mipmap_level);		

		bd._block_coef[0] = get_lpf_coef(local_scale_h);
		bd._block_coef[1] = get_lpf_coef(local_scale_v);

		bd._mm_level = mipmap_level;

		ret = ret && (!out_of_range) && CheckCompressionLimit(std::max(local_scale_h, local_scale_v));
	}	

	return ret;
}


// Output to input block mapping for blocks defined by 2x2 mesh points
template<bool MIPMAP_ENABLE>
inline bool OtoI_2x2_Db(const WarpDataContext& ctx, const mesh_node_t* _r0, const mesh_node_t* _r1, const uint32_t num_streams, const uint32_t frac_prec, block_descriptor_t& bd, bool top_half)
{
    bool ret = true;

	// Bring to the mesh format (4 fraction bits)
	const int32_t block_height = static_cast<int32_t>(ctx._hw->_block_height << frac_prec);
	const int32_t block_width = static_cast<int32_t>(ctx._hw->_block_width << frac_prec);
	// Extra pixels for bicubic filter
    const int32_t extra = 2 << frac_prec;

    // Pre-initialize to max and min values
    int32_t block_minh = std::numeric_limits<int32_t>::max();
    int32_t block_maxh = std::numeric_limits<int32_t>::min();
    int32_t block_minv = std::numeric_limits<int32_t>::max();
    int32_t block_maxv = std::numeric_limits<int32_t>::min();

    // Pre-initialize compression factors to zero
    float scale_h = 1.0f;
    float scale_v = 1.0f;

    static constexpr uint32_t MESH_MAX_STREAMS = 3;
    mesh_node_t r01[2 * MESH_MAX_STREAMS];

    //Quick check if entire block is outside screen
	for(uint32_t s = 0; s < num_streams; ++s)
    {
		const uint32_t _0 = s;
		const uint32_t _1 = s + num_streams;

        r01[_0] = {(_r0[_0]._x + _r1[_0]._x) >> 1, (_r0[_0]._y + _r1[_0]._y) >> 1};
        r01[_1] = {(_r0[_1]._x + _r1[_1]._x) >> 1, (_r0[_1]._y + _r1[_1]._y) >> 1};

        const mesh_node_t* r0 = top_half ? _r0 : r01;
        const mesh_node_t* r1 = top_half ? r01 : _r1;

        const auto [minh, maxh] = std::minmax({r0[_0]._x, r0[_1]._x, r1[_0]._x, r1[_1]._x});
        const auto [minv, maxv] = std::minmax({r0[_0]._y, r0[_1]._y, r1[_0]._y, r1[_1]._y});

        block_minh = std::min(minh, block_minh);
        block_maxh = std::max(maxh, block_maxh);
        block_minv = std::min(minv, block_minv);
        block_maxv = std::max(maxv, block_maxv);		

        // Estimate local compression
        const auto [scale_h_tmp, scale_v_tmp] = EstimateBlockScale<2>(r0+s, r1+s, num_streams, block_width, block_height);
        scale_h = std::max(scale_h, scale_h_tmp);
        scale_v = std::max(scale_v, scale_v_tmp);
    }

    block_minh -= (1 << frac_prec);
    block_minv -= (1 << frac_prec);
    block_maxh += extra;
    block_maxv += extra;

    const int32_t width = ctx._width_in << frac_prec;
    const int32_t height = ctx._height_in << frac_prec;            

    // Do not process the block if it is entirely outside the screen
    if ((block_minh >= width) || (block_maxh < 0) || (block_minv >= height) || (block_maxv < 0))
        return ret;

    uint32_t mipmap_level = 0;

    if constexpr (MIPMAP_ENABLE)
        mipmap_level = GetMipmapLevel(std::max(scale_h, scale_v));

    mm_value_scaler scale_value{static_cast<int32_t>(mipmap_level)};

    // Pre-initialize to max and min values
    block_minh = std::numeric_limits<int32_t>::max();
    block_maxh = std::numeric_limits<int32_t>::min();
    block_minv = std::numeric_limits<int32_t>::max();
    block_maxv = std::numeric_limits<int32_t>::min();

	for(uint32_t s = 0; s < num_streams; ++s)
    {
		const uint32_t _0 = s;
		const uint32_t _1 = s + num_streams;

        const mesh_node_t* r0 = top_half ? _r0 : r01;
        const mesh_node_t* r1 = top_half ? r01 : _r1;

		// Perform bilinear interpolation on the mesh nodes (to mimic RTL)
		// This is important otherwise not all necessary input blocks are fetched into the cache
		const mesh_node_t r0s[] = {
			{scale_value(r0[_0]._x << 6) >> 6, scale_value(r0[_0]._y << 6) >> 6},
			{scale_value(r0[_1]._x << 6) >> 6, scale_value(r0[_1]._y << 6) >> 6}
		};

		const mesh_node_t r1s[] = {
			{scale_value(r1[_0]._x << 6) >> 6, scale_value(r1[_0]._y << 6) >> 6},
			{scale_value(r1[_1]._x << 6) >> 6, scale_value(r1[_1]._y << 6) >> 6}
		};

		const auto [minh, maxh] = std::minmax({r0s[0]._x, r0s[1]._x, r1s[0]._x, r1s[1]._x});
		const auto [minv, maxv] = std::minmax({r0s[0]._y, r0s[1]._y, r1s[0]._y, r1s[1]._y});

        block_minh = std::min(minh, block_minh);
        block_maxh = std::max(maxh, block_maxh);
        block_minv = std::min(minv, block_minv);
        block_maxv = std::max(maxv, block_maxv);
	}

    block_minh -= (1 << frac_prec);
    block_minv -= (1 << frac_prec);	
    block_maxh += extra;
    block_maxv += extra;	

    bd._min_h = block_minh / block_width - (block_minh < 0 ? 1 : 0);
    bd._max_h = block_maxh / block_width + 1;
    bd._min_v = block_minv / block_height - (block_minv < 0 ? 1 : 0);
    bd._max_v = block_maxv / block_height + 1;

    const uint32_t block_v_range = bd._max_v - bd._min_v;
    const uint32_t block_h_range = bd._max_h - bd._min_h;

    bool out_of_range = ((block_h_range > BLOCK_GRID_WIDTH) || (block_v_range > BLOCK_GRID_HEIGHT));
    
    const float local_scale_h = scale_h / static_cast<float>(1 << mipmap_level);
    const float local_scale_v = scale_v / static_cast<float>(1 << mipmap_level);

    bd._block_list_size = block_v_range * block_h_range;
    bd._block_coef[0] = get_lpf_coef(local_scale_h);
    bd._block_coef[1] = get_lpf_coef(local_scale_v);	
    bd._mm_level = mipmap_level;

    ret = (!out_of_range && CheckCompressionLimit(std::max(local_scale_h, local_scale_v)));

    return ret;
}

} // namespace intel_vvp_warp