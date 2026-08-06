/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef __WARPENGINESTATE_H__
#define __WARPENGINESTATE_H__


#include <stdint.h>
#include <memory>
#include <vector>
#include <limits>
#include "WarpDataContext.h"

#undef DEBUG_BLOCK_LOAD

namespace intel_vvp_warp
{

using fetch_pos_t = uint16_t;

class FetchPosTracker;
using FetchPosTrackerPtr = std::shared_ptr<FetchPosTracker>;


class FetchContents
{
public:
	static constexpr fetch_pos_t LOC_INVALID = std::numeric_limits<fetch_pos_t>::max();
	
	FetchContents():
		_sfl{
			&FetchContents::_sfl0, &FetchContents::_sfl1, &FetchContents::_sfl2, &FetchContents::_sfl3,
			&FetchContents::_sfl4, &FetchContents::_sfl5, &FetchContents::_sfl6, &FetchContents::_sfl7},
		_gfl{
			&FetchContents::_gfl0, &FetchContents::_gfl1, &FetchContents::_gfl2, &FetchContents::_gfl3,
			&FetchContents::_gfl4, &FetchContents::_gfl5, &FetchContents::_gfl6, &FetchContents::_gfl7}
	{}

	void Reset(){
		std::fill_n(&_fc0[0][0], MAX_VBLOCKS * MAX_HBLOCKS, LOC_INVALID);
		std::fill_n(&_fc1[0][0], (MAX_VBLOCKS>>1) * (MAX_HBLOCKS>>1), LOC_INVALID);
		std::fill_n(&_fc2[0][0], (MAX_VBLOCKS>>2) * (MAX_HBLOCKS>>2), LOC_INVALID);
		std::fill_n(&_fc3[0][0], (MAX_VBLOCKS>>3) * (MAX_HBLOCKS>>3), LOC_INVALID);
		std::fill_n(&_fc4[0][0], (MAX_VBLOCKS>>4) * (MAX_HBLOCKS>>4), LOC_INVALID);
		std::fill_n(&_fc5[0][0], (MAX_VBLOCKS>>5) * (MAX_HBLOCKS>>5), LOC_INVALID);
		std::fill_n(&_fc6[0][0], (MAX_VBLOCKS>>6) * (MAX_HBLOCKS>>6), LOC_INVALID);
		std::fill_n(&_fc7[0][0], (MAX_VBLOCKS>>7) * (MAX_HBLOCKS>>7), LOC_INVALID);
	}

	inline void SetFetchLocation(const uint8_t mm_level, const int32_t v, const int32_t h, const fetch_pos_t pos){
		const uint8_t mm = _trim_mm(mm_level);
		((*this).*(_sfl[mm]))(_trim_v(mm,v), _trim_h(mm,h), pos);
	}

	inline fetch_pos_t GetFetchLocation(const uint8_t mm_level, const int32_t v, const int32_t h){
		const uint8_t mm = _trim_mm(mm_level);
		return ((*this).*(_gfl[mm]))(_trim_v(mm,v), _trim_h(mm,h));
	}

private:
	// Maximum v/h block dimensions
	// given maximum 8K resolution of 8192x8192
	// and default block size of 16x8 pixels
	static constexpr uint32_t MAX_VBLOCKS = 1024;
	static constexpr uint32_t MAX_HBLOCKS = 512;

	static constexpr uint32_t MAX_MIPMAP_LEVEL = 7;

	inline uint8_t _trim_mm(const uint8_t mm_level){
		return mm_level & MAX_MIPMAP_LEVEL;
	}

	inline int32_t _trim_v(const uint8_t mm, const int32_t v){
		return v & ((MAX_VBLOCKS >> mm) - 1);
	}

	inline int32_t _trim_h(const uint8_t mm, const int32_t h){
		return h & ((MAX_HBLOCKS >> mm) - 1);
	}	

	void _sfl0(const int32_t v, const int32_t h, const fetch_pos_t pos){ _fc0[v][h] = pos;}
	void _sfl1(const int32_t v, const int32_t h, const fetch_pos_t pos){ _fc1[v][h] = pos;}
	void _sfl2(const int32_t v, const int32_t h, const fetch_pos_t pos){ _fc2[v][h] = pos;}
	void _sfl3(const int32_t v, const int32_t h, const fetch_pos_t pos){ _fc3[v][h] = pos;}
	void _sfl4(const int32_t v, const int32_t h, const fetch_pos_t pos){ _fc4[v][h] = pos;}
	void _sfl5(const int32_t v, const int32_t h, const fetch_pos_t pos){ _fc5[v][h] = pos;}
	void _sfl6(const int32_t v, const int32_t h, const fetch_pos_t pos){ _fc6[v][h] = pos;}
	void _sfl7(const int32_t v, const int32_t h, const fetch_pos_t pos){ _fc7[v][h] = pos;}

