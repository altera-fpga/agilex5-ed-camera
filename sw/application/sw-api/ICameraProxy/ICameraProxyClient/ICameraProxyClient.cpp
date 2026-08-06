/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "ICameraProxyClient.h"
#include <format>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include "ICamera_ioctl.h"


ICameraProxyClient::ICameraProxyClient(const char* const model, const uint32_t idx)
{
    std::stringstream ssPath;
    ssPath << "/dev/icamera/" << model;
    if(idx != 0xFFFFFFFFU)
    {
        ssPath << "/" << idx;
    }
    std::string sPath = ssPath.str();
    m_fdDriver = open(sPath.c_str(), O_RDWR);
    if(m_fdDriver < 0)
    {
        throw std::runtime_error(std::format("ICameraProxyClient can not access {}", sPath.c_str()));
    }
}

ICameraProxyClient::~ICameraProxyClient()
{
    if(m_fdDriver >= 0)
    {
        close(m_fdDriver);
        m_fdDriver = -1;
    }
}

uint32_t ICameraProxyClient::GetMIPIInterface()
{
    uint32_t mipiInterface = 0U;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetMIPIInterface" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_MIPI_INTERFACE, &mipiInterface);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetMIPIInterface ioctl error {}", errno));
        }
    }
    return mipiInterface;
}


void ICameraProxyClient::Start()
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::Start" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::START);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::Start ioctl error {}", errno));
        }
    }
}

void ICameraProxyClient::Stop()
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::Stop" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::STOP);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::Stop ioctl error {}", errno));
        }
    }
}

void ICameraProxyClient::Reset()
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::Reset" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::RESET);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::Reset ioctl error {}", errno));
        }
    }
}

uint32_t ICameraProxyClient::GetFocus()
{
    uint32_t focus = 0U;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetFocus" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_FOCUS, &focus);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetFocus ioctl error {}", errno));
        }
    }
    return focus;
}

void ICameraProxyClient::SetFocus(const uint32_t focus)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetFocus " << focus << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_FOCUS, &focus);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetFocus ioctl error {}", errno));
        }
    }
}

std::pair<uint32_t, uint32_t> ICameraProxyClient::GetFocusRange()
{
    std::pair<uint32_t, uint32_t> focalRange {0, 0};
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetFocusRange " << std::endl;
#endif
        ICameraIOCTL_RANGE_UINT32T focusRangeIoctl;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_FOCUS_RANGE, &focusRangeIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetFocusRange ioctl error {}", errno));
        }
        focalRange.first = focusRangeIoctl.low;
        focalRange.second = focusRangeIoctl.high;
    }
    return focalRange;
}

float ICameraProxyClient::GetExposure()
{
    float exposure = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetExposure" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_EXPOSURE, &exposure);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetExposure ioctl error {}", errno));
        }
    }
    return exposure;
}

void ICameraProxyClient::SetExposure(const float exposure)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetExposure " << exposure << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_EXPOSURE, &exposure);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetExposure ioctl error {}", errno));
        }
    }
}

float ICameraProxyClient::GetExposureTime()
{
    float exposureTime = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetExposureTime" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_EXPOSURE_TIME, &exposureTime);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetExposureTime ioctl error {}", errno));
        }
    }
    return exposureTime;
}

std::pair<float, float> ICameraProxyClient::GetExposureRange()
{
    std::pair<float, float> exposureRange {0.0F, 0.0F};

    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetExposureRange " << std::endl;
#endif
        ICameraIOCTL_RANGE_FLOAT exposureRangeIoctl;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_EXPOSURE_RANGE, &exposureRangeIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetExposureRange ioctl error {}", errno));
        }
        exposureRange.first = exposureRangeIoctl.low;
        exposureRange.second = exposureRangeIoctl.high;
    }

    return exposureRange;
}

float ICameraProxyClient::GetExposureStep()
{
    float exposureStep = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetExposureStep" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_EXPOSURE_STEP, &exposureStep);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetExposureStep ioctl error {}", errno));
        }
    }
    return exposureStep;
}

float ICameraProxyClient::GetFrameRate()
{
    float framerate = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetFrameRate" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_FRAMERATE, &framerate);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetFrameRate ioctl error {}", errno));
        }
    }
    return framerate;
}

void ICameraProxyClient::SetFrameRate(const float frameRate)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetFrameRate " << frameRate << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_FRAMERATE, &frameRate);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetFrameRate ioctl error {}", errno));
        }
    }
}

