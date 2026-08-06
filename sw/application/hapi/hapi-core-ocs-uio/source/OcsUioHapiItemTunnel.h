/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "HapiItem.h"
#include "OcsUioHapiItem.h"

namespace Hapi
{
    class OcsUioHapiItemTunnel : public HapiItemTunnel
    {
    public:
        uint32_t ReadRegister(uint32_t registerIndex) override;
        void WriteRegister(uint32_t registerIndex, uint32_t value) override;

        std::shared_ptr<OcsUioHapiItem> _spOcsUioHapiItem;
    };
}