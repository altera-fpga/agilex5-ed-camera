/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#ifndef __WARPDATACONTEXT_H__
#define __WARPDATACONTEXT_H__


#include <stdint.h>
#include <memory>
#include "WarpDataUtils.h"


namespace intel_vvp_warp
{

// Note: the enums below along with the struct WarpHwContext
// introduced to decouple this library from the omni_warp driver
// where similar types are defined

enum class WarpFilterType
{
	EBILINEAR,
	EBICUBIC
};


enum class WarpMemMap
{
	ESDTV,
	EHDTV,
	EUHDTV,
	E8KUHDTV
};


struct WarpHwContext
{
	static constexpr uint32_t _MIPMAP_LEVEL_MAX = 7;
	uint32_t _engines;
	uint32_t _streams;
	uint32_t _block_width;
	uint32_t _block_height;
	uint32_t _block_cache_size;		// Block cache size in the current design
	uint32_t _block_cache_size_max;	// Maximum supported block cache size
	uint32_t _mem_model;
	bool _megablock;
	bool _output_skip;
	WarpMemMap _mem_map;
	WarpFilterType _pixel_filter;
	bool _output_block_scan;
	bool _mipmap_enable;
};


using WarpHwContextPtr = std::shared_ptr<WarpHwContext>;


struct WarpDataContext
{
	WarpDataContext(const WarpHwContextPtr hw, uint32_t wi, uint32_t hi, uint32_t wo, uint32_t ho) :
		_hw{ hw },
		_width_in{ wi }, _height_in{ hi }, _width_out{ wo }, _height_out{ ho },
		_hblocks_in{ 0 },
		_vblocks_in{ 0 },
		_hblocks_out{ (_width_out + (hw->_block_width - 1)) / hw->_block_width },
		_vblocks_out{ (_height_out + (hw->_block_height - 1)) / hw->_block_height },
		_skip_blank{ true },
		_allow_mesh_errors{ true }
	{
		const auto [width_even, width_odd] = engine_width(wo, hw->_engines, hw->_block_width * hw->_block_height);

		_engine_width[0] = width_even;
		_engine_width[1] = width_odd;		

		_engine_hblocks[0] = _engine_width[0] / hw->_block_width;
		_engine_hblocks[1] = _engine_width[1] / hw->_block_width;

		_hblocks_in[0] = (_width_in + (hw->_block_width - 1)) / hw->_block_width;
		_vblocks_in[0] = (_height_in + (hw->_block_height - 1)) / hw->_block_height;		

		// Calculate mipmap sizes in blocks
		for(uint32_t mm_level = 1; mm_level < (WarpHwContext::_MIPMAP_LEVEL_MAX + 1); ++mm_level)
		{
			const uint32_t mm_scale = (1 << mm_level);
			const uint32_t width_in_mm = (_width_in + mm_scale - 1) / mm_scale;
			const uint32_t height_in_mm = (_height_in + mm_scale - 1) / mm_scale;
			_hblocks_in[mm_level] = (width_in_mm + (hw->_block_width - 1)) / hw->_block_width;;
			_vblocks_in[mm_level] = (height_in_mm + (hw->_block_height - 1)) / hw->_block_height;
		}
	}

	const WarpHwContextPtr _hw;

	uint32_t _width_in;
	uint32_t _height_in;	
	uint32_t _width_out;
	uint32_t _height_out;

	// 0 - Even engines 1 - Odd engines
	uint32_t _engine_width[2];
	uint32_t _engine_hblocks[2];

	uint32_t _hblocks_in[WarpHwContext::_MIPMAP_LEVEL_MAX + 1];
	uint32_t _vblocks_in[WarpHwContext::_MIPMAP_LEVEL_MAX + 1];
	uint32_t _hblocks_out;
	uint32_t _vblocks_out;

	bool _skip_blank;
	bool _allow_mesh_errors;
};

} //namespace intel_vvp_warp

#endif /*__WARPDATACONTEXT_H__*/