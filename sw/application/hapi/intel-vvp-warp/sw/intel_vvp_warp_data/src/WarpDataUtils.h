/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef _WARPDATAUTILS_H_
#define _WARPDATAUTILS_H_

#include <cmath>
#include <cstdio>
#include <inttypes.h>
#include <vector>
#include <algorithm>


namespace intel_vvp_warp
{

inline uint32_t roundup(uint32_t v, uint32_t m)
{
	uint32_t ret_v = v;

	if (m)
	{
		uint32_t rm = v % m;
		ret_v = rm ? (v + m - rm) : ret_v;
	}

	return ret_v;
}

inline uint32_t roundup_pwr_two(uint32_t v, uint32_t a)
{
	return (v + (a - 1)) & ~(a - 1);
}

inline uint32_t get_next_pwr_two(uint32_t v)
{
	v--;
	v |= uint32_t{v >> 1};
	v |= uint32_t{v >> 2};
	v |= uint32_t{v >> 4};
	v |= uint32_t{v >> 8};
	v |= uint32_t{v >> 16};	
	v++;

	return v;
}

inline uint32_t get_prev_pwr_two(uint32_t v)
{
	v = v | (v >> 1);
	v = v | (v >> 2);
	v = v | (v >> 4);
	v = v | (v >> 8);
	v = v | (v >> 16);
	return v - (v >> 1);
};

inline uint32_t width_alignment(bool megablock, uint32_t block_width)
{
	return ((block_width) * (1 + (7 * (megablock ? 1 : 0))));
}

inline uint32_t height_alignment(bool megablock, uint32_t block_height)
{
	return ((block_height) * (1 + (7 * (megablock ? 1 : 0))));
}

inline std::pair<uint32_t, uint32_t> engine_width(uint32_t width, uint32_t engines, uint32_t engine_processing_step)
{
	const uint32_t total_steps = (width + engine_processing_step - 1) / engine_processing_step;
	const uint32_t mid_point = (total_steps + 1) / 2;

	// 0 - even 1 - odd engine
	uint32_t engine_steps[2] = {0};

	switch(engines)
	{
		case 4:
			engine_steps[0] = mid_point / 2;
			engine_steps[1] = mid_point - engine_steps[0];
		break;
		case 2:
			engine_steps[0] = mid_point;
			engine_steps[1] = total_steps - mid_point;
		break;
		case 1:
			engine_steps[0] = total_steps;
			engine_steps[1] = 0;
		break;
		default:
		break;
	};

	return std::make_pair<uint32_t, uint32_t>(engine_steps[0] * engine_processing_step, engine_steps[1] * engine_processing_step);
}

inline uint32_t engine_mesh_stride(const uint32_t engine_width, const uint32_t mesh_step)
{
    // RTL expects this value to be a multiple of 4 
    return roundup_pwr_two((engine_width / mesh_step + 1), 4);
}

class LpfBins
{
public:
    LpfBins(const uint32_t num_bins, const float scale = 2.0f){
        const float b = powf(scale, (1.0f/static_cast<float>(num_bins)));

        for(uint32_t i = 0; i < num_bins; ++i){
            const float v = powf(b, static_cast<float>(i));
            _bins.push_back(static_cast<uint32_t>(v * _MX));
        }
    }

    uint32_t Get(const uint32_t scale) const {
        uint32_t bin = 0;

        for(uint32_t i = 1; i < _bins.size(); ++i){
            if(scale > _bins[i])
                ++bin;
			else
				break;
        }

        return bin;        
    }

    uint32_t Get(const float scale) const {
        return Get(static_cast<uint32_t>(scale * _MX));
    }    

    void Print() const {
        for(uint32_t i = 0; i < _bins.size(); ++i){
            printf( "%" PRIu32 ": %" PRIu32 " %f\n", i, _bins[i], static_cast<float>(_bins[i]) /_MX );
        }
    }

private:
    static constexpr uint32_t _FRAC_PREC = 20;
    static constexpr float _MX = static_cast<float>(1 << _FRAC_PREC);
    std::vector<uint32_t> _bins;
};

inline uint8_t get_lpf_coef(const uint32_t f)
{
    static const LpfBins lpf_bins{8, 2.0f};

	return lpf_bins.Get(f);
}

inline uint8_t get_lpf_coef(const float f)
{
    static const LpfBins lpf_bins{8, 2.0f};

	return lpf_bins.Get(f);
}

inline uint32_t map_address(const uint32_t old_address, const int32_t memory_model)
{
#ifdef INTEL_VVP_WARP_DATA_MEMORY_MAPPING	
	uint32_t new_address = old_address;

	if(memory_model != 1)
	{
		new_address  = ((old_address & 0x000001FF) << 0); //0x0001FF
		new_address += ((old_address & 0x00000600) << 4); //0x006000
		new_address += ((old_address & 0x00007800) >> 2); //0x001E00
		new_address += ((old_address & 0x00018000) << 1); //0x030000
		new_address += ((old_address & 0x00020000) >> 2); //0x008000
		new_address += ((old_address & 0xFFFC0000) << 0); //0x1C0000
	}

	return new_address;
#else
	(void)(memory_model);
	return old_address;
#endif /* INTEL_VVP_WARP_DATA_MEMORY_MAPPING */	
}


// This is how RTL scales down values based on the mipmap level
template<typename T>
struct mm_value_scaler
{
	mm_value_scaler(const T mm_level):
		_mm_level{mm_level},
		_rounding_offset{(1 << mm_level) >> 1}
	{}

	inline T operator()(const T& v) const {
		return (v + (_rounding_offset)) >> _mm_level;
	}

private:
	T _mm_level;
	T _rounding_offset;
};


inline int32_t clamp_mesh_value(const int32_t& v){
	return std::clamp<int32_t>(v, -2048, 63487);
}


// Performs log2(scale) to get the mipmap level
// The scale must be a power of 2 in the range [1..128]
inline uint32_t mm_scale_to_level(const uint32_t scale)
{
#ifdef __GNUC__    
    return __builtin_ffs(scale) - 1;
#else
    uint32_t level = 0;

    switch(scale){
        case 128:
            ++level;		
        case 64:
            ++level;		
        case 32:
            ++level;		
        case 16:
            ++level;
        case 8:
            ++level;
        case 4:
            ++level;
        case 2:
            ++level;
        default:
            break;
    }

    return level;
#endif /* __GNUC__ */    
}


enum class WarpMemMap;
uint32_t max_frame_dim(const WarpMemMap& mem_map);

} // intel_vvp_warp

#endif /* _WARPDATAUTILS_H_ */