	fetch_pos_t _gfl0(const int32_t v, const int32_t h){ return _fc0[v][h];}
	fetch_pos_t _gfl1(const int32_t v, const int32_t h){ return _fc1[v][h];}
	fetch_pos_t _gfl2(const int32_t v, const int32_t h){ return _fc2[v][h];}
	fetch_pos_t _gfl3(const int32_t v, const int32_t h){ return _fc3[v][h];}
	fetch_pos_t _gfl4(const int32_t v, const int32_t h){ return _fc4[v][h];}
	fetch_pos_t _gfl5(const int32_t v, const int32_t h){ return _fc5[v][h];}
	fetch_pos_t _gfl6(const int32_t v, const int32_t h){ return _fc6[v][h];}
	fetch_pos_t _gfl7(const int32_t v, const int32_t h){ return _fc7[v][h];}

	void (FetchContents::*_sfl[8])(const int32_t, const int32_t, const fetch_pos_t);
	fetch_pos_t (FetchContents::*_gfl[8])(const int32_t, const int32_t);

	fetch_pos_t _fc0[MAX_VBLOCKS][MAX_HBLOCKS];
	fetch_pos_t _fc1[MAX_VBLOCKS>>1][MAX_HBLOCKS>>1];
	fetch_pos_t _fc2[MAX_VBLOCKS>>2][MAX_HBLOCKS>>2];
	fetch_pos_t _fc3[MAX_VBLOCKS>>3][MAX_HBLOCKS>>3];
	fetch_pos_t _fc4[MAX_VBLOCKS>>4][MAX_HBLOCKS>>4];
	fetch_pos_t _fc5[MAX_VBLOCKS>>5][MAX_HBLOCKS>>5];
	fetch_pos_t _fc6[MAX_VBLOCKS>>6][MAX_HBLOCKS>>6];
	fetch_pos_t _fc7[MAX_VBLOCKS>>7][MAX_HBLOCKS>>7];
};

class WarpEngineState
{
public:
	WarpEngineState(const WarpHwContextPtr hw);
	void Reset();

	// Engine geometry
	uint32_t _v_start;
	uint32_t _v_end;
	uint32_t _h_start;
	uint32_t _h_end;

	uint32_t _v_process_block;
	uint32_t _h_process_block;
	uint32_t _run_until;
	uint32_t _run_until_prev;
	uint32_t _wait_until;
	uint32_t _wait_until_prev;
	std::vector<uint16_t> _used_count;
	uint32_t _fetch_rom_address;

	FetchPosTrackerPtr _fetch_tracker;

    // Minimum distance between wait_until and run_until counters
    // as calculated by the software for the current warp
    uint32_t _wait_run_min;

    uint8_t _mm_level_max;

#ifdef DEBUG_BLOCK_LOAD
	uint8_t _load_count[270][240];
#endif /*DEBUG_BLOCK_LOAD*/

	inline fetch_pos_t GetBlockCacheLocation(const uint8_t mm_level, const int32_t v, const int32_t h){
		return _fc.GetFetchLocation(mm_level, v, h);
	}

	inline void LoadBlockToCache(const uint8_t mm_level, const int32_t v, const int32_t h, const fetch_pos_t pos){
		_fc.SetFetchLocation(mm_level, v, h, pos);
	}

	inline void UnloadBlockFromCache(const uint8_t mm_level, const int32_t v, const int32_t h){
		_fc.SetFetchLocation(mm_level, v, h, FetchContents::LOC_INVALID);
	}	

private:
	// Contains index of the input block in the cache
	FetchContents _fc;
};


class FetchPosTracker
{
public:
	FetchPosTracker(const fetch_pos_t FETCH_BLOCKS);
	~FetchPosTracker();

	void Reset();
	fetch_pos_t Get();
	void Release(fetch_pos_t pos);

private:
	fetch_pos_t _FETCH_BLOCKS;
	std::vector<fetch_pos_t> _rpos;
	fetch_pos_t _pos;
};

} // namespace intel_vvp_warp

#endif /* __WARPENGINESTATE_H__ */
