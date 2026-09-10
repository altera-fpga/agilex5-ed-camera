/* Copyright (C) Altera Corporation
 *
 * SPDX-License-Identifier: GPL-2.0-only */

#define FUSE_USE_VERSION 35
#include <fuse3/fuse.h>

#include <iostream>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include "ICameraProxyServer.h"
#include "ICamera_ioctl.h"


#ifdef CAMERA_FramosGMSL
#include "FramosGMSL.h"
#endif
#ifdef CAMERA_FramosIMX676
#include "FramosIMX676.h"
#endif
#ifdef CAMERA_FramosIMX678
#include "FramosIMX678.h"
#endif
#ifdef CAMERA_SonyIMX477
#include "SonyIMX477.h"
#endif
#ifdef CAMERA_SonyIMX519
#include "SonyIMX519.h"
#endif


struct DeviceClass
{
	std::string _name;
	std::function<void(tICameraMap& devInstanceMap)> _enumerate;
};


static const DeviceClass devices[] = {
#ifdef CAMERA_FramosGMSL
    {"framos-gmsl", [](tICameraMap& devInstanceMap){
        if(auto gmslDev = SwApi::FramosGMSL::Create(0))
            devInstanceMap.emplace(0, nullptr);
        if(auto gmslDev = SwApi::FramosGMSL::Create(1))
            devInstanceMap.emplace(1, nullptr);
    }},
#endif
#ifdef CAMERA_FramosIMX676
    {"framos-imx676", [](tICameraMap& devInstanceMap){
        if(auto camera = SwApi::FramosImx676::Create(0, 3000U))
                devInstanceMap.emplace(0, camera);
        if(auto camera = SwApi::FramosImx676::Create(1, 3000U))
                devInstanceMap.emplace(1, camera);
    }},
#endif
#ifdef CAMERA_FramosIMX678
    {"framos-imx678", [](tICameraMap& devInstanceMap){
        if(auto camera = SwApi::FramosImx678::Create(0, 6000U))
                devInstanceMap.emplace(0, camera);
        if(auto camera = SwApi::FramosImx678::Create(1, 6000U))
                devInstanceMap.emplace(1, camera);
    }},   
#endif
#ifdef CAMERA_SonyIMX477
    {"sony-imx477", [](tICameraMap& devInstanceMap){
        if(auto camera = SwApi::SonyImx477::Create(0, 3000U))
            devInstanceMap.emplace(0, camera);
    }},
#endif
#ifdef CAMERA_SonyIMX519
    {"sony-imx519", [](tICameraMap& devInstanceMap){
        if(auto camera = SwApi::SonyImx519::Create(0, 6000U))
            devInstanceMap.emplace(0, camera);
    }}
#endif
};


// Note: some of the supported cameras are powered up
// using a GPIO, controlled from the main ISP app.
// To make sure the cameras are powered up before being used
// the probing/enumeration process is postponed until the first
// access to the camera device file from the ISP app.
// The function below is called from the fuse file and folder
// access methods to ensure this order.

static void icamera_enumerate()
{
    struct fuse_context * context = fuse_get_context();
    tICameraTypeMap* iCameraTypeMap = (tICameraTypeMap*)context->private_data;
    
    // Make sure enumeration is done only once
    if(iCameraTypeMap->empty())
    {
        for(const auto& dev : devices)
        {
            const auto& name = dev._name;

            if(dev._enumerate)
            {
                auto& instances = (*iCameraTypeMap)[name];
                dev._enumerate(instances);
            }
        }      
    }  
}

static bool icamera_file_type(const char *path, std::string& node, uint32_t& idx, bool& isDir)
{
    std::string sPath(path);
    node = "";
    idx = 0xFFFFFFFFU;
    isDir = false;
    bool rc = false;

    if(sPath == "/")
    {
		node = "/";
        isDir = true;
        rc = true;
    }
	else if (sPath == "/dummy")
    {
		node = "dummy";
        idx = 0U;
        rc = true;
    }
    else
    {
        for(const auto& dev : devices)
        {
            if (sPath == std::string("/")+dev._name)
            {
                node = dev._name;
                isDir = true;
                rc = true;
                break;
            }
            else
            {
                const std::string camera_prefix = std::string("/")+dev._name+std::string("/");
                if (sPath.starts_with(camera_prefix))
                {
                    idx = static_cast<uint32_t>(std::stoul(sPath.substr(camera_prefix.length())));
                    node = dev._name;
                    rc = true;
                    break;
                }
            }
        }      
    }
    if(!isDir)
    {
        struct fuse_context * context = fuse_get_context();
        tICameraTypeMap* iCameraTypeMap = (tICameraTypeMap*)context->private_data;
        tICameraTypeMap::iterator itICameraTypeMap = iCameraTypeMap->find(node);
        if(itICameraTypeMap == iCameraTypeMap->end())
        {
            node = "";
            idx = 0xFFFFFFFFU;
            rc = false;
        }
        else
        {
            tICameraMap& iCameraMap = itICameraTypeMap->second;
            if(iCameraMap.find(idx) == iCameraMap.end())
            {
                node = "";
                idx = 0xFFFFFFFFU;
                rc = false;
            }
        }
    }
	return rc;
}

