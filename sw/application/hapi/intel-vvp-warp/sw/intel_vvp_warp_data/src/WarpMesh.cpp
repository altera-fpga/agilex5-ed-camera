/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "WarpMesh.h"
#include "WarpDataUtils.h"
#include <iostream>

#ifdef INTEL_VVP_WARP_DATA_ENABLE_LOGGING
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stdio.h>
#define INTEL_VVP_WARP_MESH_LOG(...)	printf(__VA_ARGS__)
#else
#define INTEL_VVP_WARP_MESH_LOG(...)
#endif /* INTEL_VVP_WARP_DATA_ENABLE_LOGGING */

namespace intel_vvp_warp
{

WarpMeshPtr WarpMesh::Create(const resolution_t& input, const resolution_t& output, WarpHwContextPtr hw_ctx, uint32_t streams)
{
    WarpMeshPtr mesh{nullptr};

    uint32_t max_dim = max_frame_dim(hw_ctx->_mem_map);

    const uint32_t max_width = std::max(input.first, output.first);
    const uint32_t max_height = std::max(input.second, output.second);

    if((max_width > max_dim) || (max_height > max_dim))
    {
        std::cerr << "Unable to create warp mesh: maximum resolution exceeded!\n";
        std::cerr << "Requested: " << max_width << "x" << max_height << "\n";
        std::cerr << "Supported: " << max_dim << "x" << max_dim << "\n";
    }
    else
    {
        const uint32_t step = ((input.first > 3840) || (input.second > 3840)) ? _MESH_STEP_16 : _MESH_STEP_8;
        const uint32_t fract_bits = (step == _MESH_STEP_16 ? 3 : 4);
        const uint32_t v_nodes(roundup(roundup(output.second, hw_ctx->_block_height), step) / step + 1);
	    const uint32_t h_nodes(roundup(roundup(output.first, hw_ctx->_block_width), step) / step + 1);

        mesh = std::shared_ptr<WarpMesh>{new WarpMesh(v_nodes, h_nodes, step, fract_bits, streams)};
    }

    return mesh;
}


WarpMesh::WarpMesh(uint32_t v, uint32_t h, uint32_t step, uint32_t fract_bits, uint32_t streams):
	_v_nodes(v),
	_h_nodes(h),
    _step(step),
    _fract_bits(fract_bits),
    _streams(streams),
	_mesh{allocate_warp_memory<mesh_node_t>(_v_nodes* _h_nodes * _streams)}
{
}


uint32_t WarpMesh::GetStep() const
{
	return _step;
}


uint32_t WarpMesh::GetFractBits() const
{
    return _fract_bits;
}


uint32_t WarpMesh::GetVNodes() const
{
	return _v_nodes;
}


uint32_t WarpMesh::GetHNodes() const
{
	return _h_nodes;
}


uint32_t WarpMesh::GetStreams() const
{
	return _streams;
}


const mesh_node_t* WarpMesh::GetRow(const uint32_t v) const
{
	return _mesh.get() + v * _h_nodes;
}


mesh_node_t* WarpMesh::GetRow(const uint32_t v)
{
	return const_cast<mesh_node_t*>(
        static_cast<const WarpMesh&>(*this).GetRow(v)
    );
}


WarpMeshPtr WarpMesh::Combine(std::initializer_list<const WarpMesh*> mesh_streams)
{
	WarpMeshPtr mesh{nullptr};

	if(!std::empty(mesh_streams))
	{
		auto ms = std::data(mesh_streams);
		const auto ms0 = ms[0];

		if(ms0)
		{
			uint32_t total_streams = (ms0->GetStreams());

			for(uint32_t i = 1; i < mesh_streams.size(); ++i)
			{
				bool params_match = 
					(ms[i] != nullptr) &&
					(ms[i]->GetVNodes() == ms0->GetVNodes()) &&
					(ms[i]->GetHNodes() == ms0->GetHNodes()) &&
					(ms[i]->GetStep() == ms0->GetStep()) &&
					(ms[i]->GetFractBits() == ms0->GetFractBits());

				if(params_match)
					total_streams += (ms[i]->GetStreams());
				else
				{
					total_streams = 0;
					break;
				}
			}

			if(total_streams)
			{
				mesh = std::shared_ptr<WarpMesh>{new WarpMesh(ms0->GetVNodes(), ms0->GetHNodes(), ms0->GetStep(), ms0->GetFractBits(), total_streams)};

                for(uint32_t v = 0; v < ms0->GetVNodes(); ++v)
                {
                    mesh_node_t* dst = mesh->GetRow(v);

                    for(uint32_t h = 0; h < ms0->GetHNodes(); ++h)
                    {
                        for(uint32_t i = 0; i < mesh_streams.size(); ++i)
                        {
                            const uint32_t num_src_streams = ms[i]->GetStreams();
                            const mesh_node_t* src = ms[i]->GetRow(v) + h * num_src_streams;

                            for(uint32_t s = 0; s < num_src_streams; ++s)
                            {
                                *dst++ = *src++;
                            }
                        }
                    }
                }
			}			
		}
	}

	return mesh;
}

} // intel_vvp_warp