std::pair<float, float> ICameraProxyClient::GetFrameRateRange()
{
    std::pair<float, float> framerateRange {0.0F, 0.0F};

    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetFrameRateRange " << std::endl;
#endif
        ICameraIOCTL_RANGE_FLOAT framerateRangeIoctl;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_FRAMERATE_RANGE, &framerateRangeIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetFrameRateRange ioctl error {}", errno));
        }
        framerateRange.first = framerateRangeIoctl.low;
        framerateRange.second = framerateRangeIoctl.high;
    }

    return framerateRange;
}

float ICameraProxyClient::GetFrameRateStep()
{
    float framerateStep = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetFrameRateStep" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_FRAMERATE_STEP, &framerateStep);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetFrameRateStep ioctl error {}", errno));
        }
    }
    return framerateStep;
}

float ICameraProxyClient::GetShutterSpeed()
{
    float shutterSpeed = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetShutterSpeed" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_SHUTTER_SPEED, &shutterSpeed);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetShutterSpeed ioctl error {}", errno));
        }
    }
    return shutterSpeed;
}

void ICameraProxyClient::SetShutterSpeed(float shutterSpeed)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetShutterSpeed " << shutterSpeed << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_SHUTTER_SPEED, &shutterSpeed);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetShutterSpeed ioctl error {}", errno));
        }
    }
}

std::pair<float, float> ICameraProxyClient::GetShutterSpeedRange()
{
    std::pair<float, float> shutterSpeedRange {0.0F, 0.0F};

    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetShutterSpeedRange " << std::endl;
#endif
        ICameraIOCTL_RANGE_FLOAT shutterSpeedRangeIoctl;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_SHUTTER_SPEED_RANGE, &shutterSpeedRangeIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetShutterSpeedRange ioctl error {}", errno));
        }
        shutterSpeedRange.first = shutterSpeedRangeIoctl.low;
        shutterSpeedRange.second = shutterSpeedRangeIoctl.high;
    }

    return shutterSpeedRange;
}

float ICameraProxyClient::GetShutterSpeedStep()
{
    float shutterSpeedStep = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetShutterSpeedStep" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_SHUTTER_SPEED_STEP, &shutterSpeedStep);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetShutterSpeedStep ioctl error {}", errno));
        }
    }
    return shutterSpeedStep;
}

float ICameraProxyClient::GetAnalogueGain()
{
    float analogueGain = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetAnalogueGain" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_ANALOGUE_GAIN, &analogueGain);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetAnalogueGain ioctl error {}", errno));
        }
    }
    return analogueGain;
}

void ICameraProxyClient::SetAnalogueGain(const float analogueGain)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetAnalogueGain " << analogueGain << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_ANALOGUE_GAIN, &analogueGain);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetAnalogueGain ioctl error {}", errno));
        }
    }
}

std::pair<float, float> ICameraProxyClient::GetAnalogueGainRange()
{
    std::pair<float, float> analogueGainRange {0.0F, 0.0F};

    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetAnalogueGainRange " << std::endl;
#endif
        ICameraIOCTL_RANGE_FLOAT analogueGainRangeIoctl;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_ANALOGUE_GAIN_RANGE, &analogueGainRangeIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetAnalogueGainRange ioctl error {}", errno));
        }
        analogueGainRange.first = analogueGainRangeIoctl.low;
        analogueGainRange.second = analogueGainRangeIoctl.high;
    }

    return analogueGainRange;
}

float ICameraProxyClient::GetAnalogueGainStep()
{
    float analogueGainStep = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetAnalogueGain" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_ANALOGUE_GAIN_STEP, &analogueGainStep);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetAnalogueGainStep ioctl error {}", errno));
        }
    }
    return analogueGainStep;
}

float ICameraProxyClient::GetDigitalGain()
{
    float digitalGain = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetDigitalGain" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_DIGITAL_GAIN, &digitalGain);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetDigitalGain ioctl error {}", errno));
        }
    }
    return digitalGain;
}

void ICameraProxyClient::SetDigitalGain(const float digitalGain)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetDigitalGain " << digitalGain << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_DIGITAL_GAIN, &digitalGain);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetDigitalGain ioctl error {}", errno));
        }
    }
}

std::pair<float, float> ICameraProxyClient::GetDigitalGainRange()
{
    std::pair<float, float> digitalGainRange {0.0F, 0.0F};
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetDigitalGainRange " << std::endl;
#endif
        ICameraIOCTL_RANGE_FLOAT digitalGainRangeIoctl;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_DIGITAL_GAIN_RANGE, &digitalGainRangeIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetDigitalGainRange ioctl error {}", errno));
        }
        digitalGainRange.first = digitalGainRangeIoctl.low;
        digitalGainRange.second = digitalGainRangeIoctl.high;
    }
    return digitalGainRange;
}

