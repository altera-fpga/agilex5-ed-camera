/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef _WARPDATAIMPL_H_
#define _WARPDATAIMPL_H_


#include "WarpData.h"
#include "WarpMem.h"


#define SKIP_MEGABLOCK_WIDTH_MAX		(64)
#define SKIP_MEGABLOCK_HEIGHT_MAX		(128)


namespace intel_vvp_warp
{

class WarpEngineDataImpl : public WarpEngineData
{
public:
	WarpEngineDataImpl(const uint32_t mesh_entries, const uint32_t filter_entries, const uint32_t fetch_entries):
		_mesh_data_storage{allocate_warp_memory<mesh_entry_t>(mesh_entries)},
		_filter_data_storage{allocate_warp_memory<filter_entry_t>(filter_entries)},
		_fetch_data_storage{allocate_warp_memory<fetch_entry_t>(fetch_entries)},
		_mesh_entries{mesh_entries},
		_filter_entries{filter_entries},
		_fetch_entries{fetch_entries},
		_fetch_entries_act{_fetch_entries}
	{}

	virtual ~WarpEngineDataImpl() {};

	virtual uint32_t GetMeshEntries() const override
	{
		return _mesh_entries;
	}

	virtual uint32_t GetFilterEntries() const override
	{
		return _filter_entries;
	}

	virtual uint32_t GetFetchEntries() const override
	{
		return std::min(_fetch_entries, _fetch_entries_act);
	}

	virtual const mesh_entry_t* GetMeshData() const override
	{
		return _mesh_data_storage.get();
	}

	virtual const filter_entry_t* GetFilterData() const override
	{
		return _filter_data_storage.get();
	}

	virtual const fetch_entry_t* GetFetchData() const override
	{
		return _fetch_data_storage.get();
	}

	// Non-const overloads required to fill the data in
	// Implement through their const variants
	mesh_entry_t* GetMeshData()
	{
		return const_cast<mesh_entry_t*>(
		           static_cast<const WarpEngineDataImpl&>(*this).GetMeshData()
		       );
	}

	filter_entry_t* GetFilterData()
	{
		return const_cast<filter_entry_t*>(
		           static_cast<const WarpEngineDataImpl&>(*this).GetFilterData()
		       );
	}

	fetch_entry_t* GetFetchData()
	{
		return const_cast<fetch_entry_t*>(
		           static_cast<const WarpEngineDataImpl&>(*this).GetFetchData()
		       );
	}

	void SetFetchEntries(const uint32_t fetch_entries)
	{
		_fetch_entries_act =  std::min(fetch_entries, _fetch_entries);
	}

private:
	warp_mem_ptr_t<mesh_entry_t> _mesh_data_storage;
	warp_mem_ptr_t<filter_entry_t> _filter_data_storage;
	warp_mem_ptr_t<fetch_entry_t> _fetch_data_storage;

	uint32_t _mesh_entries;
	uint32_t _filter_entries;
	uint32_t _fetch_entries;

	// Actual number of fetch entries
	// Has to be explicitly set once calculated
	uint32_t _fetch_entries_act;
};


class WarpDataImpl : public WarpData
{
public:
	static constexpr uint32_t MAX_ENGINES = 4;

	WarpDataImpl():
        _skip_ram_input_data_storage{allocate_warp_memory<uint8_t>(0)},
        _skip_megablock_data_storage{allocate_warp_memory<uint8_t>(0)},
		_engines{0},
		_engine_data{nullptr},
		_latency_lines{0},
        _mipmap_level{0},
        _mesh_step{8}
	{            
	}