static int icamera_getattr(const char *path, struct stat *stbuf,
			struct fuse_file_info *fi)
{
    int rc = 0;
	(void) fi;

    icamera_enumerate();

	stbuf->st_uid = getuid();
	stbuf->st_gid = getgid();
	stbuf->st_atime = stbuf->st_mtime = time(NULL);

    std::string node;
    uint32_t idx;
    bool isDir;
    if(icamera_file_type(path, node, idx, isDir))
    {
        if(node == "")
        {
            rc = -ENOENT;
        }
        else if(isDir)
        {
            stbuf->st_mode = S_IFDIR | 0755;
            stbuf->st_nlink = 2;
        }
        else
        {
            stbuf->st_mode = S_IFREG | 0644;
            stbuf->st_nlink = 1;
            stbuf->st_size = 1;
        }
    }

	return rc;
}

static int icamera_open(const char *path, struct fuse_file_info *fi)
{
	(void) fi;
    int rc = -ENOENT;
    std::string node;
    uint32_t idx;
    bool isDir;

    icamera_enumerate();

    if(icamera_file_type(path, node, idx, isDir))
    {
        if(node != "")
        {
            rc = 0;
        }
    }
	return rc;
}

static int icamera_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			off_t offset, struct fuse_file_info *fi,
			enum fuse_readdir_flags flags)
{
	(void) fi;
	(void) offset;
	(void) flags;

    int rc = -ENOENT;
    std::string node;
    uint32_t idx;
    bool isDir;

    icamera_enumerate();

    if(icamera_file_type(path, node, idx, isDir))
    {
        struct fuse_context * context = fuse_get_context();
        tICameraTypeMap* iCameraTypeMap = (tICameraTypeMap*)context->private_data;
        if(node == "/")
        {
            filler(buf, ".", NULL, 0, (fuse_fill_dir_flags)0);
            filler(buf, "..", NULL, 0, (fuse_fill_dir_flags)0);
            if(iCameraTypeMap->find("dummy") != iCameraTypeMap->end())
            {
                filler(buf, "dummy", NULL, 0, (fuse_fill_dir_flags)0);
            }

            for(const auto& devClass: *iCameraTypeMap)
            {
                filler(buf, devClass.first.c_str(), NULL, 0, (fuse_fill_dir_flags)0);
            }

            rc = 0;
        }
        else
        {
            if(isDir)
            {
                filler(buf, ".", NULL, 0, (fuse_fill_dir_flags)0);
                filler(buf, "..", NULL, 0, (fuse_fill_dir_flags)0);
                tICameraTypeMap::iterator itICameraTypeMap = iCameraTypeMap->find(node);
                if(itICameraTypeMap != iCameraTypeMap->end())
                {
                    tICameraMap& iCameraMap = itICameraTypeMap->second;
                    for(auto const& itICameraMap : iCameraMap)
                    {
                        std::string sIdx = std::to_string(itICameraMap.first);
                        filler(buf, sIdx.c_str(),  NULL, 0, (fuse_fill_dir_flags)0);
                    }
                }
                rc = 0;
            }
        }
    }

	return rc;
}

static int icamera_ioctl(const char *path, unsigned int cmd, void *arg,
		      struct fuse_file_info *fi, unsigned int flags, void *data)
{
	(void) arg;
	(void) fi;
	(void) flags;

    int rc = 0;
    std::shared_ptr<ICamera> iCamera = nullptr;