float ICameraProxyClient::GetDigitalGainStep()
{
    float digitalGainStep = 0.0F;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetDigitalGainStep" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_DIGITAL_GAIN_STEP, &digitalGainStep);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetDigitalGainStep ioctl error {}", errno));
        }
    }
    return digitalGainStep;
}

std::pair<uint16_t, uint16_t> ICameraProxyClient::GetResolution()
{
    std::pair<uint16_t, uint16_t> resolution {0U, 0U};
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetResolution" << std::endl;
#endif
        _ICameraIOCTL_RESOLUTION resolutionIoctl;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_RESOLUTION, &resolutionIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetResolution ioctl error {}", errno));
        }
        resolution.first = resolutionIoctl.width;
        resolution.second = resolutionIoctl.height;
    }
    return resolution;
}

void ICameraProxyClient::SetResolution(const std::pair<uint16_t, uint16_t>& resolution)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetResolution" << std::endl;
#endif
        _ICameraIOCTL_RESOLUTION resolutionIoctl;
        resolutionIoctl.width = resolution.first;
        resolutionIoctl.height = resolution.second;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_RESOLUTION, &resolutionIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetResolution ioctl error {}", errno));
        }
    }
}

uint32_t ICameraProxyClient::GetTargetFrameRate()
{
    uint32_t targetFramerate = 0U;

    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetTargetFrameRate" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_TARGET_FRAMERATE, &targetFramerate);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetTargetFrameRate ioctl error {}", errno));
        }
    }

    return targetFramerate;
}

void ICameraProxyClient::SetTargetFrameRate(const uint32_t targetFramerate)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetTargetFrameRate " << targetFramerate << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_TARGET_FRAMERATE, &targetFramerate);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetTargetFrameRate ioctl error {}", errno));
        }
    }
}

TCfaPhase ICameraProxyClient::GetCfaPhase()
{
    TCfaPhase cfaPhase = TCfaPhase::BGGR;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetCfaPhase" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_CFA_PHASE, &cfaPhase);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetCfaPhase ioctl error {}", errno));
        }
    }
    return cfaPhase;
}

void ICameraProxyClient::SetCfaPhase(const TCfaPhase& cfaPhase)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetCfaPhase" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_CFA_PHASE, &cfaPhase);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetCfaPhase ioctl error {}", errno));
        }
    }
}

void ICameraProxyClient::SetHDRState(bool hdrState)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::SetHDRState" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::SET_HDR_STATE, &hdrState);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::SetHDRState ioctl error {}", errno));
        }
    }
}

bool ICameraProxyClient::GetHDRState()
{
    bool hdrState = true;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetHDRState" << std::endl;
#endif
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_HDR_STATE, &hdrState);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetHDRState ioctl error {}", errno));
        }
    }
    return hdrState;
}

std::string ICameraProxyClient::GetModel()
{
    std::string model;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::GetModel" << std::endl;
#endif
        char modelIoctl[ICAMERA_MAX_STRING];
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::GET_MODEL, modelIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::GetModel ioctl error {}", errno));
        }
        model = std::string(modelIoctl);
    }
    return model;
}

void ICameraProxyClient::CtlWrite(const uint32_t addr, const uint32_t val)
{
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::CtlWrite " << addr << " " << val << std::endl;
#endif
        ICameraIOCTL_CTL ctlIoctl;
        ctlIoctl.addr = addr;
        ctlIoctl.val = val;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::CTL_WRITE, &ctlIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::CtlWrite ioctl error {}", errno));
        }
    }
}

uint32_t ICameraProxyClient::CtlRead(const uint32_t addr)
{
    uint32_t val = 0U;
    if(m_fdDriver < 0)
    {
        throw std::runtime_error("ICameraProxyClient device not open");
    }
    else
    {
#ifdef VERBOSE
        std::cout << "ICameraProxyClient::CtlRead " << addr << std::endl;
#endif
        ICameraIOCTL_CTL ctlIoctl;
        ctlIoctl.addr = addr;
        int rc = ioctl(m_fdDriver, (unsigned long)ICameraIOCTL::CTL_READ, &ctlIoctl);
        if(rc < 0)
        {
            throw std::runtime_error(std::format("ICameraProxyClient::CtlRead ioctl error {}", errno));
        }
        val = ctlIoctl.val;
    }
    return val;
}
