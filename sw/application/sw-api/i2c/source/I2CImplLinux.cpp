/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <filesystem>

#include "I2CImpl.h"
#include "SwUtils.h"


namespace SwApi
{

std::vector<I2CBusInfo> LinuxI2CBusEnumerator::EnumerateBuses() const
{
    std::vector<I2CBusInfo> busInfos;
    for (uint32_t i = 0; i < 0xff; i++)
    {
        std::string filename = SwUtils::FormatString("/dev/i2c-%d", i);
        std::filesystem::path i2cBusFilename(filename.c_str());
        if (not std::filesystem::exists(i2cBusFilename))
            break;

        std::string sysFilename = SwUtils::FormatString("/sys/class/i2c-dev/i2c-%d/name", i);
        std::filesystem::path i2cSysFilename = sysFilename;
        std::string sysClassName;
        if (std::filesystem::exists(i2cSysFilename))
        {
            std::shared_ptr<std::vector<std::string>> spStrings = SwUtils::IFile::LoadStrings(i2cSysFilename);
            if (spStrings and not spStrings->empty())
                sysClassName = spStrings->at(0);
        }

        busInfos.emplace_back(I2CBusInfo{ i, std::move(sysClassName) });
    }

    return busInfos;
}


I2CBusIo::I2CBusIo(uint32_t id):
    _id(id),
    _fd(-1)
{
    char filename[64] = {0};

    snprintf(filename, sizeof(filename), "/dev/i2c-%d", id);

    _fd = open(filename, O_RDWR);

    if (!IsReady())
        std::cout << "Error opening " << filename << std::endl;
}

I2CBusIo::~I2CBusIo()
{
    if (IsReady())
        close(_fd);
}

bool I2CBusIo::Read(uint16_t chipAddress, uint8_t* data, uint16_t size)
{
    if (!IsReady())
        return false;
        
    int r = ioctl(_fd, I2C_SLAVE, chipAddress);
    if (r < 0)
        return false;

    r = read(_fd, data, size);
    return (r == size);
}

bool I2CBusIo::Write(uint16_t chipAddress, const uint8_t* data, uint16_t size)
{
    if (!IsReady())
        return false;
        
    int r = ioctl(_fd, I2C_SLAVE, chipAddress);
    if (r < 0)
        return false;

    r = write(_fd, data, size);
    return (r == size);
}

} // namespace SwApi