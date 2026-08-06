/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "StitchAdapter.h"
#include "Logging.h"
#include <cmath>

namespace SwApi
{
    const uint32_t StitchAdapter::DEFAULT_INPUT_WIDTH = 1920;
    const uint32_t StitchAdapter::DEFAULT_INPUT_HEIGHT = 1080;
    const uint32_t StitchAdapter::DEFAULT_OFFSET_X = 0;
    const uint32_t StitchAdapter::DEFAULT_OFFSET_Y = 0;
    const uint32_t StitchAdapter::DEFAULT_OVERLAP = 100;
    const uint32_t StitchAdapter::ISP_LAYER = 1;


    // Factory create with a Hapi::VvpWarpPtr
    std::shared_ptr<StitchAdapter> StitchAdapter::Create(std::vector<std::shared_ptr<SwApi::StitchAdapter::tStitchInputConfig>>& vspStitchInputConfig, std::shared_ptr<IMixer> spMixer)
    {
        auto spStitchAdapter = std::make_shared<StitchAdapter>(vspStitchInputConfig, spMixer);

        return spStitchAdapter;
    }


    StitchAdapter::StitchAdapter(std::vector<std::shared_ptr<SwApi::StitchAdapter::tStitchInputConfig>>& vspStitchInputConfig, std::shared_ptr<IMixer> spMixer)
    : _vspStitchInputConfig{vspStitchInputConfig}
    , _spMixer{spMixer}
    , _mixerOutputWidth(0)
    , _mixerOutputHeight(0)
    , _offsetX(DEFAULT_OFFSET_X)
    , _offsetY(DEFAULT_OFFSET_Y)
    , _overlap(DEFAULT_OVERLAP)
    , _hfov(110.0)
    , _dfov(127.0)
    , _focalLength(0.004)
    , _distortion_a(0.004)
    , _distortion_b(0.0)
    , _distortion_c(0.0)
    , _distortion_d(0.0)
    , _directTransform(true)
    , _lensCorrection(false)
    , _lensDistortion(true)
    , _lensDistortionOverride(false)
    , _cylindricalProjection(false)
    , _pauseUpdates(false)
    {
        SetInterCameraAngle(90.0);
        for(uint32_t isp_layer = 0; isp_layer < _vspStitchInputConfig.size(); isp_layer++)
        {
            auto& stitchInputConfig = _vspStitchInputConfig[isp_layer];
            stitchInputConfig->_sensorInputWidth = 0;
            stitchInputConfig->_sensorInputHeight = 0;
            stitchInputConfig->_warpInputWidth = 0;
            stitchInputConfig->_warpInputHeight = 0;
            stitchInputConfig->_mixerInputHeight = 0;
            stitchInputConfig->_mixerInputWidth = 0;
            stitchInputConfig->_mixerInputHeight = 0;

            if(stitchInputConfig->_spStitchVfr)
            {
                stitchInputConfig->_spMask = std::make_shared<struct vfr_frame_t>((struct vfr_frame_t){
                        stitchInputConfig->_mixerInputWidth, 
                        stitchInputConfig->_mixerInputHeight, 
                        16, 
                        false, 
                        {stitchInputConfig->_mixerInputWidth*stitchInputConfig->_mixerInputHeight*2}
                    });
                stitchInputConfig->_spStitchVfr->SetResolutionOfImage(stitchInputConfig->_mixerInputWidth, stitchInputConfig->_mixerInputHeight);
                SetOverlap(_overlap, true);
                stitchInputConfig->_spStitchVfr->Run(true);
            }
            else
            {
                SetOverlap(_overlap, true);
            }
        }
        UpdateDistortion();
    }

    StitchAdapter::~StitchAdapter()
    {
    }

    void StitchAdapter::SetStitchSensorResolution(uint32_t isp_layer, uint32_t width, uint32_t height)
    {
        if(isp_layer < _vspStitchInputConfig.size())
        {
            auto& stitchInputConfig = _vspStitchInputConfig[isp_layer];

            if((width != stitchInputConfig->_sensorInputWidth) || (height != stitchInputConfig->_sensorInputHeight))
            {
                stitchInputConfig->_sensorInputWidth = width;
                stitchInputConfig->_sensorInputHeight = height;
                UpdateResolutions(isp_layer);
            }
        }
    }

