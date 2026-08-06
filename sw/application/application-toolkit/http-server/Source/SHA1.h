/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <functional>
#include <vector>
#include <stdint.h>
#include <string>

using LogicalFunction = std::function<uint32_t(uint32_t B, uint32_t C, uint32_t D)>;

class SHA1Hash
{
public:
    operator uint8_t*() { return _data; }
    uint8_t _data[20] = {};
};

class SHA1
{
public:
    SHA1();
    SHA1Hash CalculateHash(const std::string& inputValue);

private:
    uint32_t RotateLeft(uint32_t value, uint32_t nBits)
    {
        return (value << nBits) | (value >> (32 - nBits));
    }

    std::vector<LogicalFunction> _logicalFunctions;
    std::vector<uint32_t> _constants;
};