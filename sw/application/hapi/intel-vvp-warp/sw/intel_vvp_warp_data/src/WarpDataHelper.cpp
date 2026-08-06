/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WarpDataHelper.h"
#include <cstdlib>

namespace intel_vvp_warp
{

namespace WarpDataHelper
{

static constexpr uint32_t SKIP_RAM_PAGES_MAX = 2;


WarpHwContextPtr GetHwContext(intel_vvp_warp_channel_t* ch)
{
	WarpHwContextPtr hw{ nullptr };

	if (ch && ch->instance)
	{
		hw = std::make_shared<WarpHwContext>();

		if (hw)
		{
			intel_vvp_warp_instance_t* wrp = ch->instance;

			hw->_engines = ch->num_engines;
			hw->_streams = wrp->streams;
			hw->_block_width = wrp->block_width;
			hw->_block_height = wrp->block_height;
			hw->_block_cache_size = ch->block_cache_size;
			hw->_block_cache_size_max = wrp->cache_size_max;
			hw->_output_skip = (wrp->output_skip == 1);
			hw->_mem_map = wrp->mem_map ? (wrp->mem_map == 3 ? WarpMemMap::E8KUHDTV : (wrp->mem_map == 2 ? WarpMemMap::EUHDTV : WarpMemMap::EHDTV)) : WarpMemMap::ESDTV;
			hw->_mem_model = wrp->mem_model;
			hw->_pixel_filter = (ch->pixel_filter == EBicubic) ? WarpFilterType::EBICUBIC : WarpFilterType::EBILINEAR;
			// Output block scan is assumed if the output bounce is present (either block or raster)
			hw->_output_block_scan = (ch->output_block_scan ? true : false);
			// Engine megablock output order is assumed if the output bounce is present.
			// Otherwise raster output order is assumed.
			// Engine block output is not currently supported
			hw->_megablock = hw->_output_block_scan;
			hw->_mipmap_enable = wrp->mipmap_enable;
		}
	}

	return hw;
}


WarpDriverDataPtr AllocateDriverData(const uint32_t num_engines)
{
	const std::size_t sz = sizeof(intel_vvp_warp_data_t) + num_engines * sizeof(intel_vvp_warp_engine_data_t);
	WarpDriverDataPtr warp_data{
		(intel_vvp_warp_data_t*)malloc(sz),
		[](intel_vvp_warp_data_t* p){free(p);}
	};
	return warp_data;
}


WarpDriverDataPtr GenerateDriverData(const WarpDataContext& ctx, const WarpDataPtr user_data, const uint32_t coef_base_address, const uint32_t skip_ram_page)
{
	WarpDriverDataPtr driver_data{nullptr};
	
	if(user_data)
	{
		uint32_t engines = user_data->GetEngines();
		
		// Allocate intel_vvp_warp_data_t object for the required number of engines
		driver_data = AllocateDriverData(engines);
		
		if(driver_data)
		{
			// Warp coefficient table sizes could be in the range
			// 128Kb - 8Mb rounded up to the nearest power of two
			auto roundup_size = [](const uint32_t size)->uint32_t {
				static constexpr uint32_t DATA_SIZE_128KB = (128 * 1024);
				static constexpr uint32_t DATA_SIZE_8192KB = DATA_SIZE_128KB << 6;

				uint32_t sr = 0;

				if (size < DATA_SIZE_128KB)
					sr = DATA_SIZE_128KB;
				else if (size <= DATA_SIZE_8192KB)
					sr = get_next_pwr_two(size);	

				return sr;
			};

			const uint32_t mesh_stride = engine_mesh_stride(std::max(ctx._engine_width[0], ctx._engine_width[1]), user_data->GetMeshStep());

			// Populate intel_vvp_warp_data_t object
			driver_data->num_engines = engines;
			intel_vvp_warp_engine_data_t* engine_data = driver_data->engine_data;

            static constexpr uint32_t blocks_per_raster_segment = 8;

            // In raster scan mode engine processing width is in multiples of 128 pixels
            const uint32_t last_hblock_idx = (ctx._hw->_output_block_scan ? ctx._hblocks_out : roundup(ctx._hblocks_out, blocks_per_raster_segment)) - 1;

            uint32_t eng_coef_begin = coef_base_address;
			uint32_t engine_h_start = 0;
			
			for(uint32_t i = 0; i < engines; ++i)
			{				
				// Processing is split between two engines
				// 1st engine processes left half, 2nd - right half of the frame
				// Configure processing geometry here
				engine_data[i].start_h = engine_h_start;
				engine_data[i].start_v = 0;

				const uint32_t engine_hblocks = ctx._engine_hblocks[i & 0x1];
				engine_h_start += engine_hblocks;

				engine_data[i].end_h = std::min(last_hblock_idx, (engine_data[i].start_h + engine_hblocks - 1));
				engine_data[i].end_v = ctx._vblocks_out - 1;
				engine_data[i].mesh_stride = mesh_stride / 4 - 1; // Mesh nodes in multiples of 4 less 1

				// The size of actual data in bytes used when copying to the FPGA
                engine_data[i].mesh_size = user_data->GetEngineData(i)->GetMeshEntries() * sizeof(mesh_entry_t);
                engine_data[i].filter_size = user_data->GetEngineData(i)->GetFilterEntries() * sizeof(filter_entry_t);
                engine_data[i].fetch_size = user_data->GetEngineData(i)->GetFetchEntries() * sizeof(fetch_entry_t);

				// Padded size in the range 128KB - 8MB rounded up to the nearest power of two
				// Warp engine HW requires this granularity
                const uint32_t mesh_size_rounded = roundup_size(engine_data[i].mesh_size);
                const uint32_t filter_size_rounded = roundup_size(engine_data[i].filter_size);
                const uint32_t fetch_size_rounded = roundup_size(engine_data[i].fetch_size);

                // Estimate total size of all tables for the engine
				// Engine data arranged like this: [mesh   ][filter ][fetch  ]
                uint32_t eng_coef_end = eng_coef_begin;
                eng_coef_end = roundup_pwr_two(eng_coef_end + engine_data[i].mesh_size, filter_size_rounded);
                eng_coef_end = roundup_pwr_two(eng_coef_end + engine_data[i].filter_size, fetch_size_rounded);
				eng_coef_end = eng_coef_end + engine_data[i].fetch_size - 1;

                static constexpr uint32_t MASK_PAGE_16MB = 0xff000000;
                const uint32_t page_begin = eng_coef_begin & MASK_PAGE_16MB;
                const uint32_t page_end = eng_coef_end & MASK_PAGE_16MB;

                // If the tables do not fit into the same 16Mb page
                // start at the beginning of the next page
                if(page_begin != page_end)
                    eng_coef_begin = roundup_pwr_two(eng_coef_begin, (~MASK_PAGE_16MB) + 1);

				// Point engine to the location of the mesh, filter and fetch data
				engine_data[i].mesh_addr = roundup_pwr_two(eng_coef_begin, mesh_size_rounded);
				engine_data[i].filter_addr = roundup_pwr_two(engine_data[i].mesh_addr + engine_data[i].mesh_size, filter_size_rounded);
				engine_data[i].fetch_addr = roundup_pwr_two(engine_data[i].filter_addr + engine_data[i].filter_size, fetch_size_rounded);

                eng_coef_begin = engine_data[i].fetch_addr + engine_data[i].fetch_size;               
			}
			
			// Set Skip RAM data
			driver_data->skip_megablock_data = user_data->GetSkipMegablockData();
			driver_data->skip_ram_page = skip_ram_page % SKIP_RAM_PAGES_MAX;
            driver_data->mipmap_level = user_data->GetMipmapLevel();
            driver_data->skip_ram_input = user_data->GetSkipRamInputData();
            driver_data->mesh_step = user_data->GetMeshStep();
		}
	}
	
	return driver_data;
}


float GetBlockCacheUtilization(const WarpDataContext& ctx, const WarpDataPtr user_data, const uint32_t engine)
{
	float v = 0.0f;

	if(user_data && (user_data->GetEngines() > engine))
	{
		const uint32_t cache_loads_actual = user_data->GetEngineData(engine) ? user_data->GetEngineData(engine)->GetFetchEntries() : 0;
		const uint32_t cache_loads_expected = (ctx._vblocks_in[0] * ctx._hblocks_in[0]);
		v = static_cast<float>(cache_loads_actual) / static_cast<float>(cache_loads_expected);		
	}

	return v;
}


} // WarpDataHelper

} // intel_vvp_warp