    void StitchAdapter::SetStitchWarpResolution(uint32_t isp_layer, uint32_t width, uint32_t height)
    {
        if(isp_layer < _vspStitchInputConfig.size())
        {
            auto& stitchInputConfig = _vspStitchInputConfig[isp_layer];

            if((width != stitchInputConfig->_warpInputWidth) || (height != stitchInputConfig->_warpInputHeight))
            {
                stitchInputConfig->_warpInputWidth = width;
                stitchInputConfig->_warpInputHeight = height;
                UpdateWarp(isp_layer);
            }
        }
    }

    void StitchAdapter::SetStitchOutputResolution(uint32_t width, uint32_t height)
    {
        if((width != _mixerOutputWidth) || (height != _mixerOutputHeight))
        {
            _mixerOutputWidth = width;
            _mixerOutputHeight = height;
            for(uint32_t isp_layer = 0; isp_layer < _vspStitchInputConfig.size(); isp_layer++)
            {
                UpdateResolutions(isp_layer);
            }
        }
    }

    void StitchAdapter::SetOffsetX(int32_t offsetX)
    {
        if(_offsetX != offsetX)
        {
            _offsetX = offsetX;
            UpdateWarp();
        }
    }

    void StitchAdapter::SetOffsetY(int32_t offsetY)
    {
        if(_offsetY != offsetY)
        {
            _offsetY = offsetY;
            UpdateWarp();
        }
    }

    void StitchAdapter::SetOverlap(uint32_t overlap, bool forceUpdate)
    {
        if((_overlap != overlap) || forceUpdate)
        {
            _overlap = overlap;
            UpdateMixer();
        }
    }

    void StitchAdapter::SetHFoV(float hfov)
    {
        // may need epsilon here
        if(_hfov != hfov)
        {
            _hfov = hfov;
            for(uint32_t isp_layer = 0; isp_layer < _vspStitchInputConfig.size(); isp_layer++)
            {
                UpdateResolutions(isp_layer);
            }
            UpdateDistortion();
            UpdateWarp();
        }
    }

    void StitchAdapter::SetDFoV(float dfov)
    {
        // may need epsilon here
        if(_dfov != dfov)
        {
            _dfov = dfov;
            UpdateDistortion();
            UpdateWarp();
        }
    }

    void StitchAdapter::SetFocalLength(float focalLength)
    {
        // may need epsilon here
        if(_focalLength != focalLength)
        {
            _focalLength = focalLength;
            UpdateDistortion();
            UpdateWarp();
        }
    }

    void StitchAdapter::SetDistortionFTheta(float distortionFTheta)
    {
        // may need epsilon here
        if(_distortionFTheta != distortionFTheta)
        {
            _distortionFTheta = distortionFTheta;
            UpdateDistortion();
            UpdateWarp();
        }
    }

    void StitchAdapter::UpdateDistortion()
    {
        if(!_lensDistortionOverride)
        {
            float half_dfov_radians = M_PI*(_dfov/2.0)/180.0;
            float theta_power_four = powf(half_dfov_radians, 4);
            float ideal = _focalLength*half_dfov_radians;
            float error = ideal*_distortionFTheta;

            _distortion_a = _focalLength;
            _distortion_b = 0.0F;
            _distortion_c = 0.0F;
            _distortion_d = error / theta_power_four;
        }
    }

    void StitchAdapter::SetDirectTransform(bool enable)
    {
        if(_directTransform != enable)
        {
            _directTransform = enable;
            UpdateWarp();
        }
    }

    void StitchAdapter::SetLensCorrection(bool enable)
    {
        if(_lensCorrection != enable)
        {
            _lensCorrection = enable;
            UpdateWarp();
        }
    }

