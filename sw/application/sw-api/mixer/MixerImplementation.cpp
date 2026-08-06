/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "MixerImplementation.h"
#include "intel_vvp_mixer.h"

namespace SwApi
{
    std::shared_ptr<IMixer> IMixer::Create(Hapi::VvpMixerPtr spMixer)
    {
        return std::make_shared<Mixer::MixerImplementation>(spMixer);
    }

    namespace Mixer
    {
        Mixer::MixerImplementation::MixerImplementation(Hapi::VvpMixerPtr spVvpMixer):
            _spVvpMixer(spVvpMixer),
            _outputWidth{0},
            _outputHeight{0}
        {
            max_layers_supported = GetMaxLayersSupported();
            _layerStates.resize(max_layers_supported-1);

            for (int i = 0; i < max_layers_supported - 1; i++)
            {
                _layerStates[i].enable_state = MixerLayerState::MixerLayerDisabled;
                _layerStates[i].blend_mode = MixerBlendMode::MixerBlendModeTransparent;
                _layerStates[i].x_resolution = 0xFFFFFFFF;
                _layerStates[i].y_resolution = 0xFFFFFFFF;
                _layerStates[i].x_offset = 0xFFFFFFFF;
                _layerStates[i].y_offset = 0xFFFFFFFF;
                _layerStates[i].static_alpha = 0xFFFFFFFF;
            }
        }

        bool MixerImplementation::GetLiteMode()
        {
            return intel_vvp_mixer_get_lite_mode(_spVvpMixer->GetInstance());
        }

        bool MixerImplementation::GetDebugEnabled()
        {
            return intel_vvp_mixer_get_debug_enabled(_spVvpMixer->GetInstance());
        }

        bool MixerImplementation::GetIsRunning()
        {
            return intel_vvp_mixer_is_running(_spVvpMixer->GetInstance());
        }

        uint8_t MixerImplementation::GetStatus()
        {
            return intel_vvp_mixer_get_status(_spVvpMixer->GetInstance());
        }

        uint8_t MixerImplementation::GetMaxLayersSupported()
        {
            return intel_vvp_mixer_get_num_layers(_spVvpMixer->GetInstance());
        }

         bool MixerImplementation::SetLayerEnableState(uint8_t layer, MixerLayerState state)
        {
            // Layer 0 is the background layer, and cannot be disabled or modified.
            if (layer > max_layers_supported || layer == 0) return false;

            int error_code;

            switch(state)
            {
                case MixerLayerState::MixerLayerDisabled:
                    error_code =  intel_vvp_mixer_set_input_mode(_spVvpMixer->GetInstance(), layer, false, false, false);
                    break;
                case MixerLayerState::MixerLayerConsumed:
                    error_code =  intel_vvp_mixer_set_input_mode(_spVvpMixer->GetInstance(), layer, true, true, false);
                    break;
                case MixerLayerState::MixerLayerEnabled:
                    error_code =  intel_vvp_mixer_set_input_mode(_spVvpMixer->GetInstance(), layer, true, false, false);
                    break;
                case MixerLayerState::MixerLayerEnabledSoftStart:
                    error_code =  intel_vvp_mixer_set_input_mode(_spVvpMixer->GetInstance(), layer, true, false, true);
                    break;
                case MixerLayerState::MixerLayerConsumedEnabledSoftStart:
                    error_code =  intel_vvp_mixer_set_input_mode(_spVvpMixer->GetInstance(), layer, true, true, true);
                    break;                    
                case MixerLayerState::MixerLayerInvalid:
                default:
                    return false;

                if (error_code != kIntelVvpCoreOk)
                {
                    return false;
                }
            }
            _layerStates[layer - 1].enable_state = state;

            CommitWrites();

            return true;
        }

        MixerLayerState MixerImplementation::GetLayerEnableState(uint8_t layer)
        {
            if (layer == 0) return MixerLayerState::MixerLayerEnabled;
            if (layer > max_layers_supported) return MixerLayerState::MixerLayerInvalid;

            return _layerStates[layer - 1].enable_state;
        }

        bool MixerImplementation::SetLayerResolution(uint8_t layer, uint32_t width, uint32_t height)
        {
            // Layer 0 is the background layer, and cannot be disabled or modified.
            if (layer > max_layers_supported || layer == 0) return false;

            int error_code;

            error_code = intel_vvp_mixer_set_width(_spVvpMixer->GetInstance(), layer, width);

            if (error_code != kIntelVvpCoreOk) return false;

            error_code = intel_vvp_mixer_set_height(_spVvpMixer->GetInstance(), layer, height);

            if (error_code != kIntelVvpCoreOk) return false;

            _layerStates[layer - 1].x_resolution = width;
            _layerStates[layer - 1].y_resolution = height;

            CommitWrites();

            return true;
        }

        bool MixerImplementation::GetLayerResolution(uint8_t layer, uint32_t* width, uint32_t* height)
        {
            // Layer 0 is the background layer, and cannot be disabled or modified.
            if (layer > max_layers_supported || layer == 0) return false;
            if (width == nullptr || height == nullptr) return false;

            if (_layerStates[layer - 1].x_resolution == 0xFFFFFFFF ||
                _layerStates[layer - 1].y_resolution == 0xFFFFFFFF)
            {
                return false;
            }

            *width  = _layerStates[layer - 1].x_resolution;
            *height = _layerStates[layer - 1].y_resolution;

            return true;
        }

