/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include "IMixer.h"

namespace SwApi
{
    namespace Mixer
    {

        class MixerImplementation : public IMixer
        {
        public:
            MixerImplementation(Hapi::VvpMixerPtr spMixer);

            // Generics used on everything
            bool GetLiteMode() override;
            bool GetDebugEnabled() override;
            bool GetIsRunning() override;
            uint8_t GetStatus() override;

            // Core specific

            uint8_t GetMaxLayersSupported() override;
            bool SetLayerEnableState(uint8_t layer, MixerLayerState state) override;
            MixerLayerState GetLayerEnableState(uint8_t layer) override;

            bool SetLayerResolution(uint8_t layer, uint32_t width, uint32_t height) override;
            bool GetLayerResolution(uint8_t layer, uint32_t* width, uint32_t* height) override;

            bool SetLayerOffsetPosition(uint8_t layer, uint32_t x_offset, uint32_t y_offset) override;
            bool GetLayerOffsetPosition(uint8_t layer, uint32_t* x_offset, uint32_t* y_offset) override;
            virtual std::pair<uint32_t, uint32_t> GetLayerOffsetPosition(const uint32_t layer) override;

            bool SetLayerBlendMode(uint8_t layer, MixerBlendMode blend_mode) override;
            MixerBlendMode GetLayerBlendMode(uint8_t layer) override;

            bool SetLayerStaticAlphaViaUnorm(uint8_t layer, uint32_t alpha) override;
            bool SetLayerStaticAlphaViaDouble(uint8_t layer, uint8_t layer_bps, double alpha) override;
            uint32_t GetLayerStaticAlpha(uint8_t layer) override;

            // Lite mode only
            bool SetOutputWidth(uint32_t width) override;
            uint32_t GetOutputWidth() override;

            bool SetOutputHeight(uint32_t height) override;
            uint32_t GetOutputHeight() override;

            bool CommitWrites() override;
        private:

            Hapi::VvpMixerPtr _spVvpMixer;

            struct layer_state
            {
                MixerLayerState enable_state;
                MixerBlendMode blend_mode;
                uint32_t x_resolution;
                uint32_t y_resolution;
                uint32_t x_offset;
                uint32_t y_offset;
                uint32_t static_alpha;
            };

            std::vector<struct layer_state> _layerStates;

            uint8_t max_layers_supported;

            uint32_t _outputWidth;
            uint32_t _outputHeight;

        };

    } // namespace Mixer
} // namespace SwApi