    void StitchAdapter::SetLensDistortionCoef(float a, float b, float c, float d)
    {
        _distortion_a = a;
        _distortion_b = b;
        _distortion_c = c;
        _distortion_d = d;
        UpdateWarp();
    }

    void StitchAdapter::SetLensDistortion(bool enable)
    {
        if(_lensDistortion != enable)
        {
            _lensDistortion = enable;
            UpdateWarp();
        }
    }

    void StitchAdapter::SetLensDistortionOverride(bool enable)
    {
        if(_lensDistortionOverride != enable)
        {
            _lensDistortionOverride = enable;
            if(!_lensDistortionOverride)
            {
                UpdateDistortion();
            }
            UpdateWarp();
        }
    }

    void StitchAdapter::SetCylindricalProjection(bool enable)
    {
        if(_cylindricalProjection != enable)
        {
            _cylindricalProjection = enable;
            UpdateWarp();
        }
    }

    void StitchAdapter::GetLensDistortionCoef(float& a, float& b, float& c, float& d)
    {
        a = _distortion_a;
        b = _distortion_b;
        c = _distortion_c;
        d = _distortion_d;
    }

    void StitchAdapter::SetInterCameraAngle(float interCameraAngle)
    {
        _interCameraAngle = interCameraAngle;
        float currentAngle = -0.5 * (_vspStitchInputConfig.size() - 1) * interCameraAngle;
        for(uint32_t isp_layer = 0; isp_layer < _vspStitchInputConfig.size(); isp_layer++)
        {
            auto& stitchInputConfig = _vspStitchInputConfig[isp_layer];
            stitchInputConfig->_cameraOffsetAngle = currentAngle;
            currentAngle += interCameraAngle;
        }
        for(uint32_t isp_layer = 0; isp_layer < _vspStitchInputConfig.size(); isp_layer++)
        {
            UpdateResolutions(isp_layer);
        }
    }

    uint32_t StitchAdapter::GetLayerOverlap()
    {
        auto& stitchInputConfig = _vspStitchInputConfig[0];
        return (uint32_t)(((_hfov-_interCameraAngle)/_hfov)*stitchInputConfig->_mixerInputWidth);
    }

    void StitchAdapter::PauseUpdates(bool pauseUpdates)
    {
        _pauseUpdates = pauseUpdates;
        if(pauseUpdates == false)
        {
            GenerateStitchBlend();
            UpdateMixer();
            UpdateDistortion();
            UpdateWarp();
        }
    }

