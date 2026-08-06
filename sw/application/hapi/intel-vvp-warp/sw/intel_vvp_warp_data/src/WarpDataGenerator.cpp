/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#define DONT_USE_DIV

#include <cassert>
#include <algorithm>
#include <functional>
#include <map>
#include "WarpDataGenerator.h"
#include "WarpDataImpl.h"


#ifndef ALT_SINGLE_THREADED
#include <future>
#endif /* ALT_SINGLE_THREADED */

#ifdef INTEL_VVP_WARP_DATA_ENABLE_LOGGING
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#define INTEL_VVP_WARP_DATA_LOG(...)	printf(__VA_ARGS__)
#else
#define INTEL_VVP_WARP_DATA_LOG(...)
#endif /* INTEL_VVP_WARP_DATA_ENABLE_LOGGING */


#undef DEBUG_BLOCK
#define DEBUG_BLOCK_V 0
#define DEBUG_BLOCK_H 0


namespace intel_vvp_warp
{


WarpDataGenerator::WarpDataGenerator():
	_block_map{},
	_state{},
	_page_filter_cache{},
	_error_code{uint32_t{ErrorCode::Unknown}}
{
}


WarpDataGenerator::~WarpDataGenerator()
{
}


WarpDataImplPtr WarpDataGenerator::AllocateWarpData(const WarpDataContext& ctx, const WarpMesh* mesh)
{
	static constexpr std::size_t DATA_SIZE_MAX = 0x800000; // Current RTL limit is 8Mb

	const uint32_t num_engines = ctx._hw->_engines;

	/* Mesh entries*/
	const uint32_t num_streams = mesh->GetStreams();
    const uint32_t mesh_step = mesh->GetStep();
	const uint32_t mesh_rows = roundup(ctx._height_out, mesh_step) / mesh_step + 1;

	const uint32_t mesh_stride = engine_mesh_stride(std::max(ctx._engine_width[0], ctx._engine_width[1]), mesh_step);
	const uint32_t mesh_entries = mesh_stride * mesh_rows * num_streams;

	// Allocate maximum possible number of cache entries
	const uint32_t fetch_entries = DATA_SIZE_MAX / sizeof(fetch_entry_t);

	/* Filter coefficients */
	const uint32_t ho_blocks = std::max(ctx._engine_hblocks[0], ctx._engine_hblocks[1]);
	const uint32_t vo_blocks = ctx._vblocks_out;
	const uint32_t filter_entries = vo_blocks * ho_blocks;

	WarpDataImplPtr warp_data{ nullptr };

	// Sanity check against max possible engine data size (2Mb)
	auto check_warp_data_size = [](const char* name, const std::size_t sz)->bool
	{
		bool size_exceeds = (sz > DATA_SIZE_MAX);
		if(size_exceeds)
		{
			INTEL_VVP_WARP_DATA_LOG("%s data size: %zu exceeds engine limit!\n", name, sz);
		}
		return !size_exceeds;
	};

	bool size_ok = true;
	size_ok = size_ok && check_warp_data_size("Mesh", mesh_entries * sizeof(mesh_entry_t));
	size_ok = size_ok && check_warp_data_size("Fetch", fetch_entries * sizeof(fetch_entry_t));
	size_ok = size_ok && check_warp_data_size("Filter", filter_entries * sizeof(filter_entry_t));

	try
	{
		if(size_ok)
			warp_data = std::make_shared<WarpDataImpl>(num_engines, mesh_entries, filter_entries, fetch_entries);
	}
	catch (const std::bad_alloc& e)
	{
		(void)(e);
		INTEL_VVP_WARP_DATA_LOG("Unable to allocate memory for warp data!\n");
		_error_code = static_cast<uint32_t>(ErrorCode::DataAllocError);
	}

	return warp_data;
}


bool  WarpDataGenerator::ValidateInputParams(const WarpDataContext& ctx, const WarpMeshPtr mesh)
{
	bool ret = false;

	if(ctx._hw && mesh)
	{
        if(ctx._hw->_pixel_filter == WarpFilterType::EBICUBIC)
        {
            if(ctx._hw->_streams == mesh->GetStreams())
            {
                const uint32_t max_dim = max_frame_dim(ctx._hw->_mem_map);

                if((ctx._width_in > max_dim) || (ctx._height_in > max_dim) ||
                        (ctx._width_out > max_dim) || (ctx._height_out > max_dim))
                {
                    INTEL_VVP_WARP_DATA_LOG("Maximum image size exceeded!\n");
                }
                else
                    ret = true;
            }
            else
                INTEL_VVP_WARP_DATA_LOG("Invalid number of mesh streams: provided: %" PRIu32 " required: %" PRIu32 "\n",  mesh->GetStreams(), ctx._hw->_streams);
        }
        else
            INTEL_VVP_WARP_DATA_LOG("Unsupported pixel filter! Current software version supports Bicubic.");
	}
	else
		INTEL_VVP_WARP_DATA_LOG("Hardware context not initialized!\n");

	return ret;
}


WarpDataPtr WarpDataGenerator::GenerateData(const WarpDataContext& ctx, WarpMeshPtr mesh)
{
	WarpDataPtr data{nullptr};
	_error_code = static_cast<uint32_t>(ErrorCode::Unknown);

	if(ValidateInputParams(ctx, mesh))
	{
		auto tmp_data = AllocateWarpData(ctx, mesh.get());

		if (tmp_data)
		{
			bool ret = GenerateMeshData(ctx, mesh.get(), tmp_data);

			if(ret)
			{
				if(ctx._hw->_output_block_scan)
					ret = GenerateFetchAndFilterData(ctx, mesh.get(), tmp_data);
				else
					ret = GenerateFetchAndFilterDataRaster(ctx, mesh.get(), tmp_data);

				if(ret)
				{
                    // Set the maximum mipmap level
                    if(ctx._hw->_mipmap_enable)
                    {                       
                        uint8_t mipmap_level_max = _state[0]._mm_level_max;

                        for(uint32_t e = 1; e < ctx._hw->_engines; ++e)
                            mipmap_level_max = std::max(mipmap_level_max, _state[e]._mm_level_max);

                        tmp_data->SetMipmapLevel(mipmap_level_max);
                    }

					data = tmp_data;

					if(_error_code == ErrorCode::Unknown)
						_error_code = static_cast<uint32_t>(ErrorCode::Success);
				}
			}
		}
	}

	return data;
}


WarpLatencyParams WarpDataGenerator::GenerateLatencyParams(const WarpDataContext& ctx, const WarpDataPtr warp_data, uint32_t system_clock, uint32_t video_clock, uint32_t full_height, uint32_t frame_rate) const
{
	// Minimum numbers of video lines received before engine can start processing
	const uint32_t input_delay_blocks = ctx._hw->_output_block_scan ? 8 : 2;
	const uint32_t input_delay_lines = input_delay_blocks * ctx._hw->_block_height;

	// Engine takes time to process data
	// Depending on the output bounce assume either a megablock/block of lines
	// or only 2 lines for the direct (raster) output
	const uint32_t engine_min_delay_lines = ctx._hw->_output_block_scan ? height_alignment(ctx._hw->_megablock, ctx._hw->_block_height) : 2;

	// Number of video lines required by B2R before it can produce output
	const uint32_t b2r_delay_lines = (ctx._hw->_output_block_scan ? ctx._hw->_block_height : 2);

	WarpLatencyParams params{ 0,0 };

	uint32_t frame_period_sysclk = static_cast<uint32_t>((static_cast<uint64_t>(system_clock) * 100) / frame_rate);
	uint32_t warp_output_delay_lines = engine_min_delay_lines + b2r_delay_lines;
	uint32_t warp_output_latency = 1;	// Initialize to the smallest possible offset (used in the single bounce setup)

	// In dual bounce setup calculate and use actual offset
	if(ctx._hw->_output_block_scan)
		warp_output_latency = frame_period_sysclk - static_cast<uint32_t>((static_cast<uint64_t>(frame_period_sysclk) * (warp_output_delay_lines)) / full_height);

	params._output_latency = warp_output_latency;

	uint32_t frame_period_vidclk = static_cast<uint32_t>((static_cast<uint64_t>(video_clock) * 100) / frame_rate);
	uint32_t engine_delay_lines = input_delay_lines + std::max(roundup(warp_data->GetLatencyLines(), engine_min_delay_lines), engine_min_delay_lines);
	uint32_t v_blanking = full_height - ctx._height_out;
	uint32_t total_latency_lines = engine_delay_lines + warp_output_delay_lines + v_blanking;
	uint32_t warp_total_latency = static_cast<uint32_t>((static_cast<uint64_t>(frame_period_vidclk) * total_latency_lines) / full_height);

	params._total_latency = warp_total_latency;

	return params;
}


uint32_t WarpDataGenerator::GetLastErrorCode() const
{
	return _error_code;
}


std::string WarpDataGenerator::GetErrorString(const uint32_t error_code) const
{
	static const std::map<uint32_t, const char*> error_string
	{
		{ErrorCode::Success,				"Success"},
		{ErrorCode::BlockCacheUsageHigh,	"Block cache usage too high"},
		{ErrorCode::DataAllocError,			"Data memory allocation error"},
		{ErrorCode::CompressionTooHigh,		"Compression too high"},
		{ErrorCode::BlockCacheFull,			"Block cache full"},
		{ErrorCode::FetchLogicError,		"Fetch logic error"},
		{ErrorCode::OutOfFetchMem,			"Too many block cache loads"},
		{ErrorCode::Unknown,				"Unknown error"}
	};

	const uint32_t key = (error_string.count(error_code) > 0 ? static_cast<ErrorCode>(error_code) : ErrorCode::Unknown);

	return error_string.at(key);
}


bool WarpDataGenerator::GenerateMeshData(const WarpDataContext& ctx, WarpMesh* mesh, WarpDataImplPtr data)
{
	bool ret = false;

	// Single engine mesh stride used in the FW for UHDTV memory model: 256 nodes
	// which, given the mesh interval of 8 pixels limits maximum line width to
	//
	//  	8 * (256 - 1) = 2040 pixels. Note: 3 mesh nodes but 2 intervals:  *--*--*
	//
	// To simplify memory allocation and alignment we will also round down number of
	// vertical nodes to the nearest power of 2:
	//
	//		4096 / 8 = 512 intervals which requires 513 vertical nodes.
	// 		We will limit vertical nodes to 512
	//
	//		New maximum height:
	//
	//		8 * (512 - 1) = 4088 pixels
	const uint32_t mem_model = ctx._hw->_mem_model;

	// Engine processing width rounded to megablock boundary
	// Used for all engines except the last one
	// Currently HW expects mesh streams to be interleaved in memory
	// and have the same dimension
    const uint32_t num_streams = mesh->GetStreams();
	const uint32_t v_nodes = mesh->GetVNodes();
	const uint32_t mesh_step = mesh->GetStep();

	float max_distance = 0.0f;
    const uint32_t mesh_fract_bits = mesh->GetFractBits();
	float wif = static_cast<float>(ctx._width_in << mesh_fract_bits);
	float hif = static_cast<float>(ctx._height_in << mesh_fract_bits);
	float hof = static_cast<float>(ctx._height_out);

	const uint32_t mesh_stride = engine_mesh_stride(std::max(ctx._engine_width[0], ctx._engine_width[1]), mesh_step);

	uint32_t h_nodes_begin = 0;

	for( uint32_t engine = 0; engine < ctx._hw->_engines; ++engine )
	{
		const uint32_t engine_h_nodes = (ctx._engine_width[engine & 0x1] / mesh_step) + 1;
		const uint32_t h_nodes_current = std::min(engine_h_nodes, (mesh->GetHNodes() - h_nodes_begin));

		assert(h_nodes_current <= mesh_stride);

		mesh_entry_t* mesh_data = data->GetEngineData(engine)->GetMeshData();

		for( uint32_t v = 0; v < v_nodes; ++v )
		{
			float yo_norm = static_cast<float>(v * mesh_step) / hof;

            uint32_t entry_index = (v * mesh_stride * num_streams);
            mesh_node_t* node = mesh->GetRow(v) + h_nodes_begin * num_streams;

			for( uint32_t h = 0; h < (h_nodes_current * num_streams); ++h )
			{
            	node->_x = clamp_mesh_value(node->_x);
				node->_y = clamp_mesh_value(node->_y);

				uint32_t
				data_word  = ((node->_x) & 0xffff) << 0;
				data_word += ((node->_y) & 0xffff) << 16;

                uint32_t byte_address = entry_index << 2;
                uint32_t remap_address = map_address(byte_address, mem_model);

                mesh_data[remap_address >> 2] = data_word;

                float yi_norm = static_cast<float>(node->_y) / hif;
                float xi_norm = static_cast<float>(node->_x) / wif;

                ++node;
                ++entry_index;

                if ((yi_norm >= 0.0f) && (yi_norm < 1.0f) &&
                        (xi_norm >= 0.0f) && (xi_norm < 1.0f))
                {
                    float distance = std::max((yi_norm - yo_norm), 0.0f);
                    max_distance = std::max(max_distance, distance);
                }
			}
		}

		h_nodes_begin += (engine_h_nodes - 1);
	}   

	data->SetLatencyLines(static_cast<uint32_t>(max_distance * hof + 0.5f));
    data->SetMeshStep(mesh_step);
	ret = true;

	return ret;
}


void WarpDataGenerator::ResetBlockMap(uint32_t max_blocks)
{
	if(_block_map.size() < max_blocks)
		_block_map.resize(max_blocks);

	std::fill_n(reinterpret_cast<uint8_t*>(_block_map.data()), _block_map.size() * sizeof(block_descriptor_t), 0x0);
}


void WarpDataGenerator::ResetEngineState(const WarpDataContext& ctx)
{
	_state.clear();

	// Initialise warp engine state objects
	for (std::size_t e = 0; e < ctx._hw->_engines; ++e)
	{
		 _state.emplace_back( ctx._hw );
		WarpEngineState& wes{_state.back()};

		if(ctx._hw->_output_block_scan)
		{
			wes._v_start = 0;
			wes._v_end = ctx._vblocks_out - 1;
			wes._h_start = ((e == 0) ? 0 : _state[e - 1]._h_end + 1);
			wes._h_end = std::min(ctx._hblocks_out, wes._h_start + ctx._engine_hblocks[e & 0x1]) - 1;

			wes._h_process_block = wes._h_start;
		}
		else
		{
			const uint32_t raster_block_width = ctx._hw->_block_height * ctx._hw->_block_width;
			const uint32_t raster_hblocks_out = (ctx._width_out + raster_block_width - 1) / raster_block_width;

			wes._v_start = 0;
			wes._v_end = ctx._height_out - 1;

			const uint32_t raster_engine_hblocks = ctx._engine_hblocks[e & 0x1] / 8;
			wes._h_start = ((e == 0) ? 0 : _state[e - 1]._h_end + 1);
			wes._h_end = std::min(raster_hblocks_out, wes._h_start + raster_engine_hblocks) - 1;

			wes._h_process_block = wes._h_start;
		}

		wes._fetch_rom_address = 0;
	}
}


bool WarpDataGenerator::GenerateFetchAndFilterData(const WarpDataContext& ctx, const WarpMesh* mesh, WarpDataImplPtr data)
{
    bool done = false;

	ResetBlockMap(ctx._vblocks_out * ctx._hblocks_out);

	// For each output block work out which input blocks to fetch

    const uint32_t mesh_step = mesh->GetStep();

    auto otoi_mapper_func = (mesh_step == 16) ? &WarpDataGenerator::OutputToInputMapperMesh16x16 : &WarpDataGenerator::OutputToInputMapperMesh8x8;

#ifndef ALT_SINGLE_THREADED
    const uint32_t hw_concurr = std::min(static_cast<uint32_t>(std::thread::hardware_concurrency()), ctx._hw->_engines);

    if(hw_concurr > 1)
    {
        std::vector<std::future<bool>> f(hw_concurr - 1);
        const uint32_t thread_v_step = mesh->GetVNodes() / hw_concurr;
        uint32_t thread_v_begin = 0;

        for(uint32_t i = 0; i < (hw_concurr - 1); ++i, thread_v_begin += thread_v_step)
        {
            f[i] = std::async(std::launch::async, otoi_mapper_func, this, std::cref(ctx), mesh, thread_v_begin, thread_v_begin + thread_v_step);
        }

        done = std::invoke(otoi_mapper_func, this, ctx, mesh, thread_v_begin, mesh->GetVNodes()-1);

        for(uint32_t i = 0; i < (hw_concurr - 1); ++i) done = done && f[i].get();
    }
    else
#endif /* ALT_SINGLE_THREADED */        
		done = std::invoke(otoi_mapper_func, this, ctx, mesh, 0, mesh->GetVNodes()-1);

	if(done)
	{
		done = false;

		ResetEngineState(ctx);

		// Work out block fetch logic
		// Use parallel threads if more than 1 engine involved

		const uint32_t engines = ctx._hw->_engines;

		auto block_cache_func = [&](const uint32_t eng_begin, const uint32_t eng_end)->bool
		{
			bool ret = true;

			for(uint32_t e = eng_begin; e < eng_end; ++e)
			{
                if(ctx._hw->_mipmap_enable)
				    ret = ret && BlockCacheWrapper<true>(ctx, e, data);
                else
                    ret = ret && BlockCacheWrapper<false>(ctx, e, data);
			}

			return ret;
		};

#ifndef ALT_SINGLE_THREADED
		const uint32_t hw_concurr = std::min(static_cast<uint32_t>(std::thread::hardware_concurrency()), engines);

		if(hw_concurr > 1)
		{
			std::vector<std::future<bool>> f(hw_concurr - 1);
			const uint32_t engines_per_thread = engines / hw_concurr;
			uint32_t eng_begin = 0;

			for(uint32_t i = 0; i < (hw_concurr - 1); ++i, eng_begin += engines_per_thread)
				f[i] = std::async(std::launch::async, block_cache_func, eng_begin, eng_begin + engines_per_thread);

			done = block_cache_func(eng_begin, engines);

			for(uint32_t i = 0; i < (hw_concurr - 1); ++i) done = done && f[i].get();
		}
		else
#endif /* ALT_SINGLE_THREADED */
			done = block_cache_func(0, engines);

#ifdef DEBUG_BLOCK_LOAD
		INTEL_VVP_WARP_DATA_LOG("# CACHE USAGE\n");

		for(uint32_t engine=0; engine < engines; ++engine)
		{
			INTEL_VVP_WARP_DATA_LOG("# ENGINE: %d\n", engine);

			for(int v = 0; v < 270; ++v)
			{
				for(int h = 0; h < 240; ++h)
				{
					if(_state[engine]._load_count[v][h] > 1)
						INTEL_VVP_WARP_DATA_LOG("[%d, %d] -> %d\n", v, h, _state[engine]._load_count[v][h]);
				}
			}
		}
#endif /*DEBUG_BLOCK_LOAD*/       
    }

	return done;
}


bool WarpDataGenerator::RunOutputProcess(const WarpDataContext& ctx, uint32_t engine)
{
	// If unused not found run output process to identify blocks being used by processing

	const uint32_t fetch_size = ctx._hw->_block_cache_size;

	const uint32_t h_output = _state[engine]._h_process_block;
	const uint32_t v_output = _state[engine]._v_process_block;
	const uint32_t output_block_idx = v_output * ctx._hblocks_out + h_output;

	const block_descriptor_t& tmp_bd = _block_map[output_block_idx];

	if(tmp_bd._block_list_size)
	{
		const uint8_t mm_level = tmp_bd._mm_level;

		const int16_t hblock_in_max = static_cast<int16_t>(ctx._hblocks_in[mm_level]);
		const int16_t vblock_in_max = static_cast<int16_t>(ctx._vblocks_in[mm_level]);	

		// Cache contains only actual blocks (those within frame boundaries)
		const int16_t v_start = std::max(tmp_bd._min_v, static_cast<int16_t>(0));
		const int16_t v_end = std::min(tmp_bd._max_v, vblock_in_max);

		const int16_t h_start = std::max(tmp_bd._min_h, static_cast<int16_t>(0));
		const int16_t h_end = std::min(tmp_bd._max_h, hblock_in_max);

		for (int16_t iv = v_start; iv < v_end; ++iv)
		{
			for (int16_t ih = h_start; ih < h_end; ++ih)
			{
				fetch_pos_t fetch_idx = _state[engine].GetBlockCacheLocation(mm_level, iv, ih);

				//reduce the use count while checking for internal error
				if (fetch_idx < fetch_size)
				{
					if (_state[engine]._used_count[fetch_idx] > 0)
					{
						_state[engine]._used_count[fetch_idx]--;

						if (_state[engine]._used_count[fetch_idx] == 0)
						{
							_state[engine].UnloadBlockFromCache(mm_level, iv, ih);
							_state[engine]._fetch_tracker->Release(fetch_idx);
						}
					}
					else
					{
						INTEL_VVP_WARP_DATA_LOG("Error: Using block which is marked as no longer needed\n");
						_error_code = static_cast<uint32_t>(ErrorCode::FetchLogicError);
						return false;
					}
				}
				else
				{
					if (!ctx._allow_mesh_errors)
					{
						INTEL_VVP_WARP_DATA_LOG("Error: Engine [%" PRIu32 "] unloading InBlock[%i,%i] which is not in fetch - OutBlock[%" PRIu32 ",%" PRIu32 "] requires InBlocks ", engine, iv, ih, v_output, h_output);
						PrintBlockMapping(tmp_bd);
						INTEL_VVP_WARP_DATA_LOG("\n");
						_error_code = static_cast<uint32_t>(ErrorCode::FetchLogicError);
						return false;
					}
				}
			}
		}
	}

	_state[engine]._wait_until++;

	uint32_t& h_process_block = _state[engine]._h_process_block;
	uint32_t& v_process_block = _state[engine]._v_process_block;

	if (ctx._hw->_megablock)
	{
		if ((h_process_block & 0x7) == 0x7 || h_process_block >= _state[engine]._h_end)
		{
			if ((v_process_block & 0x7) == 0x7 || v_process_block >= _state[engine]._v_end)
			{
				if (h_process_block >= _state[engine]._h_end)
				{
					h_process_block = _state[engine]._h_start;
					v_process_block++;
				}
				else
				{
					h_process_block++;
					v_process_block = v_process_block & 0xfffffff8;
				}
			}
			else
			{
				h_process_block = h_process_block & 0xfffffff8;
				v_process_block++;
			}
		}
		else
		{
			h_process_block++;
		}
	}
	else
	{
		h_process_block++;

		if(h_process_block >= _state[engine]._h_end)
		{
			h_process_block = _state[engine]._h_start;
			v_process_block++;
		}
	}

	return true;
}


inline void WarpDataGenerator::PopulateFilterEntry(filter_entry_t& filter_entry, const fetch_pos_t* fetch_pos, const block_descriptor_t* bd)
{
	int32_t min_v = 0;
	int32_t min_h = 0;
	uint32_t block_coef_v = 0;
	uint32_t block_coef_h = 0;
	uint32_t mm_level = 0;

	if(bd)
	{
		min_v = bd->_min_v;
		min_h = bd->_min_h;
		block_coef_v = bd->_block_coef[1];
		block_coef_h = bd->_block_coef[0];
		mm_level = bd->_mm_level;
	}

	uint32_t* data_word = filter_entry._word;

	data_word[0]  = (fetch_pos[0] & 0x3ff) << 0;
	data_word[0] += (fetch_pos[1] & 0x3ff) << 10;
	data_word[0] += (fetch_pos[2] & 0x3ff) << 20;
	data_word[0] += (fetch_pos[3] & 0x003) << 30;
	data_word[1]  = (fetch_pos[3] & 0x3fc) >> 2;
	data_word[1] += (fetch_pos[4] & 0x3ff) << 8;
	data_word[1] += (fetch_pos[5] & 0x3ff) << 18;
	data_word[1] += (fetch_pos[6] & 0x00F) << 28;
	data_word[2]  = (fetch_pos[6] & 0x3f0) >> 4;
	data_word[2] += (fetch_pos[7] & 0x3ff) << 6;
	data_word[2] += (fetch_pos[8] & 0x3ff) << 16;
	data_word[2] += (fetch_pos[9] & 0x03F) << 26;
	data_word[3]  = (fetch_pos[9] & 0x3c0) >> 6;
	data_word[3] += (fetch_pos[10] & 0x3ff) << 4;
	data_word[3] += (fetch_pos[11] & 0x3ff) << 14;
	data_word[3] += (fetch_pos[12] & 0x0FF) << 24;
	data_word[4]  = (fetch_pos[12] & 0x300) >> 8;
	data_word[4] += (fetch_pos[13] & 0x3ff) << 2;
	data_word[4] += (fetch_pos[14] & 0x3ff) << 12;
	data_word[4] += (fetch_pos[15] & 0x3ff) << 22;
	data_word[5]  = (fetch_pos[16] & 0x3ff) << 0;
	data_word[5] += (fetch_pos[17] & 0x3ff) << 10;
	data_word[5] += (fetch_pos[18] & 0x3ff) << 20;
	data_word[5] += (fetch_pos[19] & 0x003) << 30;
	data_word[6]  = (fetch_pos[19] & 0x3fc) >> 2;
	data_word[6] += (fetch_pos[20] & 0x3ff) << 8;
	data_word[6] += (fetch_pos[21] & 0x3ff) << 18;
	data_word[6] += (fetch_pos[22] & 0x00F) << 28;
	data_word[7]  = (fetch_pos[22] & 0x3f0) >> 4;
	data_word[7] += (fetch_pos[23] & 0x3ff) << 6;

	data_word[7] += (min_h & 0x7) << 16;
	data_word[7] += (min_v & 0x7) << 19;
	data_word[7] += (block_coef_h & 0x7) << 22;
	data_word[7] += (block_coef_v & 0x7) << 25;

	data_word[7] += (mm_level & 0x7) << 28;
}


inline void WarpDataGenerator::PopulateFetchEntry(fetch_entry_t& fetch_entry, const uint32_t engine, const fetch_pos_t fetch_idx, const int32_t iv, const int32_t ih, const uint8_t mm_level)
{
	const uint32_t run_until = _state[engine]._run_until - _state[engine]._run_until_prev;
	const uint32_t wait_until = _state[engine]._wait_until - _state[engine]._wait_until_prev;

	_state[engine]._run_until_prev = _state[engine]._run_until;
	_state[engine]._wait_until_prev = _state[engine]._wait_until;

	const uint32_t lower_word = 
		(static_cast<uint32_t>((run_until & 0xfff))) | 
		(static_cast<uint32_t>(wait_until & 0xfff) << 12) |
		(static_cast<uint32_t>(mm_level & 0x7) << 24) | // Mipmap reference (currently the same for V and H)
		(static_cast<uint32_t>(mm_level & 0x7) << 28);

	const uint32_t upper_word = 
		(static_cast<uint32_t>(ih & 0x1ff)) |
		(static_cast<uint32_t>(iv & 0x3ff) << 9) |
		(static_cast<uint32_t>(fetch_idx  & 0x3ff) << 19);

	fetch_entry._lower_word = lower_word;
	fetch_entry._upper_word = upper_word;
}


void WarpDataGenerator::InsertFetchNopEntries(const uint32_t engine, WarpDataImplPtr data, const uint32_t mem_model, fetch_entry_t** last_fetch_entry)
{
	static constexpr uint32_t MAX_RUN_WAIT = 0xfff;

	const auto run_distance = _state[engine]._run_until - _state[engine]._run_until_prev;
	const auto wait_distance = _state[engine]._wait_until - _state[engine]._wait_until_prev;

	auto run_nops = run_distance / MAX_RUN_WAIT;
	auto wait_nops = wait_distance / MAX_RUN_WAIT;

	while(run_nops || wait_nops)
	{
		uint32_t run_until = 0;
		uint32_t wait_until = 0;

		if(run_nops)
		{
			run_until = MAX_RUN_WAIT;
			_state[engine]._run_until_prev += MAX_RUN_WAIT;
			--run_nops;
		}											

		if(wait_nops)
		{
			wait_until = MAX_RUN_WAIT;
			_state[engine]._wait_until_prev += MAX_RUN_WAIT;
			--wait_nops;
		}

		const uint32_t byte_address = _state[engine]._fetch_rom_address * sizeof(fetch_entry_t);
		const uint32_t remap_address = map_address(byte_address, mem_model);
		const uint32_t fetch_entry_idx = remap_address / sizeof(fetch_entry_t);

		if(fetch_entry_idx < data->GetEngineData(engine)->GetFetchEntries())
		{
			fetch_entry_t* fetch_entry = data->GetEngineData(engine)->GetFetchData() + fetch_entry_idx;

			fetch_entry->_lower_word = (static_cast<uint32_t>(run_until & 0xfff));
			fetch_entry->_lower_word |= (static_cast<uint32_t>(wait_until & 0xfff) << 12);

			fetch_entry->_upper_word = 0x80000000; // NOP

			*last_fetch_entry = fetch_entry;
		}

		_state[engine]._fetch_rom_address++;
	}	
}


bool WarpDataGenerator::FinalizeFetchData(fetch_entry_t* last_fetch_entry, WarpDataImplPtr data, const uint32_t engine)
{
	bool ret = false;

	// If there is no data for the engine due to requested transform
	// create a dummy fetch entry
	if(!last_fetch_entry)
	{
		last_fetch_entry = data->GetEngineData(engine)->GetFetchData();
		*last_fetch_entry = fetch_entry_t{0,0};
		_state[engine]._fetch_rom_address = 1;
		_state[engine]._wait_run_min = 0x0003ffff;
	}

	// Finalize block cache data and update the size
	const uint32_t cache_loads_required = _state[engine]._fetch_rom_address;

	if(cache_loads_required <= data->GetEngineData(engine)->GetFetchEntries())
	{
		last_fetch_entry->_upper_word |= 0x40000000;
		data->GetEngineData(engine)->SetFetchEntries(cache_loads_required);
		ret = true;
	}
	else
	{
		INTEL_VVP_WARP_DATA_LOG("Error: Engine [%" PRIu32 "] requires %" PRIu32 " fetch entries of %" PRIu32 " max!\n", engine, cache_loads_required, data->GetEngineData(engine)->GetFetchEntries());
		_error_code = static_cast<uint32_t>(ErrorCode::OutOfFetchMem);
	}

	return ret;
}


template<bool MIPMAP_ENABLE>
bool WarpDataGenerator::BlockCacheWrapper(const WarpDataContext& ctx, const uint32_t engine, WarpDataImplPtr data)
{
	bool ret = false;

	assert(ctx._hw->_block_cache_size <= ctx._hw->_block_cache_size_max);

	const uint32_t megablock = (ctx._hw->_megablock ? 1 : 0);
	const uint32_t i_range = (1 + (7 * megablock));		// Inner loop range. 1 for block scan 8 for megablock scan

	const uint32_t oh_start = _state[engine]._h_start / i_range;
	const uint32_t oh_end = (_state[engine]._h_end) / i_range + 1;
	const uint32_t ov_limit = (_state[engine]._v_end) / i_range + 1;

	const uint32_t hblock_out_max = ctx._hblocks_out;
	const uint32_t vblock_out_max = ctx._vblocks_out;
	const fetch_pos_t BLOCK_CACHE_NA = ctx._hw->_block_cache_size_max - 1;
	const fetch_pos_t BLOCK_CACHE_SIZE = (ctx._hw->_block_cache_size < BLOCK_CACHE_NA ? ctx._hw->_block_cache_size : BLOCK_CACHE_NA - 1);

	uint32_t filter_rom_address = 0;
	fetch_entry_t* last_fetch_entry{nullptr};

    uint8_t mm_level_max = 0;

	for( uint32_t ov = 0; ov < ov_limit; ov++ )
	{
		const uint32_t iv_limit = ((ov == (ov_limit-1)) ? (vblock_out_max - (ov * i_range)) : i_range);

		for( uint32_t oh = oh_start; oh < oh_end; oh++ )
		{
			bool megablock_blank = ctx._skip_blank && ctx._hw->_output_skip;
			uint32_t filter_rom_address_hold = filter_rom_address;

			const uint32_t ih_limit = std::min(i_range, hblock_out_max - (oh * i_range));

			for( uint32_t iv = 0; iv < iv_limit; iv++ )
			{
				const uint32_t v = ov * i_range + iv;

				for( uint32_t ih = 0; ih < ih_limit; ih++ )
				{
					const uint32_t h = oh * i_range + ih;

					fetch_pos_t filter_fetch_loc[MAX_BLOCK_SPAN];

					// Initialize as blank block
					for(uint32_t i=0; i < MAX_BLOCK_SPAN; ++i)
						filter_fetch_loc[i] = BLOCK_CACHE_NA;

					const block_descriptor_t& bd = _block_map[v * ctx._hblocks_out + h];
					const uint8_t mm_level = bd._mm_level;

					if(bd._block_list_size > 0)
					{
						const int32_t hblock_in_max  = static_cast<int32_t>(ctx._hblocks_in[mm_level]);
						const int32_t vblock_in_max = static_cast<int32_t>(ctx._vblocks_in[mm_level]);

						megablock_blank = false;

						for(uint32_t i=0; i < MAX_BLOCK_SPAN; ++i)
						{
							const int32_t bh = bd._min_h + (i % 4);
							const int32_t bv = bd._min_v + (i / 4);

							// Only care about the real blocks (those within the frame boundaries)
							if(bh < 0 || bv < 0 || bh >= hblock_in_max || bv >= vblock_in_max)
								continue;

							// If the block is required by the mesh make sure it's in the cache
							if((bh < bd._max_h) && (bv < bd._max_v))
							{
								// If block already in cache - reuse it
								fetch_pos_t fetch_idx = _state[engine].GetBlockCacheLocation(mm_level, bv, bh);

								// Otherwise find available cache location
								while(fetch_idx >= BLOCK_CACHE_SIZE)
								{
									fetch_idx = _state[engine]._fetch_tracker->Get();

									// If no available slots in cache try freeing some by running output process
									if (fetch_idx == BLOCK_CACHE_SIZE)
									{
										bool output_process_ok = RunOutputProcess(ctx, engine);

										if(!output_process_ok)
											break;

										fetch_idx = _state[engine]._fetch_tracker->Get();
									}

									if (fetch_idx < BLOCK_CACHE_SIZE)
									{
										_state[engine].LoadBlockToCache(mm_level, bv, bh, fetch_idx);

										// Insert NOP entries first if necessary
										InsertFetchNopEntries(engine, data, ctx._hw->_mem_model, &last_fetch_entry);

										const uint32_t byte_address = _state[engine]._fetch_rom_address * sizeof(fetch_entry_t);
										const uint32_t remap_address = map_address(byte_address, ctx._hw->_mem_model);
										const uint32_t fetch_entry_idx = remap_address / sizeof(fetch_entry_t);

										if(fetch_entry_idx < data->GetEngineData(engine)->GetFetchEntries())
										{
											fetch_entry_t* fetch_entry = data->GetEngineData(engine)->GetFetchData();

											PopulateFetchEntry(fetch_entry[fetch_entry_idx], engine, fetch_idx, bv, bh, mm_level);

											last_fetch_entry = &(fetch_entry[fetch_entry_idx]);
										}

										_state[engine]._fetch_rom_address++;

#ifdef DEBUG_BLOCK_LOAD
										_state[engine]._load_count[bv][bh]++;
#endif /*DEBUG_BLOCK_LOAD*/

										break;
									}
								}

								if (fetch_idx < BLOCK_CACHE_SIZE)
								{
									_state[engine]._used_count[fetch_idx]++;
									filter_fetch_loc[i] = fetch_idx;

                                    // Update R2B skip RAM
                                    if constexpr (MIPMAP_ENABLE)                                        
                                        UpdateInputSkipRam(bv, bh, mm_level, data->GetSkipRamInputData());

								}
								else
								{
									INTEL_VVP_WARP_DATA_LOG("Error: Engine [%" PRIu32 "] InBlock[%i,%i] required by OutBlock[%" PRIu32 ",%" PRIu32 "]: Unable to load into cache!\n", engine, bv, bh, v, h);
									_error_code = static_cast<uint32_t>(ErrorCode::BlockCacheFull);
									return false;
								}
							}
						}
					}

					uint32_t byte_address = filter_rom_address * sizeof(filter_entry_t);
					uint32_t remap_address  = map_address(byte_address, ctx._hw->_mem_model);

					// Note: remap_address accepts and returns byte offsets
					// Convert it to filter_entry_t* to access the data
					filter_entry_t* pfilter_data = data->GetEngineData(engine)->GetFilterData();
					filter_entry_t& filter_entry = pfilter_data[(remap_address / sizeof(filter_entry_t))];

					PopulateFilterEntry(filter_entry, filter_fetch_loc, &bd);

                    mm_level_max = std::max(mm_level_max, bd._mm_level);

#ifdef DEBUG_BLOCK
					if (v == DEBUG_BLOCK_V && h == DEBUG_BLOCK_H)
					{
						printf("\nFilter data:");

						for(int i = 0; i < MAX_BLOCK_SPAN; ++i)
						{
							if(i % BLOCK_GRID_WIDTH == 0)
								printf("\n");
							printf(" %4d", filter_fetch_loc[i]);
						}
						printf("\n");
						printf("MIN V: %d MIN H: %d\n", bd._min_v, bd._min_h);
						printf("MM: %d\n", bd._mm_level);
						printf("LPF V:%d H:%d\n", bd._block_coef[0], bd._block_coef[1]);
						printf("\n");
					}					
#endif /* DEBUG_BLOCK */

					// Make sure the most recent fetch entry "run unil" field is updated
					if(last_fetch_entry)
					{
						auto run_distance = _state[engine]._run_until - _state[engine]._run_until_prev;

						if(run_distance)
						{
							run_distance += (last_fetch_entry->_lower_word & 0xfff);

							static constexpr uint32_t MAX_RUN_WAIT = 0xfff;

							// Insert NOP entries if necessary
							auto run_nops = run_distance / MAX_RUN_WAIT;

							while(run_nops)
							{
								--run_nops;

								last_fetch_entry->_lower_word &= (~0xfff);
								last_fetch_entry->_lower_word |= MAX_RUN_WAIT;
								run_distance -= MAX_RUN_WAIT;

								// Insert new entry
								const uint32_t byte_address = _state[engine]._fetch_rom_address * sizeof(fetch_entry_t);
								const uint32_t remap_address = map_address(byte_address, ctx._hw->_mem_model);
								const uint32_t fetch_entry_idx = remap_address / sizeof(fetch_entry_t);

								if(fetch_entry_idx < data->GetEngineData(engine)->GetFetchEntries())
								{
									fetch_entry_t* p_fetch_entry = data->GetEngineData(engine)->GetFetchData();
									auto& fetch_entry = p_fetch_entry[fetch_entry_idx];

                                    // Clear "wait_until" field otherwise engine stalls
									fetch_entry._lower_word = last_fetch_entry->_lower_word & 0xff000fff;
									fetch_entry._upper_word = last_fetch_entry->_upper_word;

									last_fetch_entry->_upper_word |= 0x80000000;	// NOP
									last_fetch_entry = &(p_fetch_entry[fetch_entry_idx]);
								}

								_state[engine]._fetch_rom_address++;
							}

							last_fetch_entry->_lower_word &= (~0xfff);
							last_fetch_entry->_lower_word |= uint32_t{run_distance & 0xfff};

							_state[engine]._run_until_prev = _state[engine]._run_until;
						}
					}

					filter_rom_address++;
					_state[engine]._run_until++;
				}
			}

			if(megablock_blank)
			{
				uint8_t* megablock_data = data->GetSkipMegablockData();
				megablock_data[ov * SKIP_MEGABLOCK_WIDTH_MAX + oh] = 1;
				filter_rom_address = filter_rom_address_hold;
			}
		}
	}

	ret = FinalizeFetchData(last_fetch_entry, data, engine);

    _state[engine]._mm_level_max = mm_level_max;

	return ret;
}


bool WarpDataGenerator::OutputToInputMapperMesh8x8(const WarpDataContext& ctx, const WarpMesh* mesh, uint32_t v_begin, uint32_t v_end)
{
    const uint32_t mesh_h_nodes = mesh->GetHNodes();
	const uint32_t num_streams = mesh->GetStreams();
	const uint32_t frac_prec = mesh->GetFractBits();

	auto otoi_func = ctx._hw->_output_block_scan ? 
        (ctx._hw->_mipmap_enable ? OtoI_2x3_Db<true> : OtoI_2x3_Db<false>):
        (ctx._hw->_mipmap_enable ? OtoI_2x3_Sb<true> : OtoI_2x3_Sb<false>);        

	for(uint32_t r = v_begin; r < v_end; ++r)
	{
		// 16x8 pixel block requires two 8x8 pixel mesh sections
        for(uint32_t c = 0; c < mesh_h_nodes - 2; c += 2)
		{
			const uint32_t hidx = c * num_streams;

			const mesh_node_t* r0 = mesh->GetRow(r + 0) + hidx;
			const mesh_node_t* r1 = mesh->GetRow(r + 1) + hidx;

			auto& bd = _block_map[r * ctx._hblocks_out + (c >> 1)];

            bool block_ok = std::invoke(otoi_func, ctx, r0, r1, num_streams, frac_prec, bd);

			if(!block_ok)
			{
				INTEL_VVP_WARP_DATA_LOG("Error: OutBlock[%" PRIu32 ",%" PRIu32 "] requires odd layout of input fetch blocks. Required InBlocks:\n", r, c >> 1);
				PrintBlockMapping(bd);
				INTEL_VVP_WARP_DATA_LOG("\n");

				_error_code = static_cast<uint32_t>(ErrorCode::CompressionTooHigh);

				// If mesh errors allowed, mark block as empty and continue
				if(ctx._allow_mesh_errors)
				{
					bd._block_list_size = 0;
					block_ok = true;
				}
				else
					return false;
			}
		}	
	}

	return true;
}


bool WarpDataGenerator::OutputToInputMapperMesh16x16(const WarpDataContext& ctx, const WarpMesh* mesh, uint32_t v_begin, uint32_t v_end)
{
	const uint32_t mesh_h_nodes = mesh->GetHNodes();
	const uint32_t num_streams = mesh->GetStreams();
	const uint32_t frac_prec = mesh->GetFractBits();

    auto otoi_func = ctx._hw->_output_block_scan ? 
        (ctx._hw->_mipmap_enable ? OtoI_2x2_Db<true> : OtoI_2x2_Db<false>):
        (ctx._hw->_mipmap_enable ? OtoI_2x2_Sb<true> : OtoI_2x2_Sb<false>);
   
	auto otoi_block = [&](const intel_vvp_warp::mesh_node_t *r0, const intel_vvp_warp::mesh_node_t *r1, const uint32_t bv, const uint32_t bh)->bool {
		auto& bd = _block_map[bv * ctx._hblocks_out + bh];

        bool top_half = ((bv & 0x1) == 0x0);
        bool ret = std::invoke(otoi_func, ctx, r0, r1, num_streams, frac_prec, bd, top_half);

		if(!ret)
		{
			INTEL_VVP_WARP_DATA_LOG("Error: OutBlock[%" PRIu32 ",%" PRIu32 "] requires odd layout of input fetch blocks. Required InBlocks:\n", bv, bh);
			PrintBlockMapping(bd);
			INTEL_VVP_WARP_DATA_LOG("\n");

			_error_code = static_cast<uint32_t>(ErrorCode::CompressionTooHigh);

			// If mesh errors allowed, mark block as empty and continue
			if(ctx._allow_mesh_errors){
				bd._block_list_size = 0;
				ret = true;
			}
		}

		return ret;
	};

	for(uint32_t r = v_begin; r < v_end; ++r)
	{
        for(uint32_t c = 0; c < mesh_h_nodes - 1; ++c)
		{
			const mesh_node_t* r0 = mesh->GetRow(r + 0) + c * num_streams;
			const mesh_node_t* r1 = mesh->GetRow(r + 1) + c * num_streams;

			// 16x16 pixel mesh section covers two 16x8 pixel blocks
			const uint32_t bv1 = 2 * r;
            const uint32_t bv2 = bv1 + 1;
			const uint32_t bh = c;

			// If the mesh exceeds the output height only proess top half
			bool block_ok = otoi_block(r0, r1, bv1, bh);

            if((bv2) < ctx._vblocks_out)
                block_ok = block_ok && otoi_block(r0, r1, bv2, bh);            

			if(!block_ok)
					return false;
		}
	}

	return true;
}


void WarpDataGenerator::PrintBlockMapping(const block_descriptor_t& bd)
{
	INTEL_VVP_WARP_DATA_LOG("Min:[%i,%i] - Max:[%i,%i]\n", bd._min_v, bd._min_h, bd._max_v, bd._max_h);

	for(uint32_t b = 0; b < bd._block_list_size; ++b)
	{
		const auto& br = bd._block_list[b];
		const uint8_t num_refs = br._ref_end - br._ref_begin;

		if(num_refs)
		{
			INTEL_VVP_WARP_DATA_LOG("%" PRIu32 " [%3i,%3i]: %" PRIu8 " -> ", b, br._v, br._h, num_refs);

			for(uint8_t l = 0; l < num_refs; ++l)
				INTEL_VVP_WARP_DATA_LOG("%" PRIu8 ", ", br._ref_begin + l);

			INTEL_VVP_WARP_DATA_LOG("\n");
		}
	}	
}


// Raster scan implementation

void  WarpDataGenerator::ResetFilterCache(const WarpDataContext& ctx)
{
	_page_filter_cache.clear();

	const fetch_pos_t BLOCK_CACHE_NA = static_cast<fetch_pos_t>(ctx._hw->_block_cache_size_max - 1);
	const uint32_t vo_blocks = ctx._vblocks_out;

	for (std::size_t e = 0; e < ctx._hw->_engines; ++e)
	{
		const uint32_t ho_blocks = ctx._engine_hblocks[e & 0x1];
		const uint32_t filter_entries = vo_blocks * ho_blocks;

		page_filter_data_t filter_data{};
		filter_data._filter_grid.reserve(filter_entries);

		for(uint32_t i = 0; i < filter_entries; ++i)
		{
			filter_data._filter_grid.emplace_back(filter_grid_t{BLOCK_CACHE_NA});
		}

		_page_filter_cache.emplace_back(std::move(filter_data));
	}
}


bool WarpDataGenerator::RunRasterOutputProcess(const WarpDataContext& ctx, uint32_t engine)
{
	bool ret = false;

	const uint32_t rv = _state[engine]._v_process_block;
	const uint32_t rh = _state[engine]._h_process_block;

	// Note we refer to the filter 16x8 block here
	// We want to make sure this output location has been
	// processed so we can reload relevant cache slots if necessary
#ifdef DONT_USE_DIV    
	uint32_t ov = rv >> 3;
#else
    uint32_t ov = rv / ctx._hw->_block_height;
#endif /* DONT_USE_DIV */    

	const uint32_t blocks_per_raster_segment = 8;
	const uint32_t oh_start = rh * blocks_per_raster_segment;
	const uint32_t oh_end = std::min(oh_start + blocks_per_raster_segment, ctx._hblocks_out);

	// Iterate over 16x8 processing blocks
	for(uint32_t oh = oh_start; oh < oh_end; ++oh)
	{
		uint32_t output_block_idx = ov * ctx._hblocks_out + oh;
		const block_descriptor_t& bd = _block_map[output_block_idx];
		const uint8_t mm_level = bd._mm_level;

		const int32_t hblock_in_max = static_cast<int32_t>(ctx._hblocks_in[mm_level]);
		const int32_t vblock_in_max = static_cast<int32_t>(ctx._vblocks_in[mm_level]);		

		for(uint32_t b = 0; b < bd._block_list_size; ++b)
		{
			const auto& br = bd._block_list[b];

			const int16_t iv = br._v;
			const int16_t ih = br._h;

			// Only care about the real blocks (those within the frame boundaries)
			if(ih < 0 || iv < 0 ||ih >= hblock_in_max || iv >= vblock_in_max)
				continue;

#ifdef DONT_USE_DIV            
			const uint8_t block_row = rv & 7;
#else
            const uint8_t block_row = rv % ctx._hw->_block_height;
#endif /* DONT_USE_DIV */            

			if((block_row >= br._ref_begin) && (block_row < br._ref_end))
			{
				fetch_pos_t fetch_idx = _state[engine].GetBlockCacheLocation(mm_level, iv, ih);

#ifdef DEBUG
				// Reduce the use count while checking for internal error
				if (fetch_idx < ctx._hw->_block_cache_size)
				{
					if (_state[engine]._used_count[fetch_idx] > 0)
					{
#endif /* DEBUG */						
						_state[engine]._used_count[fetch_idx]--;

						if (_state[engine]._used_count[fetch_idx] == 0)
						{
							_state[engine].UnloadBlockFromCache(mm_level, iv, ih);
							_state[engine]._fetch_tracker->Release(fetch_idx);
						}
#ifdef DEBUG						
					}
					else
					{
						if (!ctx._allow_mesh_errors)
						{
							INTEL_VVP_WARP_DATA_LOG("Error: Using block which is marked as no longer needed\n");
							return ret;
						}
					}
				}
				else
				{
					if (!ctx._allow_mesh_errors)
					{
						INTEL_VVP_WARP_DATA_LOG("Error: Engine [%" PRIu32 "] unloading InBlock[%i,%i] which is not in fetch - OutBlock[%" PRIu32 ",%" PRIu32 "] requires InBlocks ", engine, iv, ih, ov, oh);
						PrintBlockMapping(bd);
						INTEL_VVP_WARP_DATA_LOG("\n");
						return ret;
					}
				}
#endif /* DEBUG */
			}
		}
	}

	_state[engine]._h_process_block++;

	if(_state[engine]._h_process_block > _state[engine]._h_end)
	{
		_state[engine]._h_process_block = _state[engine]._h_start;
		_state[engine]._v_process_block++;
	}

	const uint32_t engine_raster_hblocks = _state[engine]._h_end - _state[engine]._h_start + 1;
	const uint32_t wait_until = (_state[engine]._v_process_block * engine_raster_hblocks) + (_state[engine]._h_process_block - _state[engine]._h_start);
	const uint32_t wait_run_distance = (wait_until < _state[engine]._run_until) ? _state[engine]._run_until - wait_until : 0;
	_state[engine]._wait_run_min = std::min(_state[engine]._wait_run_min, wait_run_distance);

	if(_state[engine]._wait_run_min > 0)
	{
		_state[engine]._wait_until = wait_until;
		ret = true;

		const uint32_t safe_distance = MIN_WAIT_RUN_DISTANCE * (_state[engine]._h_end - _state[engine]._h_start + 1);

		if(_state[engine]._wait_run_min < safe_distance)
			_error_code = static_cast<uint32_t>(ErrorCode::BlockCacheUsageHigh);
	}
	else
	{
		INTEL_VVP_WARP_DATA_LOG("Error: Engine [%" PRIu32 "] WU - RU distance: %" PRIu32 "\n", engine, _state[engine]._wait_run_min);
	}

	return ret;
}


// Horizontal start, end and limit params are in raster segments
template<bool MIPMAP_ENABLE>
bool WarpDataGenerator::BlockCacheWrapperRaster(const WarpDataContext& ctx, const uint32_t engine, WarpDataImplPtr data)
{
	bool done = false;

	assert(ctx._hw->_block_cache_size <= ctx._hw->_block_cache_size_max);

	const uint32_t BLOCK_CACHE_NA = ctx._hw->_block_cache_size_max - 1;
	const uint32_t BLOCK_CACHE_SIZE = (ctx._hw->_block_cache_size < BLOCK_CACHE_NA ? ctx._hw->_block_cache_size : BLOCK_CACHE_NA - 1);

	const uint32_t blocks_per_raster_segment = 8;

	const uint32_t rh_start = _state[engine]._h_start;
	const uint32_t rh_end = _state[engine]._h_end + 1;
	const uint32_t engine_bh_start = rh_start * blocks_per_raster_segment;
	const uint32_t engine_h_blocks = ctx._engine_hblocks[engine & 0x1];

	const uint32_t rv_max = ctx._vblocks_out * ctx._hw->_block_height;

	fetch_entry_t* last_fetch_entry{nullptr};

    uint8_t mm_level_max = 0;

    uint8_t* input_skip_ram_data = data->GetSkipRamInputData();

	for(uint32_t rv = 0; rv < rv_max; ++rv)
	{
		for(uint32_t rh = rh_start; rh < rh_end; ++rh)
		{
			const uint32_t bh_start = rh * blocks_per_raster_segment;
			const uint32_t bh_end = std::min(bh_start + blocks_per_raster_segment, ctx._hblocks_out);

			// Try place all required blocks into cache
			for(uint32_t bh = bh_start; bh < bh_end; ++bh)
			{
#ifdef DONT_USE_DIV                
				const uint32_t bv = rv >> 3;
#else
                const uint32_t bv = rv / ctx._hw->_block_height;
#endif /* DONT_USE_DIV */                

				block_descriptor_t& bd = _block_map[bv * ctx._hblocks_out + bh];
				const uint8_t mm_level = bd._mm_level;

				const int32_t hblock_in_max  = static_cast<int32_t>(ctx._hblocks_in[mm_level]);
				const int32_t vblock_in_max = static_cast<int32_t>(ctx._vblocks_in[mm_level]);				

				for(uint32_t b = 0; b < bd._block_list_size; ++b)
				{
					auto& br = bd._block_list[b];

					// If the input block is required in the current output line
#ifdef DONT_USE_DIV
                    const uint32_t block_row = rv & 7;
#else
                    const uint32_t block_row = rv % ctx._hw->_block_height;
#endif /* DONT_USE_DIV */

					if((block_row >= br._ref_begin) && (block_row < br._ref_end))
					{
						const int16_t iv = br._v;
						const int16_t ih = br._h;

						// Only care about the real blocks (those within the frame boundaries)
						if(ih < 0 || iv < 0 ||ih >= hblock_in_max || iv >= vblock_in_max)
							continue;

						// If block already in cache - reuse it
						uint32_t fetch_idx = _state[engine].GetBlockCacheLocation(mm_level, iv, ih);

						// Otherwise find available cache location
						while(fetch_idx >= BLOCK_CACHE_SIZE)
						{
							fetch_idx = _state[engine]._fetch_tracker->Get();

							// If no available slots in cache try freeing some by running output process
							if (fetch_idx == BLOCK_CACHE_SIZE)
							{
								bool output_process_ok = RunRasterOutputProcess(ctx, engine);

								if(!output_process_ok)
									break;

								fetch_idx = _state[engine]._fetch_tracker->Get();
							}

							if (fetch_idx < BLOCK_CACHE_SIZE)
							{
								_state[engine].LoadBlockToCache(mm_level, iv, ih, fetch_idx);

								// Insert NOP entries first if necessary
								InsertFetchNopEntries(engine, data, ctx._hw->_mem_model, &last_fetch_entry);

								const uint32_t byte_address = _state[engine]._fetch_rom_address * sizeof(fetch_entry_t);
								const uint32_t remap_address = map_address(byte_address, ctx._hw->_mem_model);
								const uint32_t fetch_entry_idx = remap_address / sizeof(fetch_entry_t);

								if(fetch_entry_idx < data->GetEngineData(engine)->GetFetchEntries())
								{
									fetch_entry_t* fetch_entry = data->GetEngineData(engine)->GetFetchData();

									PopulateFetchEntry(fetch_entry[fetch_entry_idx], engine, fetch_idx, iv, ih, mm_level);

									last_fetch_entry = &(fetch_entry[fetch_entry_idx]);
								}

								_state[engine]._fetch_rom_address++;

#ifdef DEBUG_BLOCK_LOAD
								_state[engine]._load_count[iv][ih]++;
#endif /*DEBUG_BLOCK_LOAD*/
							}
						}

						if (fetch_idx < BLOCK_CACHE_SIZE)
						{
							// When input block is required by an output block for the first time
							// we increase the usage counter by the total number of references.
							// This is to make sure the input block is kept in cache until
							// all relevant lines of the output block have been drawn
							// and not reloaded into a different cache location half way through
							if(br._in_cache == 0)
							{
								_state[engine]._used_count[fetch_idx] += (br._ref_end - br._ref_begin);

                                auto& filter_grid = _page_filter_cache[engine]._filter_grid[bv * engine_h_blocks + (bh - engine_bh_start)];
                                auto& fetch_loc = filter_grid._fetch_loc;
                                const uint32_t grid_v = iv - bd._min_v;
                                const uint32_t grid_h = ih - bd._min_h;
                                const uint32_t filter_grid_idx = grid_v * BLOCK_GRID_WIDTH + grid_h;
                                fetch_loc[filter_grid_idx] = fetch_idx;

                                // Update R2B skip RAM
                                if constexpr (MIPMAP_ENABLE)
                                    UpdateInputSkipRam(iv, ih, mm_level, input_skip_ram_data);

                                br._in_cache = 1;
                            }
						}
						else
						{
							INTEL_VVP_WARP_DATA_LOG("Error: Engine [%" PRIu32 "] InBlock[%i][%i,%i] required by OutBlock[%" PRIu32 ",%" PRIu32 "]: Unable to load into cache!\n", engine, mm_level, iv, ih, bv, bh);
							_error_code = static_cast<uint32_t>(ErrorCode::BlockCacheFull);
							return done;
						}
					}
				}
			}

			// Make sure the most recent fetch entry "run unil" field is updated
			if(last_fetch_entry)
			{
				auto run_distance = _state[engine]._run_until - _state[engine]._run_until_prev;

				if(run_distance)
				{
					run_distance += (last_fetch_entry->_lower_word & 0xfff);

					static constexpr uint32_t MAX_RUN_WAIT = 0xfff;

					// Insert NOP entries if necessary
					auto run_nops = run_distance / MAX_RUN_WAIT;

					while(run_nops)
					{
						--run_nops;

						last_fetch_entry->_lower_word &= (~0xfff);
						last_fetch_entry->_lower_word |= MAX_RUN_WAIT;
						run_distance -= MAX_RUN_WAIT;

						// Insert new entry
						const uint32_t byte_address = _state[engine]._fetch_rom_address * sizeof(fetch_entry_t);
						const uint32_t remap_address = map_address(byte_address, ctx._hw->_mem_model);
						const uint32_t fetch_entry_idx = remap_address / sizeof(fetch_entry_t);

						if(fetch_entry_idx < data->GetEngineData(engine)->GetFetchEntries())
						{
							fetch_entry_t* p_fetch_entry = data->GetEngineData(engine)->GetFetchData();
							auto& fetch_entry = p_fetch_entry[fetch_entry_idx];

							fetch_entry._lower_word = last_fetch_entry->_lower_word;
							fetch_entry._upper_word = last_fetch_entry->_upper_word;

							last_fetch_entry->_upper_word |= 0x80000000;	// NOP
							last_fetch_entry = &(p_fetch_entry[fetch_entry_idx]);
						}

						_state[engine]._fetch_rom_address++;
					}

					last_fetch_entry->_lower_word &= (~0xfff);
					last_fetch_entry->_lower_word |= uint32_t{run_distance & 0xfff};

					_state[engine]._run_until_prev = _state[engine]._run_until;
				}
			}

			_state[engine]._run_until++;
		}
	}

	done = FinalizeFetchData(last_fetch_entry, data, engine);

	if(done)
	{
		// Generate Filter data
		const uint32_t oh_start = engine_bh_start;
		const uint32_t oh_end = std::min(oh_start + engine_h_blocks, ctx._hblocks_out);
		const uint32_t hw_engine_hblocks = roundup((oh_end - oh_start), blocks_per_raster_segment);

		filter_entry_t* pfilter_data = data->GetEngineData(engine)->GetFilterData();

		for(uint32_t ov = 0; ov < ctx._vblocks_out; ++ov)
		{
			uint32_t block_idx = ov * engine_h_blocks;   // Software filter data tables have the same stride for simplicity
			uint32_t filter_rom_idx = ov * hw_engine_hblocks;

			for(uint32_t oh = oh_start; oh < oh_end; ++oh)
			{
				uint32_t byte_address = filter_rom_idx * sizeof(filter_entry_t);
				uint32_t remap_address  = map_address(byte_address, ctx._hw->_mem_model);

				// Note: remap_address accepts and returns byte offsets
				// Convert it to filter_entry_t* to access the data

				filter_entry_t& filter_entry = pfilter_data[(remap_address / sizeof(filter_entry_t))];

				const auto& filter_fetch_loc = _page_filter_cache[engine]._filter_grid[block_idx]._fetch_loc;

                const block_descriptor_t& bd = _block_map[ov * ctx._hblocks_out + oh];

				PopulateFilterEntry(filter_entry, filter_fetch_loc, &bd);

                mm_level_max = std::max(mm_level_max, bd._mm_level);

#ifdef DEBUG_BLOCK
				if (ov == DEBUG_BLOCK_V && oh == DEBUG_BLOCK_H)
				{
					printf("\nFilter data:");

					for(int i = 0; i < MAX_BLOCK_SPAN; ++i)
					{
						if(i % BLOCK_GRID_WIDTH == 0)
							printf("\n");
						printf(" %4d", filter_fetch_loc[i]);
					}
					printf("\n");
					printf("MIN V: %d MIN H: %d\n", bd._min_v, bd._min_h);
					printf("MM: %d\n", bd._mm_level);
					printf("LPF V:%d H:%d\n", bd._block_coef[0], bd._block_coef[1]);
					printf("\n");
				}
#endif /* DEBUG_BLOCK */				

				++block_idx;
				++filter_rom_idx;
			}
		}
	}

    _state[engine]._mm_level_max = mm_level_max;

	INTEL_VVP_WARP_DATA_LOG("Eng %" PRIu32 " Min wait - run distance: %" PRIu32 "\n", engine, _state[engine]._wait_run_min);

	return done;
}


bool WarpDataGenerator::GenerateFetchAndFilterDataRaster(const WarpDataContext& ctx, const WarpMesh* mesh, WarpDataImplPtr data)
{
	bool done = false;

	ResetBlockMap(ctx._vblocks_out * ctx._hblocks_out);
	ResetEngineState(ctx);
	ResetFilterCache(ctx);

	// For each output block work out which input blocks to fetch

    const uint32_t mesh_step = mesh->GetStep();

	auto otoi_mapper_func = (mesh_step == 16) ? &WarpDataGenerator::OutputToInputMapperMesh16x16 : &WarpDataGenerator::OutputToInputMapperMesh8x8;	

#ifndef ALT_SINGLE_THREADED
	// If available - process in parallel in sections of approx 64 vertical blocks
	const uint32_t hw_concurr = std::min(static_cast<uint32_t>(std::thread::hardware_concurrency()), (ctx._vblocks_out >> 6));

	if(hw_concurr > 1)
	{
        std::vector<std::future<bool>> f(hw_concurr - 1);
        const uint32_t thread_v_step = mesh->GetVNodes() / hw_concurr;
        uint32_t thread_v_begin = 0;

        for(uint32_t i = 0; i < (hw_concurr - 1); ++i, thread_v_begin += thread_v_step)
        {
            f[i] = std::async(std::launch::async, otoi_mapper_func, this, std::cref(ctx), mesh, thread_v_begin, thread_v_begin + thread_v_step);
        }

		done = std::invoke(otoi_mapper_func, this, ctx, mesh, thread_v_begin, mesh->GetVNodes()-1);

		for(uint32_t i = 0; i < (hw_concurr - 1); ++i) done = done && f[i].get();
	}
	else
#endif /* ALT_SINGLE_THREADED */
		done = std::invoke(otoi_mapper_func, this, ctx, mesh, 0, mesh->GetVNodes()-1);

	if(done)
	{
		done = false;
		const uint32_t engines = ctx._hw->_engines;

		auto block_cache_func = [&](const uint32_t eng_begin, const uint32_t eng_end)->bool
		{
			bool ret = true;

			for(uint32_t e = eng_begin; e < eng_end; ++e)
			{
                if(ctx._hw->_mipmap_enable)
                    ret = ret && BlockCacheWrapperRaster<true>(ctx, e, data);
                else
                    ret = ret && BlockCacheWrapperRaster<false>(ctx, e, data);
			}

			return ret;
		};

#ifndef ALT_SINGLE_THREADED
		const uint32_t hw_concurr = std::min(static_cast<uint32_t>(std::thread::hardware_concurrency()), engines);

		if(hw_concurr > 1)
		{
			std::vector<std::future<bool>> f(hw_concurr - 1);
			const uint32_t engines_per_thread = engines / hw_concurr;
			uint32_t eng_begin = 0;

			for(uint32_t i = 0; i < (hw_concurr - 1); ++i, eng_begin += engines_per_thread)
				f[i] = std::async(std::launch::async, block_cache_func, eng_begin, eng_begin + engines_per_thread);

			done = block_cache_func(eng_begin, engines);

			for(uint32_t i = 0; i < (hw_concurr - 1); ++i) done = done && f[i].get();
		}
		else
#endif /* ALT_SINGLE_THREADED */
			done = block_cache_func(0, engines);

#ifdef DEBUG_BLOCK_LOAD
		INTEL_VVP_WARP_DATA_LOG("CACHE USAGE\n");

		for(uint32_t engine=0; engine < engines; ++engine)
		{
			INTEL_VVP_WARP_DATA_LOG("# ENGINE: %d\n", engine);

			for(int v = 0; v < 270; ++v)
			{
				for(int h = 0; h < 240; ++h)
				{
					if(_state[engine]._load_count[v][h] > 1)
						INTEL_VVP_WARP_DATA_LOG("[%d, %d] -> %d\n", v, h, _state[engine]._load_count[v][h]);
				}

				INTEL_VVP_WARP_DATA_LOG("\n");
			}
		}
#endif /*DEBUG_BLOCK_LOAD*/
	}

	return done;
}


} //namespace intel_vvp_warp
