/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "OcsUioHapiCore.h"
#include "OcsUioHapiItemTunnel.h"

namespace Hapi
{
    uint32_t OcsUioHapiItemTunnel::ReadRegister(uint32_t registerIndex)
    {
        uint32_t value = 0;
        if (_spOcsUioHapiItem)
            value = _spOcsUioHapiItem->ReadRegister(registerIndex);

        return value;
    }

    void OcsUioHapiItemTunnel::WriteRegister(uint32_t registerIndex, uint32_t value)
    {
        if (_spOcsUioHapiItem)
            _spOcsUioHapiItem->WriteRegister(registerIndex, value);
    }
} // namespace Hapi