    void StitchAdapter::UpdateResolutions(uint32_t isp_layer)
    {
        float outputAngleLeft;
        float outputAngleRight;
        for(uint32_t isp_layer = 0; isp_layer < _vspStitchInputConfig.size(); isp_layer++)
        {
            float inputAngleLeft = _vspStitchInputConfig[isp_layer]->_cameraOffsetAngle - _hfov/2.0;
            float inputAngleRight = _vspStitchInputConfig[isp_layer]->_cameraOffsetAngle + _hfov/2.0;
            if((isp_layer == 0) || (inputAngleLeft < outputAngleLeft))
            {
                outputAngleLeft = inputAngleLeft;
            }
            if((isp_layer == 0) || (inputAngleRight > outputAngleRight))
            {
                outputAngleRight = inputAngleRight;
            }
        }
        float outputFoV = outputAngleRight - outputAngleLeft;
        if(isp_layer < _vspStitchInputConfig.size())
        {
            auto& stitchInputConfig = _vspStitchInputConfig[isp_layer];

            stitchInputConfig->_mixerInputWidth = (uint32_t)(_mixerOutputWidth*_hfov/outputFoV);
            stitchInputConfig->_mixerInputHeight = (uint32_t)(((float)stitchInputConfig->_mixerInputWidth * 9.0) / 16.0);
            
            float inputAngleLeft = _vspStitchInputConfig[isp_layer]->_cameraOffsetAngle - _hfov/2.0;
            float inputAngleRight = _vspStitchInputConfig[isp_layer]->_cameraOffsetAngle + _hfov/2.0;

            float fminx = ((inputAngleLeft - outputAngleLeft)/outputFoV)*_mixerOutputWidth;
            float fmaxx = ((inputAngleRight - outputAngleLeft)/outputFoV)*_mixerOutputWidth;
            int32_t minx = (int32_t)fminx;
            int32_t maxx = (int32_t)fmaxx;
            if(minx < 0)
            {
                stitchInputConfig->_mixerInputX = 0;
                stitchInputConfig->_warpInputX = fminx;
            }
            else if(maxx > (int32_t)_mixerOutputWidth)
            {
                stitchInputConfig->_mixerInputX = _mixerOutputWidth - stitchInputConfig->_mixerInputWidth;
                stitchInputConfig->_warpInputX = fmaxx - (float)_mixerOutputWidth;
            }
            else
            {
                stitchInputConfig->_mixerInputX = minx;
                stitchInputConfig->_warpInputX = 0.0;
            }

            if(stitchInputConfig->_mixerInputHeight > _mixerOutputHeight)
            {
                stitchInputConfig->_mixerInputY = 0;
                stitchInputConfig->_warpInputY = ((float)_mixerOutputHeight - (float)stitchInputConfig->_mixerInputHeight)/2.0;
                stitchInputConfig->_mixerInputHeight = _mixerOutputHeight;
            }
            else
            {
                stitchInputConfig->_mixerInputY = (_mixerOutputHeight - stitchInputConfig->_mixerInputHeight)/2;
                stitchInputConfig->_warpInputY = 0.0;
            }


            if((stitchInputConfig->_sensorInputWidth != 0) && (stitchInputConfig->_sensorInputHeight != 0) && 
               (stitchInputConfig->_mixerInputWidth != 0) && (stitchInputConfig->_mixerInputHeight != 0))
            {
                SwApi::MixerLayerState currentLayerState = _spMixer->GetLayerEnableState(ISP_LAYER+isp_layer);
                _spMixer->SetLayerEnableState(ISP_LAYER+isp_layer, SwApi::MixerLayerState::MixerLayerDisabled);

                _spMixer->SetLayerResolution(ISP_LAYER+isp_layer, stitchInputConfig->_mixerInputWidth, stitchInputConfig->_mixerInputHeight);
                _spMixer->SetLayerOffsetPosition(ISP_LAYER+isp_layer, stitchInputConfig->_mixerInputX, stitchInputConfig->_mixerInputY);

                _spMixer->SetLayerEnableState(ISP_LAYER+isp_layer, currentLayerState);
            }
        }
        UpdateWarp(isp_layer);
        UpdateMixer();
    }

    void StitchAdapter::UpdateMixer()
    {
        if(!_pauseUpdates)
        {
            uint32_t layerOverlap = GetLayerOverlap();
            uint32_t start = 0;
            uint32_t stop = 0;
            if(_overlap < layerOverlap)
            {
                start = (layerOverlap/2-_overlap/2) & 0xFFFFFFFE;
                stop = (layerOverlap/2+_overlap/2) & 0xFFFFFFFE;
            }
            if(stop == 0)
            {
                stop = 2;
            }
            SetGradient(1, start, stop);
            GenerateStitchBlend();

            for(uint32_t isp_layer = 0; isp_layer < _vspStitchInputConfig.size(); isp_layer++)
            {
                auto& stitchInputConfig = _vspStitchInputConfig[isp_layer];

                if((stitchInputConfig->_mixerInputWidth != 0) && (stitchInputConfig->_mixerInputHeight != 0))
                {
                    std::cout << "Updating Mixer Layer [" << isp_layer << "]: xmin: " << stitchInputConfig->_mixerInputX << ", ymin:" << stitchInputConfig->_mixerInputY << " xmax: " << stitchInputConfig->_mixerInputX+stitchInputConfig->_mixerInputWidth << ", ymax:" << stitchInputConfig->_mixerInputY+stitchInputConfig->_mixerInputHeight << std::endl << std::flush;
                    _spMixer->SetLayerOffsetPosition(ISP_LAYER+isp_layer, (stitchInputConfig->_mixerInputX /2) * 2, stitchInputConfig->_mixerInputY);

                    ImageConfig inputInfo = VideoStandard(
                        stitchInputConfig->_mixerInputWidth,
                        stitchInputConfig->_mixerInputHeight,
                        6000, SimpleRgbColorInfo());
                    if(inputInfo != stitchInputConfig->_inputCore->GetCurrentInputVideoInfo())
                    {
                        stitchInputConfig->_inputCore->ApplyOutputVideoInfo(inputInfo);
                    }
                }
            }
        }
    }

