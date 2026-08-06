/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#ifndef _WARPMESH_H_
#define _WARPMESH_H_


#include <stdint.h>
#include <memory>
#include <vector>
#include "WarpDataContext.h"
#include "WarpMem.h"

namespace intel_vvp_warp
{

typedef struct mesh_node
{
	int32_t _x;
	int32_t _y;
} mesh_node_t;


class WarpMesh;
using WarpMeshPtr = std::shared_ptr<WarpMesh>;

using resolution_t = std::pair<uint32_t, uint32_t>;


class WarpMesh
{
public:
    static WarpMeshPtr Create(const resolution_t& input, const resolution_t& output, WarpHwContextPtr hw_ctx, uint32_t streams = 1);
	static WarpMeshPtr Combine(std::initializer_list<const WarpMesh*> mesh_streams);

	uint32_t GetStep() const;
    uint32_t GetFractBits() const;
	uint32_t GetVNodes() const;
	uint32_t GetHNodes() const;
    uint32_t GetStreams() const;
	const mesh_node_t* GetRow (const uint32_t v) const;
    mesh_node_t* GetRow(const uint32_t v);

private:
    WarpMesh(uint32_t v, uint32_t h, uint32_t step, uint32_t fract_bits, uint32_t streams);

    static constexpr uint32_t _MESH_STEP_8 = 8;
    static constexpr uint32_t _MESH_STEP_16 = 16;

	uint32_t _v_nodes;
	uint32_t _h_nodes;
	uint32_t _step;
    uint32_t _fract_bits;
    uint32_t _streams;

	warp_mem_ptr_t<mesh_node_t> _mesh;
};

} // namespace intel_vvp_warp

#endif /* _WARPMESH_H_ */
