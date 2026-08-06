/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "VideoPipeline/PipelineCore.h"
#include "VideoFrameReader.h"
#include "HapiVvpAlphaChannelGenerator.h"
#include "IMixer.h"
#include "Scaler.h"
#include "WarpAdapter.h"

namespace SwApi
{
    class StitchAdapter
    {
    public:
        using gradient_t = struct {
            uint32_t _start;
            uint32_t _stop;
            bool _start_at_one;
        };

        using tStitchInputConfig = struct
        {
            friend class StitchAdapter;
        public:
            std::shared_ptr<SwApi::VideoFrameReader> _spStitchVfr;
            Hapi::VvpAlphaChannelGeneratorPtr _spAlphaChGen;

            std::shared_ptr<VideoPipeline::PipelineCore> _inputCore;
            std::shared_ptr<SwApi::WarpAdapter> _spWarpAdapter;
        protected:
            std::shared_ptr<struct vfr_frame_t> _spMask;
            gradient_t _gradient;

            float _cameraOffsetAngle;
            uint32_t _sensorInputWidth;
            uint32_t _sensorInputHeight;
            uint32_t _warpInputWidth;
            uint32_t _warpInputHeight;
            float _warpInputX;
            float _warpInputY;
            uint32_t _mixerInputWidth;
            uint32_t _mixerInputHeight;
            uint32_t _mixerInputX;
            uint32_t _mixerInputY;
        };
        
        static std::shared_ptr<StitchAdapter> Create(std::vector<std::shared_ptr<SwApi::StitchAdapter::tStitchInputConfig>>& vspStitchInputConfig, std::shared_ptr<IMixer> spMixer);

        StitchAdapter(std::vector<std::shared_ptr<SwApi::StitchAdapter::tStitchInputConfig>>& vspStitchInputConfig, std::shared_ptr<IMixer> spMixer);
        virtual ~StitchAdapter();

        void SetStitchSensorResolution(uint32_t isp_layer, uint32_t width, uint32_t height);
        void SetStitchWarpResolution(uint32_t isp_layer, uint32_t width, uint32_t height);
        void SetStitchOutputResolution(uint32_t width, uint32_t height);

        void SetOffsetX(int32_t offsetX);
        void SetOffsetY(int32_t offsetY);
        void SetOverlap(uint32_t overlap, bool forceUpdate=false);
        void SetInterCameraAngle(float interCameraAngle);
        void SetHFoV(float hfov);
        void SetDFoV(float hfov);
        void SetFocalLength(float focalLength);
        void SetDistortionFTheta(float distortionFTheta);
        void SetLensDistortionCoef(float a, float b, float c, float d);
        void SetDirectTransform(bool enable);
        void SetLensCorrection(bool enable);
        void SetLensDistortion(bool enable);
        void SetLensDistortionOverride(bool enable);
        void SetCylindricalProjection(bool enable);

        void GetLensDistortionCoef(float& a, float& b, float& c, float& d);
        uint32_t GetLayerOverlap();

        void PauseUpdates(bool pauseUpdates);
    protected:
        static const uint32_t ALL_LAYERS = 0xFFFFFFFFU;
        void UpdateDistortion();
        void UpdateResolutions(uint32_t isp_layer);
        void UpdateMixer();
        void UpdateWarp(uint32_t isp_layer = StitchAdapter::ALL_LAYERS);
        void SetGradient(uint32_t isp_layer, uint32_t start, uint32_t stop);
        void GenerateStitchBlend();
        float FindTheta(float radius);

    private:
        static const uint32_t DEFAULT_INPUT_WIDTH;
        static const uint32_t DEFAULT_INPUT_HEIGHT;
        static const uint32_t DEFAULT_OFFSET_X;
        static const uint32_t DEFAULT_OFFSET_Y;
        static const uint32_t DEFAULT_OVERLAP;
        static const uint32_t ISP_LAYER;

        std::vector<std::shared_ptr<SwApi::StitchAdapter::tStitchInputConfig>> _vspStitchInputConfig;
        std::shared_ptr<IMixer> _spMixer;

        uint32_t _mixerOutputWidth;
        uint32_t _mixerOutputHeight;
        int32_t _offsetX;
        int32_t _offsetY;
        uint32_t _overlap;
        float _interCameraAngle;
        float _hfov;
        float _dfov;
        float _focalLength;
        float _distortionFTheta;
        float _distortion_a;
        float _distortion_b;
        float _distortion_c;
        float _distortion_d;
        bool _directTransform;
        bool _lensCorrection;
        bool _lensDistortion;
        bool _lensDistortionOverride;
        bool _cylindricalProjection;

        bool _pauseUpdates;
    };
} // namespace SwApi