    void StitchAdapter::UpdateWarp(uint32_t isp_layer)
    {
        if(!_pauseUpdates)
        {
            constexpr float PIXEL_SIZE = 0.000002;
            constexpr uint32_t dims = 9;
            constexpr float increment = 1.0/(dims-1);
            float half_hfov_radians = M_PI*(_hfov/2.0)/180.0;

            uint32_t isp_layer_start = 0;
            uint32_t isp_layer_end = _vspStitchInputConfig.size();
            if(isp_layer != ALL_LAYERS)
            {
                isp_layer_start = isp_layer;
                isp_layer_end = isp_layer + 1;
            }
            for(uint32_t current_isp_layer = isp_layer_start; current_isp_layer < isp_layer_end; current_isp_layer++)
            {
                auto& stitchInputConfig = _vspStitchInputConfig[current_isp_layer];

                if( (stitchInputConfig->_sensorInputWidth == 0) || 
                    (stitchInputConfig->_sensorInputHeight == 0) ||
                    (stitchInputConfig->_warpInputWidth == 0) ||
                    (stitchInputConfig->_warpInputHeight == 0) ||
                    (stitchInputConfig->_mixerInputWidth == 0) ||
                    (stitchInputConfig->_mixerInputHeight == 0) )
                {
                    break;
                }
                TRACE << "[Stitch Adapter " << std::to_string(current_isp_layer) << "]: "
                    << stitchInputConfig->_sensorInputWidth << "x" << stitchInputConfig->_sensorInputHeight << " -> "
                    << stitchInputConfig->_mixerInputWidth << "x" << stitchInputConfig->_mixerInputHeight
                    << '\n' << std::flush;

                float offsetX = 0.0;
                float offsetY = 0.0;
                if(current_isp_layer == 0)
                {
                    offsetX = -1.0 * _offsetX / 2.0;
                    offsetY = -1.0 * _offsetY / 2.0;
                }
                else
                {
                    offsetX = _offsetX / 2.0;
                    offsetY = _offsetY / 2.0;
                }
                offsetX *= (float)stitchInputConfig->_mixerInputWidth/(float)stitchInputConfig->_sensorInputWidth;
                offsetY *= (float)stitchInputConfig->_mixerInputHeight/(float)stitchInputConfig->_sensorInputHeight;
                auto configurator = stitchInputConfig->_spWarpAdapter->GetConfiguratorSafe();

                configurator->SetArbitraryKnotsNum(dims);

                uint32_t idx = 0;
                for(uint32_t y = 0; y < dims; y++)
                {
                    float fy = y*increment;
                    float py = stitchInputConfig->_sensorInputHeight*(fy - 0.5);
                    for(uint32_t x = 0; x < dims; x++)
                    {
                        float fx = x*increment;
                        float px = stitchInputConfig->_sensorInputWidth*(fx - 0.5);

                        float corrected_px = px;
                        float corrected_py = py;

                        if(_directTransform)
                        {
                            float radius = sqrt(corrected_px*corrected_px+corrected_py*corrected_py);
                            float theta = radius*PIXEL_SIZE/_focalLength;
                            float psi = atan2(corrected_py, corrected_px);
                            float cylinder_radius = stitchInputConfig->_mixerInputWidth*0.5/half_hfov_radians;

                            float radius_corrected = radius; 
                            if(_lensDistortion)
                            {
                                theta = FindTheta(radius*PIXEL_SIZE);
                                radius_corrected = _distortion_a*theta + _distortion_b*theta*theta + _distortion_c*theta*theta*theta + _distortion_d*theta*theta*theta*theta;
                                radius_corrected = radius_corrected/PIXEL_SIZE;
                            }

                            float p = sqrt(cylinder_radius*cylinder_radius/(sin(theta)*sin(theta)*cos(psi)*cos(psi) + cos(theta)*cos(theta)));
                            float cylinder_coord_x = p*sin(theta)*cos(psi);
                            float cylinder_coord_y = p*sin(theta)*sin(psi);
                            float cylinder_coord_z = p*cos(theta);

                            float cylinder_angle = atan2(cylinder_coord_x, cylinder_coord_z);
                            corrected_px = cylinder_radius*cylinder_angle;
                            corrected_py = cylinder_coord_y;
                        }
                        else
                        {
                            float adjacent = stitchInputConfig->_mixerInputWidth*0.5/tan(half_hfov_radians);
                            if(_lensCorrection)
                            {
                                float radius = sqrt(corrected_px*corrected_px+corrected_py*corrected_py);
                                float theta = radius*PIXEL_SIZE/_focalLength;
                                float radius_corrected = radius; 
                                if(_lensDistortion)
                                {
                                    theta = FindTheta(radius*PIXEL_SIZE);
                                    radius_corrected = _distortion_a*theta + _distortion_b*theta*theta + _distortion_c*theta*theta*theta + _distortion_d*theta*theta*theta*theta;
                                    radius_corrected = radius_corrected/PIXEL_SIZE;
                                }
                                float radius_distorted = adjacent*tan(radius*PIXEL_SIZE/_focalLength);

                                float phy = atan2(corrected_py, corrected_px);
                                corrected_px = radius_distorted * cos(phy);
                                corrected_py = radius_distorted * sin(phy);
                            }

                            if(_cylindricalProjection)
                            {
                                float cylinder_radius = stitchInputConfig->_mixerInputWidth*0.5/half_hfov_radians;
                                float theta = atan(corrected_px/adjacent);
                                float gamma = atan(corrected_py/sqrt(corrected_px*corrected_px+adjacent*adjacent));

                                //float theta_dash = theta - asin(0.04*sin(theta)/4.0);

                                corrected_px = stitchInputConfig->_mixerInputWidth*0.5*theta/(half_hfov_radians);
                                corrected_py = cylinder_radius*tan(gamma);
                            }
                        }

                        configurator->SetArbitraryKnot(idx, (stitchInputConfig->_mixerInputWidth*0.5 + corrected_px) + offsetX + stitchInputConfig->_warpInputX, (stitchInputConfig->_mixerInputHeight*0.5 + corrected_py) + offsetY + stitchInputConfig->_warpInputY);
                        idx++;
                    }
                }
                stitchInputConfig->_spWarpAdapter->UpdateArbitrary();
            }
        }
    }