        bool MixerImplementation::SetLayerOffsetPosition(uint8_t layer, uint32_t x_offset, uint32_t y_offset)
        {
            // Layer 0 is the background layer, and cannot be disabled or modified.
            if (layer > max_layers_supported || layer == 0) return false;

            int error_code;

            uint32_t x_offset_local = x_offset;
            do {
                error_code = intel_vvp_mixer_set_horiz_offset(_spVvpMixer->GetInstance(), layer, x_offset_local);
                x_offset_local++;
                // Scan for nearest valid offset match
                if (x_offset_local > (x_offset + 100))
                    return false;
            } while (error_code != kIntelVvpCoreOk);
            
            error_code = intel_vvp_mixer_set_vert_offset(_spVvpMixer->GetInstance(), layer, y_offset);

            if (error_code != kIntelVvpCoreOk) return false;

            _layerStates[layer - 1].x_offset = x_offset;
            _layerStates[layer - 1].y_offset = y_offset;

            CommitWrites();
            return true;
        }

        bool MixerImplementation::GetLayerOffsetPosition(uint8_t layer, uint32_t* x_offset, uint32_t* y_offset)
        {
            // Layer 0 is the background layer, and cannot be disabled or modified.
            if (layer > max_layers_supported || layer == 0) return false;
            if (x_offset == nullptr || y_offset == nullptr) return false;

            if (_layerStates[layer - 1].x_offset == 0xFFFFFFFF ||
                _layerStates[layer - 1].y_offset == 0xFFFFFFFF)
            {
                return false;
            }

            *x_offset = _layerStates[layer - 1].x_offset;
            *y_offset = _layerStates[layer - 1].y_offset;

            return true;
        }

        std::pair<uint32_t, uint32_t> MixerImplementation::GetLayerOffsetPosition(const uint32_t layer)
        {
            auto rv = std::make_pair<uint32_t, uint32_t>(0, 0);

            if(layer <= max_layers_supported)
            {
                rv.first = (_layerStates[layer - 1].x_offset < 0xffffffff ? _layerStates[layer - 1].x_offset : 0);
                rv.second = (_layerStates[layer - 1].y_offset < 0xffffffff ? _layerStates[layer - 1].y_offset : 0);
            }

            return rv;
        }

        bool MixerImplementation::SetLayerBlendMode(uint8_t layer, MixerBlendMode blend_mode)
        {
            if (layer > max_layers_supported || layer == 0) return false;

            int error_code = intel_vvp_mixer_set_blend_mode(_spVvpMixer->GetInstance(), layer, static_cast<eIntelVvpBlendMode>(blend_mode));

            if (error_code != kIntelVvpCoreOk)
            {
                return false;
            }

            _layerStates[layer - 1].blend_mode = blend_mode;

            CommitWrites();

            return  true;
        }

        MixerBlendMode MixerImplementation::GetLayerBlendMode(uint8_t layer)
        {
            return _layerStates[layer - 1].blend_mode;
        }

        bool MixerImplementation::SetLayerStaticAlphaViaUnorm(uint8_t layer, uint32_t alpha)
        {
            if (layer > max_layers_supported || layer == 0) return false;

            int error_code = intel_vvp_mixer_set_static_alpha(_spVvpMixer->GetInstance(), layer, alpha);

            if (error_code != kIntelVvpCoreOk) return false;

            _layerStates[layer - 1].static_alpha = alpha;

            CommitWrites();

            return true;
        }

        bool MixerImplementation::SetLayerStaticAlphaViaDouble(uint8_t layer, uint8_t layer_bps, double alpha)
        {
            if (layer > max_layers_supported || layer == 0) return false;
            if (alpha > 1.0) alpha = 1.0;
            if (alpha < 0.0) alpha = 0.0;
            if (layer_bps < 4 || layer_bps > 20) return false;

            int error_code;

            uint32_t unorm_alpha;

            unorm_alpha = (0x1 << (layer_bps)) - 1;
            unorm_alpha *= alpha;

            error_code = intel_vvp_mixer_set_static_alpha(_spVvpMixer->GetInstance(), layer, unorm_alpha);

            if (error_code != kIntelVvpCoreOk) return false;

            _layerStates[layer - 1].static_alpha = unorm_alpha;

            CommitWrites();

            return  true;
        }

        uint32_t MixerImplementation::GetLayerStaticAlpha(uint8_t layer)
        {
            return _layerStates[layer - 1].static_alpha;
        }

        bool MixerImplementation::SetOutputWidth(uint32_t newWidth) 
        {
            if (intel_vvp_core_set_img_info_width(_spVvpMixer->GetInstance(), newWidth) == kIntelVvpCoreOk) 
            {
                _outputWidth = newWidth;
                CommitWrites();
                return true;
            }

            return false;
        }

        uint32_t MixerImplementation::GetOutputWidth() 
        {
            return _outputWidth;
        }

        bool MixerImplementation::SetOutputHeight(uint32_t newHeight) 
        {
            if (intel_vvp_core_set_img_info_height(_spVvpMixer->GetInstance(), newHeight) == kIntelVvpCoreOk) 
            {
                _outputHeight = newHeight;
                CommitWrites();
                return true;
            }

            return false;
        }

        uint32_t MixerImplementation::GetOutputHeight() 
        {
            return _outputHeight;
        }

        bool MixerImplementation::CommitWrites() {

            if (kIntelVvpCoreOk == intel_vvp_mixer_commit_writes(_spVvpMixer->GetInstance()))
                return true;
            else 
                return false;
        }

    } // namespace Mixer
}; // namespace SwApi