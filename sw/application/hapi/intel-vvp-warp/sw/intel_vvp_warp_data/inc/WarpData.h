/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#ifndef _WARPDATA_H_
#define _WARPDATA_H_


#include <stdint.h>
#include <memory>


namespace intel_vvp_warp
{

typedef uint32_t mesh_entry_t;


struct filter_entry_t
{
	uint32_t _word[8];
};


struct fetch_entry_t
{
	uint32_t _lower_word;
	uint32_t _upper_word;
};


class WarpEngineData
{
public:
	virtual ~WarpEngineData() {};

	virtual uint32_t GetMeshEntries() const = 0;
	virtual uint32_t GetFilterEntries() const = 0;
	virtual uint32_t GetFetchEntries() const = 0;

	virtual const mesh_entry_t* GetMeshData() const = 0;
	virtual const filter_entry_t* GetFilterData() const = 0;
	virtual const fetch_entry_t* GetFetchData() const = 0;
};

class WarpData
{
public:
	virtual ~WarpData() {};

	virtual uint32_t GetEngines() const = 0;
    virtual const uint8_t* GetSkipRamInputData() const = 0;
	virtual const uint8_t* GetSkipMegablockData() const = 0;
	virtual const WarpEngineData* GetEngineData(const uint32_t engine) const = 0;
	virtual uint32_t GetLatencyLines() const = 0;
    virtual uint32_t GetMipmapLevel() const = 0;
    virtual uint32_t GetMeshStep() const = 0;
};

using WarpDataPtr = std::shared_ptr<WarpData>;


struct WarpLatencyParams
{
	uint32_t _output_latency;
	uint32_t _total_latency;
};

} //namespace intel_vvp_warp

#endif /* _WARPDATA_H_ */