    void StitchAdapter::SetGradient(uint32_t isp_layer, uint32_t start, uint32_t stop)
    {
        if(isp_layer < _vspStitchInputConfig.size())
        {
            auto& spStitchInputConfig = _vspStitchInputConfig[isp_layer];
            spStitchInputConfig->_gradient._start = start;
            spStitchInputConfig->_gradient._stop = stop;
        }
    }

    void StitchAdapter::GenerateStitchBlend()
    {
        for(uint32_t isp_layer = 0; isp_layer < _vspStitchInputConfig.size(); isp_layer++)
        {
            auto& spStitchInputConfig = _vspStitchInputConfig[isp_layer];
            if(spStitchInputConfig->_spAlphaChGen)
            {
                uint16_t pixel_start = spStitchInputConfig->_gradient._start & 0xFFFE;
                uint16_t pixel_stop = spStitchInputConfig->_gradient._stop & 0xFFFE;
                altera_vvp_alpha_channel_generator_set_config(spStitchInputConfig->_spAlphaChGen->GetInstance(), pixel_start, pixel_stop);
                altera_vvp_alpha_channel_generator_write_data(spStitchInputConfig->_spAlphaChGen->GetInstance(), 0U, 0x0U, 0x0U);
                if(pixel_stop < pixel_start)
                {
                    pixel_stop = pixel_start;
                }
                uint32_t length = pixel_stop - pixel_start;
                uint32_t lut_addr = 1;
                for(uint16_t index = 2U; index < length; index+=2)
                {
                    float f_alpha_even = (float)index/(float)length;
                    float f_alpha_odd = (float)(index+1)/(float)length;
                    uint16_t alpha_even = (uint16_t)((float)0x3FFU * f_alpha_even);
                    uint16_t alpha_odd = (uint16_t)((float)0x3FFU * f_alpha_odd);
                    altera_vvp_alpha_channel_generator_write_data(spStitchInputConfig->_spAlphaChGen->GetInstance(), lut_addr, alpha_odd, alpha_even);
                    lut_addr++;
                }
                altera_vvp_alpha_channel_generator_write_data(spStitchInputConfig->_spAlphaChGen->GetInstance(), lut_addr, 0x3FFU, 0x3FFU);
                altera_vvp_alpha_channel_generator_write_data(spStitchInputConfig->_spAlphaChGen->GetInstance(), lut_addr+1, 0x3FFU, 0x3FFU);
            }
            else if(spStitchInputConfig->_spStitchVfr)
            {
                uint16_t* p = (uint16_t*)spStitchInputConfig->_spMask->_data.data();
                for(uint32_t y = 0; y < spStitchInputConfig->_spMask->_height; y++)
                {
                    uint16_t currectValue = 0;
                    bool gradient_down = false;
                    for(uint32_t x = 0; x < spStitchInputConfig->_spMask->_width; x++)
                    {
                        if(x >= spStitchInputConfig->_gradient._start)
                        {
                            if(gradient_down)
                            {
                                currectValue = 0xFFFF - (0xFFFF * (x - spStitchInputConfig->_gradient._start))/(spStitchInputConfig->_gradient._stop - spStitchInputConfig->_gradient._start);
                            }
                            else
                            {
                                currectValue = (0xFFFF * (x - spStitchInputConfig->_gradient._start))/(spStitchInputConfig->_gradient._stop - spStitchInputConfig->_gradient._start);
                            }
                        }
                        if(x == spStitchInputConfig->_gradient._stop)
                        {
                            if(gradient_down)
                            {
                                currectValue = 0;
                                gradient_down = false;
                            }
                            else
                            {
                                currectValue = 0xFFFF;
                                gradient_down = true;
                            }
                        }
                        *p = currectValue;
                        p++;
                    }
                }
                spStitchInputConfig->_spStitchVfr->WriteFrame(*spStitchInputConfig->_spMask, false);
            }
        }
    }

