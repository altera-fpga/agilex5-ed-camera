/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WarpEngineState.h"
#include "WarpDataGenerator.h"
#include <algorithm>
#include <cassert>

namespace intel_vvp_warp
{

WarpEngineState::WarpEngineState(const WarpHwContextPtr hw):
	_used_count(hw->_block_cache_size, 0),
	_fetch_tracker{nullptr},
    _wait_run_min{std::numeric_limits<uint32_t>::max()}
{
	// Check the chosen integral type for cache position is big enough
	assert(hw->_block_cache_size <= std::numeric_limits<fetch_pos_t>::max());

	const uint32_t block_cache_na = hw->_block_cache_size_max - 1;
	const uint32_t block_cache_size = (hw->_block_cache_size < block_cache_na ? hw->_block_cache_size : block_cache_na - 1);

	_fetch_tracker = std::make_shared<FetchPosTracker>(block_cache_size);
	Reset();
}


void WarpEngineState::Reset()
{
	_v_start = 0;
	_v_end = 0;
	_h_start = 0;
	_h_end = 0;

	_v_process_block	= 0;
	_h_process_block	= 0;
	_run_until			= 0;
	_run_until_prev		= 0;
	_wait_until			= 0;
	_wait_until_prev	= 0;
	_fetch_rom_address	= 0;

	std::fill_n(_used_count.begin(), _used_count.size(), 0);

	_fetch_tracker->Reset();

	_wait_run_min = std::numeric_limits<uint32_t>::max();
	
	_fc.Reset();

    _mm_level_max = 0;

#ifdef DEBUG_BLOCK_LOAD
	std::fill_n(&(_load_count[0][0]), sizeof(_load_count), 0);
#endif /*DEBUG_BLOCK_LOAD*/
}


////////////////////////////////////////


FetchPosTracker::FetchPosTracker(const fetch_pos_t FETCH_BLOCKS):
	_FETCH_BLOCKS{FETCH_BLOCKS},
	_rpos{},
	_pos{0}
{
	_rpos.reserve(_FETCH_BLOCKS);
}


FetchPosTracker::~FetchPosTracker()
{
}


void FetchPosTracker::Reset()
{
	_pos = 0;
	_rpos.clear();
}


fetch_pos_t FetchPosTracker::Get()
{
	fetch_pos_t ret = _FETCH_BLOCKS;

	if(_rpos.empty())
	{
		ret = _pos;
		if(_pos < _FETCH_BLOCKS) ++_pos;
	}
	else
	{
		ret = _rpos.back();
		_rpos.pop_back();
	}

	return ret;
}


void FetchPosTracker::Release(fetch_pos_t pos)
{
	_rpos.push_back(pos);
}

} // namespace intel_vvp_warp
