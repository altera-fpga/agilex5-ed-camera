/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#ifndef _WARPMEM_H_
#define _WARPMEM_H_


#include <memory>
#include <cstdlib>


namespace intel_vvp_warp
{

template<typename T>
using warp_mem_ptr_t = std::unique_ptr<T, void(*)(T*)>;

template<typename T>
warp_mem_ptr_t<T> allocate_warp_memory(const std::size_t num_entries)
{
    // If MSGDMA is used for data transfer the buffer has to be 1Kb alignment (assuming 512 bit access, 16 beat bursts)
    static constexpr std::size_t MSGDMA_BUFFER_ALIGNMENT = 1024;

#ifdef _MSC_VER
    return warp_mem_ptr_t<T>{(T*)(_aligned_malloc(num_entries * sizeof(T), MSGDMA_BUFFER_ALIGNMENT)), [](T* p) {_aligned_free(p); }};
#else
    return warp_mem_ptr_t<T>{(T*)(::aligned_alloc(MSGDMA_BUFFER_ALIGNMENT, num_entries * sizeof(T))), [](T* p){free(p);}};
#endif /* _MSC_VER */
}

}
#endif /*_WARP_MEM_H_*/