    float StitchAdapter::FindTheta(float radius)
    {
        // radius = a*theta + b*theta^2 + c*theta^3 + c*theta^4
        // so we need to approximate the inverse function
        float theta_min = 0.0;
        float theta_max = M_PI;
        const float epsilon = 0.0001;
        float theta_test = 0.5*M_PI;

        float half_hfov_radians = M_PI*(_hfov/2.0)/180.0;
        float max_radius = _distortion_a*theta_test + 
                           _distortion_b*theta_test*theta_test + 
                           _distortion_c*theta_test*theta_test*theta_test + 
                           _distortion_d*theta_test*theta_test*theta_test*theta_test;

        if(max_radius < radius)
        {
            theta_test = half_hfov_radians + (radius - max_radius)/_focalLength;
        }
        else
        {
            while((theta_max - theta_min) > epsilon)
            {
                theta_test = (theta_max + theta_min)*0.5f;
                float radius_test = _distortion_a*theta_test + 
                                    _distortion_b*theta_test*theta_test + 
                                    _distortion_c*theta_test*theta_test*theta_test + 
                                    _distortion_d*theta_test*theta_test*theta_test*theta_test;
                if(radius_test == radius)
                {
                    return theta_test;
                }
                else if (radius_test < radius)
                {
                    theta_min = theta_test;
                }
                else
                {
                    theta_max = theta_test;
                }
            }
        }
        return theta_test;
    }

} // namespace SwApi