	WarpDataImpl(uint32_t engines, uint32_t mesh_entries, uint32_t filter_entries, uint32_t fetch_entries):
        _skip_ram_input_data_storage{allocate_warp_memory<uint8_t>(SKIP_MEGABLOCK_WIDTH_MAX * SKIP_MEGABLOCK_HEIGHT_MAX * 2)},  // 1st page - original input; 2nd page - mipmaps
        _skip_megablock_data_storage{allocate_warp_memory<uint8_t>(SKIP_MEGABLOCK_WIDTH_MAX * SKIP_MEGABLOCK_HEIGHT_MAX)},
		_engines(engines < MAX_ENGINES ? engines : MAX_ENGINES),
		_engine_data{nullptr},
		_latency_lines{0},
        _mipmap_level{0},
        _mesh_step{8}
	{
        // Input skip RAM pre-initialized with 1s, output with 0s
        std::fill(_skip_ram_input_data_storage.get(), _skip_ram_input_data_storage.get() + SKIP_MEGABLOCK_WIDTH_MAX * SKIP_MEGABLOCK_HEIGHT_MAX * 2, 0x1);
        std::fill(_skip_megablock_data_storage.get(), _skip_megablock_data_storage.get() + SKIP_MEGABLOCK_WIDTH_MAX * SKIP_MEGABLOCK_HEIGHT_MAX, 0x0);

		for(uint32_t i = 0; i < _engines; ++i)
			_engine_data[i] = std::make_shared<WarpEngineDataImpl>(mesh_entries, filter_entries, fetch_entries);
	}

	virtual ~WarpDataImpl() {};

    bool AppendEngineData(std::shared_ptr<WarpEngineDataImpl> engine_data)
    {
        bool ret = false;

        if(_engines < MAX_ENGINES)
        {
            _engine_data[_engines] = engine_data;
            ++_engines;
            ret = true;
        }

        return ret;
    }

	virtual uint32_t GetEngines() const override
	{
		return _engines;
	}

    virtual const uint8_t* GetSkipRamInputData() const override
    {
        return _skip_ram_input_data_storage.get();
    }

	virtual const uint8_t* GetSkipMegablockData() const override
	{
        return _skip_megablock_data_storage.get();
	}

	virtual const WarpEngineData* GetEngineData(const uint32_t engine) const override
	{
		return (engine < _engines ? _engine_data[engine].get() : nullptr);
	}

	virtual uint32_t GetLatencyLines() const override
	{
		return _latency_lines;
	}

	void SetLatencyLines(const uint32_t latency_lines)
	{
		_latency_lines = latency_lines;
	}

    virtual uint32_t GetMipmapLevel() const override
    {
        return _mipmap_level;
    }

	void SetMipmapLevel(const uint32_t mipmap_level)
	{
		_mipmap_level = mipmap_level;
	}

    virtual uint32_t GetMeshStep() const override
    {
        return _mesh_step;
    }

    void SetMeshStep(const uint32_t mesh_step)
    {
        _mesh_step = mesh_step;
    }     

	// Non-const overloads required to fill the data in
	// Implement through their const variants
	WarpEngineDataImpl* GetEngineData(const uint32_t engine)
	{
		return dynamic_cast<WarpEngineDataImpl*>(
		           const_cast<WarpEngineData*>(
		               static_cast<const WarpDataImpl&>(*this).GetEngineData(engine)
		           )
		       );
	}

	uint8_t* GetSkipRamInputData()
	{
		return const_cast<uint8_t*>(
		           static_cast<const WarpDataImpl&>(*this).GetSkipRamInputData()
		       );
	}

	uint8_t* GetSkipMegablockData()
	{
		return const_cast<uint8_t*>(
		           static_cast<const WarpDataImpl&>(*this).GetSkipMegablockData()
		       );
	}

private:
    warp_mem_ptr_t<uint8_t> _skip_ram_input_data_storage;
    warp_mem_ptr_t<uint8_t> _skip_megablock_data_storage;
	uint32_t _engines;
	std::shared_ptr<WarpEngineDataImpl> _engine_data[MAX_ENGINES];
	uint32_t _latency_lines;
    uint32_t _mipmap_level;
    uint32_t _mesh_step;
};


using WarpDataImplPtr = std::shared_ptr<WarpDataImpl>;

} //namespace intel_vvp_warp

#endif /*_WARPDATAIMPL_H_*/