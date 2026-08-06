/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <cstdint>
#include <cstdlib>
#include <memory>


namespace SwUtils
{

struct mem_ptr_deleter_t
{
        void operator()(uint8_t* p) const {free(p);}
};

using mem_ptr_t = std::unique_ptr<uint8_t, mem_ptr_deleter_t>;

struct mem_buffer_t
{
    mem_buffer_t():
        _data{nullptr},
        _size{0}
    {
    }

    mem_buffer_t(const std::size_t size):
        _data{(uint8_t*)malloc(size)},
        _size{0}
    {
        if(_data)
            _size = size;
    }

    mem_buffer_t(const std::size_t size, const std::size_t alignment):
        _data{(uint8_t*)aligned_alloc(alignment, size)},
        _size{0}
    {
        if(_data)
            _size = size;
    }

    

    const uint8_t* data() const
    {
        return _data.get();
    }

    uint8_t* data()
    {
        return const_cast<uint8_t*>(const_cast<const mem_buffer_t&>(*this).data());
    }

    std::size_t size() const
    {
        return _size;
    }

    mem_ptr_t _data;
    std::size_t _size;
};

} // namespace SwUtils