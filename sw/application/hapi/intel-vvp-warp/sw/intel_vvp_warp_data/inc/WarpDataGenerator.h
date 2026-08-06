/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#ifndef _WARPDATAGENERATOR_H_
#define _WARPDATAGENERATOR_H_


#include <memory>
#include <vector>
#include <cstring>
#include <stdint.h>
#include <stdexcept>
#include <atomic>
#include <functional>
#include "WarpDataImpl.h"
#include "WarpBlockMapping.h"
#include "WarpEngineState.h"
#include "WarpDataContext.h"
#include "WarpMesh.h"

#ifdef __GNUC__
#define PACKED( __Declaration__ ) __Declaration__ __attribute__((__packed__))
#endif

#ifdef _MSC_VER
#define PACKED( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__ __pragma( pack(pop))
#endif


namespace intel_vvp_warp
{

class WarpDataGenerator
{
public:
	WarpDataGenerator();
	virtual ~WarpDataGenerator();

    WarpDataPtr GenerateData(const WarpDataContext& ctx, WarpMeshPtr mesh);
	WarpLatencyParams GenerateLatencyParams(const WarpDataContext& ctx, const WarpDataPtr warp_data, uint32_t system_clock, uint32_t video_clock, uint32_t full_height, uint32_t frame_rate) const;

	enum ErrorCode
	{
		Success = 0,
		BlockCacheUsageHigh,
		DataAllocError,
		CompressionTooHigh,
		BlockCacheFull,
		FetchLogicError,
		OutOfFetchMem,
		Unknown
	};

	uint32_t GetLastErrorCode() const;
	std::string GetErrorString(const uint32_t error_code) const;

private:
	static constexpr uint32_t MIN_WAIT_RUN_DISTANCE	= 2;	// In video lines

	bool ValidateInputParams(const WarpDataContext& ctx, const WarpMeshPtr mesh);
	
    WarpDataImplPtr AllocateWarpData(const WarpDataContext& ctx, const WarpMesh* mesh);

    bool GenerateMeshData(const WarpDataContext& ctx, WarpMesh* mesh, WarpDataImplPtr data);
    bool GenerateFetchAndFilterData(const WarpDataContext& ctx, const WarpMesh* mesh, WarpDataImplPtr data);

	bool OutputToInputMapperMesh8x8(const WarpDataContext& ctx, const WarpMesh* mesh, uint32_t v_begin, uint32_t v_end);
    bool OutputToInputMapperMesh16x16(const WarpDataContext& ctx, const WarpMesh* mesh, uint32_t v_begin, uint32_t v_end);

    template<bool MIPMAP_ENABLE>
	bool BlockCacheWrapper(const WarpDataContext& ctx, const uint32_t engine, WarpDataImplPtr data);
	bool RunOutputProcess(const WarpDataContext& ctx, uint32_t engine);

	void ResetBlockMap(uint32_t max_blocks);
	void ResetEngineState(const WarpDataContext& ctx);

	void PopulateFilterEntry(filter_entry_t& filter_entry, const fetch_pos_t* fetch_pos, const block_descriptor_t* bd);    
	void PopulateFetchEntry(fetch_entry_t& fetch_entry, const uint32_t engine, const fetch_pos_t fetch_idx, const int32_t iv, const int32_t ih, const uint8_t mm_level);
	void InsertFetchNopEntries(const uint32_t engine, WarpDataImplPtr data, const uint32_t mem_model, fetch_entry_t** last_fetch_entry);
	bool FinalizeFetchData(fetch_entry_t* last_fetch_entry, WarpDataImplPtr data, const uint32_t engine);

    bool GenerateFetchAndFilterDataRaster(const WarpDataContext& ctx, const WarpMesh* mesh, WarpDataImplPtr data);
	void ResetFilterCache(const WarpDataContext& ctx);
    template<bool MIPMAP_ENABLE>
	bool BlockCacheWrapperRaster(const WarpDataContext& ctx, const uint32_t engine, WarpDataImplPtr data);
	bool RunRasterOutputProcess(const WarpDataContext& ctx, uint32_t engine);

    inline void UpdateInputSkipRam(const int16_t bv, const int16_t bh, const uint8_t mm_level, uint8_t* skip_ram_data)
    {
        static constexpr uint8_t skip_ram_row_offset[8] = {
            0,
            SKIP_MEGABLOCK_HEIGHT_MAX,
            static_cast<uint8_t>(skip_ram_row_offset[1] + (SKIP_MEGABLOCK_HEIGHT_MAX >> 1)),
            static_cast<uint8_t>(skip_ram_row_offset[2] + (SKIP_MEGABLOCK_HEIGHT_MAX >> 2)),
            static_cast<uint8_t>(skip_ram_row_offset[3] + (SKIP_MEGABLOCK_HEIGHT_MAX >> 3)),
            static_cast<uint8_t>(skip_ram_row_offset[4] + (SKIP_MEGABLOCK_HEIGHT_MAX >> 4)),
            static_cast<uint8_t>(skip_ram_row_offset[5] + (SKIP_MEGABLOCK_HEIGHT_MAX >> 5)),
            static_cast<uint8_t>(skip_ram_row_offset[6] + (SKIP_MEGABLOCK_HEIGHT_MAX >> 6)),
        };

        const uint16_t mb_v = (static_cast<uint16_t>(bv) >> 3);
        const uint16_t mb_h = (static_cast<uint16_t>(bh) >> 3);

        uint32_t offset = (skip_ram_row_offset[mm_level] + mb_v) * SKIP_MEGABLOCK_WIDTH_MAX + mb_h;
        skip_ram_data[offset] = 0;        
    }

    void PrintBlockMapping(const block_descriptor_t& bd);    

	// Output to input block map
	std::vector<block_descriptor_t> _block_map;

	// Engine runtime state
	std::vector<WarpEngineState> _state;	

	struct filter_grid_t
	{
		fetch_pos_t _fetch_loc[MAX_BLOCK_SPAN];
		filter_grid_t(const fetch_pos_t BLOCK_CACHE_NA){ std::fill(&_fetch_loc[0], &_fetch_loc[MAX_BLOCK_SPAN], BLOCK_CACHE_NA); }
	};

	struct page_filter_data_t
	{
		std::vector<filter_grid_t> _filter_grid;
	};

	// Filter data cache for the raster scan
	std::vector<page_filter_data_t> _page_filter_cache;

	std::atomic<uint32_t> _error_code;
};

} // namespace intel_vvp_warp

#endif /* _WARPDATAGENERATOR_H_ */