    struct fuse_context * context = fuse_get_context();
    std::string node;
    uint32_t idx;
    bool isDir;
    if(!icamera_file_type(path, node, idx, isDir))
    {
        rc = -EINVAL;
    }
	else if (node == "")
    {
        rc = -EINVAL;
    }
	else if (isDir)
    {
        rc = -EINVAL;
    }
    else if ((flags & FUSE_IOCTL_COMPAT))
    {
        rc = -ENOSYS;
    }
    if(context == nullptr)
    {
        rc = -EINVAL;
    }
    else
    {
        tICameraTypeMap* iCameraTypeMap = (tICameraTypeMap*)context->private_data;
        tICameraTypeMap::iterator itICameraTypeMap = iCameraTypeMap->find(node);
        if(itICameraTypeMap == iCameraTypeMap->end())
        {
            rc = -EINVAL;
        }
        else
        {
            tICameraMap& iCameraMap = itICameraTypeMap->second;
            if(iCameraMap.find(idx) != iCameraMap.end())
            {
                iCamera = itICameraTypeMap->second.at(idx);
            }
            else
            {
                rc = -EINVAL;
            }
        }

        if(iCamera == nullptr)
        {
            rc = -EINVAL;
        }
    }
    if((rc == 0) && (iCamera != nullptr))
    {
        switch ((ICameraIOCTL)cmd)
        {
        case ICameraIOCTL::GET_MIPI_INTERFACE:
            {
                uint32_t index = iCamera->GetMIPIInterface();
                *(uint32_t*)data = index;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::START:
            {
                iCamera->Start();
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::STOP:
            {
                iCamera->Stop();
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::RESET:
            {
                iCamera->Reset();
                rc = 0;
            }
            break;
            

        case ICameraIOCTL::GET_FOCUS:
            {
                uint32_t focus = iCamera->GetFocus();
                *(uint32_t*)data = focus;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_FOCUS:
            {
                uint32_t focus = *(uint32_t*)data;
                iCamera->SetFocus(focus);
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_FOCUS_RANGE:
            {
                ICameraIOCTL_RANGE_UINT32T* pRange = (ICameraIOCTL_RANGE_UINT32T*)data;
                std::pair<uint32_t, uint32_t> range = iCamera->GetFocusRange();
                pRange->low = range.first;
                pRange->high = range.second;
                rc = 0;
            }
            break;           
    
        case ICameraIOCTL::GET_EXPOSURE:
            {
                float exposure = iCamera->GetExposure();
                *(float*)data = exposure;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_EXPOSURE:
            {
                float exposure = *(float*)data;
                iCamera->SetExposure(exposure);
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_EXPOSURE_TIME:
            {
                float exposureTime = iCamera->GetExposureTime();
                *(float*)data = exposureTime;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_EXPOSURE_RANGE:
            {
                ICameraIOCTL_RANGE_FLOAT* pRange = (ICameraIOCTL_RANGE_FLOAT*)data;
                std::pair<float, float> range = iCamera->GetExposureRange();
                pRange->low = range.first;
                pRange->high = range.second;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_EXPOSURE_STEP:
            {
                float exposureStep = iCamera->GetExposure();
                *(float*)data = exposureStep;
                rc = 0;
            }
            break;

        case ICameraIOCTL::GET_FRAMERATE:
            {
                float framerate = iCamera->GetFrameRate();
                *(float*)data = framerate;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_FRAMERATE:
            {
                float framerate = *(float*)data;
                iCamera->SetFrameRate(framerate);
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_FRAMERATE_RANGE:
            {
                ICameraIOCTL_RANGE_FLOAT* pRange = (ICameraIOCTL_RANGE_FLOAT*)data;
                std::pair<float, float> range = iCamera->GetFrameRateRange();
                pRange->low = range.first;
                pRange->high = range.second;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_FRAMERATE_STEP:
            {
                float framerateStep = iCamera->GetFrameRateStep();
                *(float*)data = framerateStep;
                rc = 0;
            }
            break;

        case ICameraIOCTL::GET_SHUTTER_SPEED:
            {
                float shutterSpeed = iCamera->GetShutterSpeed();
                *(float*)data = shutterSpeed;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_SHUTTER_SPEED:
            {
                float shutterSpeed = *(float*)data;
                iCamera->SetShutterSpeed(shutterSpeed);
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_SHUTTER_SPEED_RANGE:
            {
                ICameraIOCTL_RANGE_FLOAT* pRange = (ICameraIOCTL_RANGE_FLOAT*)data;
                std::pair<float, float> range = iCamera->GetShutterSpeedRange();
                pRange->low = range.first;
                pRange->high = range.second;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_SHUTTER_SPEED_STEP:
            {
                float shutterSpeedStep = iCamera->GetShutterSpeedStep();
                *(float*)data = shutterSpeedStep;
                rc = 0;
            }
            break;

        case ICameraIOCTL::GET_ANALOGUE_GAIN:
            {
                float analogueGain = iCamera->GetAnalogueGain();
                *(float*)data = analogueGain;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_ANALOGUE_GAIN:
            {
                float analogueGain = *(float*)data;
                iCamera->SetAnalogueGain(analogueGain);
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_ANALOGUE_GAIN_RANGE:
            {
                ICameraIOCTL_RANGE_FLOAT* pRange = (ICameraIOCTL_RANGE_FLOAT*)data;
                std::pair<float, float> range = iCamera->GetAnalogueGainRange();
                pRange->low = range.first;
                pRange->high = range.second;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_ANALOGUE_GAIN_STEP:
            {
                float analogueGainStep = iCamera->GetAnalogueGainStep();
                *(float*)data = analogueGainStep;
                rc = 0;
            }
            break;
            

        case ICameraIOCTL::GET_DIGITAL_GAIN:
            {
                float digitalGain = iCamera->GetDigitalGain();
                *(float*)data = digitalGain;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_DIGITAL_GAIN:
            {
                float digitialGain = *(float*)data;
                iCamera->SetDigitalGain(digitialGain);
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_DIGITAL_GAIN_RANGE:
            {
                ICameraIOCTL_RANGE_FLOAT* pRange = (ICameraIOCTL_RANGE_FLOAT*)data;
                std::pair<float, float> range = iCamera->GetDigitalGainRange();
                pRange->low = range.first;
                pRange->high = range.second;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_DIGITAL_GAIN_STEP:
            {
                float digitalGainStep = iCamera->GetDigitalGainStep();
                *(float*)data = digitalGainStep;
                rc = 0;
            }
            break;
            

        case ICameraIOCTL::GET_RESOLUTION:
            {
                ICameraIOCTL_RESOLUTION* pResolution = (ICameraIOCTL_RESOLUTION*)data;
                std::pair<uint16_t, uint16_t> resolution = iCamera->GetResolution();
                pResolution->width = resolution.first;
                pResolution->height = resolution.second;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_RESOLUTION:
            {
                ICameraIOCTL_RESOLUTION* pResolution = (ICameraIOCTL_RESOLUTION*)data;
                std::pair<uint16_t, uint16_t> resolution;
                resolution.first = pResolution->width;
                resolution.second = pResolution->height;
                iCamera->SetResolution(resolution);
                rc = 0;
            }
            break;
            

        case ICameraIOCTL::GET_TARGET_FRAMERATE:
            {
                uint32_t targetFramerate = iCamera->GetTargetFrameRate();
                *(uint32_t*)data = targetFramerate;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_TARGET_FRAMERATE:
            {
                uint32_t targetFramerate = *(uint32_t*)data;
                iCamera->SetTargetFrameRate(targetFramerate);
                rc = 0;
            }
            break;
            

        case ICameraIOCTL::GET_CFA_PHASE:
            {
                TCfaPhase cfaPhase = iCamera->GetCfaPhase();
                *(TCfaPhase*)data = cfaPhase;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_CFA_PHASE:
            {
                TCfaPhase cfaPhase = *(TCfaPhase*)data;
                iCamera->SetCfaPhase(cfaPhase);
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_HDR_STATE:
            {
                bool hdrState = iCamera->GetHDRState();
                *(bool*)data = hdrState;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::SET_HDR_STATE:
            {
                bool hdrState = *(bool*)data;
                iCamera->SetHDRState(hdrState);
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::GET_MODEL:
            {
                std::string model = iCamera->GetModel();
                memset(data, 0, ICAMERA_MAX_STRING);
                strncpy((char*)data, model.c_str(), ICAMERA_MAX_STRING-1U);
                rc = 0;
            }
            break;

        case ICameraIOCTL::CTL_READ:
            {
                _ICameraIOCTL_CTL* pCtlRead = (_ICameraIOCTL_CTL*)data;
                uint32_t addr = pCtlRead->addr;
                uint32_t val = iCamera->CtlRead(addr);
                pCtlRead->val = val;
                rc = 0;
            }
            break;
            
        case ICameraIOCTL::CTL_WRITE:
            {
                _ICameraIOCTL_CTL* pCtlRead = (_ICameraIOCTL_CTL*)data;
                uint32_t addr = pCtlRead->addr;
                uint32_t val = pCtlRead->val;
                iCamera->CtlWrite(addr, val);
                rc = 0;
            }
            break;

        default:
            rc = -EINVAL;
            break;
        }
    }
	return rc;
}

static const struct fuse_operations icamera_oper = {
	.getattr	= icamera_getattr,
	.open		= icamera_open,
	.readdir	= icamera_readdir,
	.ioctl		= icamera_ioctl
};

int ICamera_main(int argc, char *argv[])
{
    tICameraTypeMap iCameraTypeMap;
	return fuse_main(argc, argv, &icamera_oper, (void*)&iCameraTypeMap);
}
