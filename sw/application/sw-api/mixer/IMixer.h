/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <utility>
#include "HapiVvpMixer.h"
#include "MixerUtils.h"

namespace SwApi
{
    /**
     * High-level interface definition for the VVP Mixer IP
     * 
     * The Mixer IP allows for the combination of multiple video streams into one,
     * and performs this by assigning video streams into layers and rendering them
     * on top of a background stream.
     * 
     * The Mixer allows for each stream where layer_id > 0 to be independently 
     * sized and positioned on the background layer, and to have independent alpha
     * blending.
     * 
     * Layers are enabled and disabled through the use of SetLayerEnableState(),
     * and their resolution/position are assigned through the use of
     * SetLayerResolution(), which specifices width and height, and
     * SetLayerOffsetPosition(), which specifies x and y offset from the top left
     * of the background stream.
     * 
     * The Mixer can be built with support for several blending modes, such as
     * only allowing for opaque layers, assigning a static alpha for the entire
     * layer to allow fading in and out, and finally allowing layers to provide an
     * alpha channel for per-pixel alpha blending. These modes can be set through
     * using SetLayerBlendMode()
     * 
     * If a layer is set onto MixerBlendModeStaticAlpha, which corresponds to a
     * static alpha for the entire layer, the static alpha can be provided via the
     * use of SetLayerStaticAlphaViaUnorm(), which requires the end-user to calculate
     * an unsigned normalised integer, where 0xFFF..FF corresponds to 1.0 and 
     * 0x000..00 corresponds to 0.0, depending on the bits per second the core was 
     * configured to use.
     * Alternatively, SetLayerStaticAlphaViaDouble() allows the user to provide a
     * double precision floating point value between 0.0 and 1.0, and the bps that
     * the core was configured to use. With those values, it will calculate the 
     * required integer and assign it to the layer.
     * 
    */
    struct IMixer
    {
    public:
        static std::shared_ptr<IMixer> Create(Hapi::VvpMixerPtr spMixer);
        virtual ~IMixer() { };

        // Generics used on everything
        virtual bool GetLiteMode() = 0;
        virtual bool GetDebugEnabled() = 0;
        virtual bool GetIsRunning() = 0;
        virtual uint8_t GetStatus() = 0;

        // Core specific

        virtual uint8_t GetMaxLayersSupported() = 0;
        virtual bool SetLayerEnableState(uint8_t layer, MixerLayerState state) = 0;
        virtual MixerLayerState GetLayerEnableState(uint8_t layer) = 0;

        virtual bool SetLayerResolution(uint8_t layer, uint32_t width, uint32_t height) = 0;
        virtual bool GetLayerResolution(uint8_t layer, uint32_t* width, uint32_t* height) = 0;

        virtual bool SetLayerOffsetPosition(uint8_t layer, uint32_t x_offset, uint32_t y_offset) = 0;
        virtual bool GetLayerOffsetPosition(uint8_t layer, uint32_t* x_offset, uint32_t* y_offset) = 0;
        virtual std::pair<uint32_t, uint32_t> GetLayerOffsetPosition(const uint32_t layer) = 0;

        virtual bool SetLayerBlendMode(uint8_t layer, MixerBlendMode blend_mode) = 0;
        virtual MixerBlendMode GetLayerBlendMode(uint8_t layer) = 0;

        virtual bool SetLayerStaticAlphaViaUnorm(uint8_t layer, uint32_t alpha) = 0;
        virtual bool SetLayerStaticAlphaViaDouble(uint8_t layer, uint8_t layer_bps, double alpha) = 0;
        virtual uint32_t GetLayerStaticAlpha(uint8_t layer) = 0;

        virtual bool SetOutputWidth(uint32_t width) = 0;
        virtual uint32_t GetOutputWidth() = 0;

        virtual bool SetOutputHeight(uint32_t height) = 0;
        virtual uint32_t GetOutputHeight() = 0;

        virtual bool CommitWrites() = 0;
    };
} // namespace SwApi
