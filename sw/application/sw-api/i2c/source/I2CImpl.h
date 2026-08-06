/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/
#pragma once

#include "I2C.h"

namespace SwApi
{

class LinuxI2CBusEnumerator : public II2CBusEnumerator
{
public:
    std::vector<I2CBusInfo> EnumerateBuses() const override;
};


class I2CBusIo : public II2CBusIo
{
public:
    I2CBusIo(uint32_t id);
    I2CBusIo(const I2CBusIo& other) = delete;
    ~I2CBusIo() override;

    bool Read(uint16_t chipAddress, uint8_t* data, uint16_t size) override;
    bool Write(uint16_t chipAddress, const uint8_t* data, uint16_t size) override;
    uint32_t GetID() const override { return _id; }

private:
    I2CBusIo& operator=(const I2CBusIo& other) = delete;
    bool IsReady() const { return _fd >= 0; }
    uint32_t _id;
    int _fd;
};

} // namespace SwApi