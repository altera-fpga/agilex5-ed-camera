/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <cmath>
#include <ranges>
#include "Lut1D.h"
#include "Lut1dUtils.h"
#include "intel_vvp_1d_lut.h"


namespace SwApi
{
    std::shared_ptr<Lut1D> Lut1D::Create(Hapi::Vvp1dLutPtr spLut1D)
    {
        return std::make_shared<Lut1D>(spLut1D);
    }

    Lut1D::Lut1D(Hapi::Vvp1dLutPtr spVvpLut1D):
        _spVvpLut1D{spVvpLut1D},
        _width{0},
        _height{0},
        _spMessageQueue{std::make_shared<SwUtils::MessageQueue>("1DLutMessageQ")}
    {
        _bitsIn = intel_vvp_1d_lut_get_bits_per_sample_in(_spVvpLut1D->GetInstance());
        _bitsOut = intel_vvp_1d_lut_get_bits_per_sample_out(_spVvpLut1D->GetInstance());
        _bitsLut = intel_vvp_1d_lut_get_bits_lut(_spVvpLut1D->GetInstance());

        // Create default gamma layer
        LutLayer layer = { true, {TransferFunctionType::OETF, TransferFunctionCurve::Gamma, 1.0f, 1.0f, 1.0f, {}, {}} };
        AddLutLayer(layer);
        LutLayer customLayer = { false, {TransferFunctionType::OETF, TransferFunctionCurve::Custom, 1.0f, 1.0f, 1.0f, {}, {}} };
        AddLutLayer(customLayer);
        UploadLUTs(); // Initialise LUT
    }

    std::pair<uint32_t, uint32_t> Lut1D::GetResolution() const
    {
        return {_width, _height};
    }

    bool Lut1D::SetResolution(uint32_t width, uint32_t height)
    {
        //std::scoped_lock lock(_hwMutex);

        bool ret = true;

        std::scoped_lock hwLock{_hwMutex};

        ret = ret && (intel_vvp_core_set_img_info_width(_spVvpLut1D->GetInstance(), width) == kIntelVvpCoreOk);
        ret = ret && (intel_vvp_core_set_img_info_height(_spVvpLut1D->GetInstance(), height) == kIntelVvpCoreOk);

        if(ret)
        {
            _width = width;
            _height = height;
        }

        return ret;
    }    

    bool Lut1D::SetBypass(bool bypass)
    {
        std::scoped_lock hwLock{_hwMutex};
        return (intel_vvp_1d_lut_set_bypass(_spVvpLut1D->GetInstance(), bypass)) == kIntelVvpCoreOk;
    }

    bool Lut1D::GetBypass() const
    {
        return intel_vvp_1d_lut_get_bypass(_spVvpLut1D->GetInstance());
    }

    bool Lut1D::GetFrameStats(uint32_t *stats_out) const
    {
        return intel_vvp_1d_lut_get_frame_stats(_spVvpLut1D->GetInstance(), stats_out) == kIntelVvpCoreOk;
    }

    uint32_t Lut1D::GetBitsIn() const
    {
        return _bitsIn;
    }

    uint32_t Lut1D::GetBitsOut() const
    {
        return _bitsOut;
    }

    uint32_t Lut1D::GetBitsLut() const
    {
        return _bitsLut;
    }

    void Lut1D::AddLutLayer(const LutLayer& layer)
    {
        _lutLayers.push_back(layer);
    }

    void Lut1D::RemoveLutLayer(uint32_t id)
    {
        if (id < _lutLayers.size()) // If layer exists
        {
            _lutLayers.erase(_lutLayers.begin() + id);
        }
    }

    void Lut1D::Clear()
    {
        _lutLayers.clear();
    }

    uint32_t Lut1D::GetNumLayers()
    {
        return _lutLayers.size();
    }


    LutLayer Lut1D::GetLutLayer(uint32_t id)
    {
        if (id < _lutLayers.size()) // If layer exists
        {
            return _lutLayers[id];
        }
        return {true, {TransferFunctionType::OETF, TransferFunctionCurve::Gamma, 0, 0, 0, {}, {}}};
    }

    void Lut1D::SetLutLayer(const uint32_t& id, const LutLayer& layer)
    {
        if (id < _lutLayers.size()) // If layer exists
        {
            _lutLayers[id] = layer;
        }
    }

    void Lut1D::EnableLayer(const uint32_t& layer, const bool& enable)
    {
        if (layer < _lutLayers.size())
        {
            _lutLayers[layer].enable = enable;
        }
    }

    void Lut1D::SetTransferFunction(const uint32_t& layer, const TransferFunctionType& type)
    {
        if (layer < _lutLayers.size()) // If layer exists
        {
            _lutLayers[layer].params._type = type;
        }
    }

    void Lut1D::SetTransferCurve(const uint32_t& layer, const TransferFunctionCurve& curve)
    {
        if (layer < _lutLayers.size()) // If layer exists
        {
            _lutLayers[layer].params._curve = curve;
        }
    }

    void Lut1D::SetGamma(const uint32_t& layer, const FloatT& gamma)
    {
        if (layer < _lutLayers.size()) // If layer exists
        {
            _lutLayers[layer].params._gamma = gamma;
        }
    }

    void Lut1D::SetBlackLum(const uint32_t& layer, const FloatT& blackLum)
    {
        if (layer < _lutLayers.size()) // If layer exists
        {
            _lutLayers[layer].params._blackLum = blackLum;
        }
    }

    void Lut1D::SetWhiteLum(const uint32_t& layer, const FloatT& whiteLum)
    {
        if (layer < _lutLayers.size()) // If layer exists
        {
            _lutLayers[layer].params._whiteLum = whiteLum;
        }
    }

    void Lut1D::SetKnots(const uint32_t& layer, const std::vector<std::pair<FloatT, FloatT>>& knots)
    {
        if (layer < _lutLayers.size()) // If layer exists
        {
            _lutLayers[layer].params._knots = knots;
        }
    }

    void Lut1D::SetControlPoints(const uint32_t& layer, const std::vector<std::pair<FloatT, FloatT>>& controlPoints)
    {
        if (layer < _lutLayers.size()) // If layer exists
        {
            _lutLayers[layer].params._controlPoints = controlPoints;
        }
    }

    void Lut1D::UploadLUTsPrivate(ltf_t ltf)
    {
        const uint32_t bits_in = GetBitsIn();
        const uint32_t bits_out = GetBitsOut();
        const uint32_t bits_lut = GetBitsLut();

        Lut1d lut = Generate1dLut(ltf, bits_in, bits_out, bits_lut);

        uint32_t scaled_lut[lut._lut.size() * 3];
        uint32_t entry;

        for(std::size_t i = 0; i < lut._lut.size(); ++i)
        {
            entry = (uint32_t)lut._lut_shifted[i] << bits_in;
            entry |= (uint32_t)lut._lut[i];
            scaled_lut[i] = entry; // B
            scaled_lut[i + lut._lut.size()] = entry; // G
            scaled_lut[i + 2 * lut._lut.size()] = entry; // R
        }

        std::scoped_lock hwLock{_hwMutex};

        int rv = intel_vvp_1d_lut_write_data_array(_spVvpLut1D->GetInstance(), scaled_lut, lut._lut.size() * 3);

        if(rv != kIntelVvpCoreOk)
        {
            std::cerr << "Failed to write LUT data array to hardware" << std::endl;
        }
    }

    bool Lut1D::UploadLUTs()
    {
        bool ret = false;

        std::size_t layer_idx = 0;

        for(const auto& layer: std::views::reverse(_lutLayers))
        {
            if(layer.enable)
                break;
            ++layer_idx;
        }

        if(layer_idx < _lutLayers.size())
        {
            layer_idx = _lutLayers.size() - layer_idx - 1;

            auto ltf = LtfFactory(_lutLayers[layer_idx].params);

            auto doUploadLuts = [this, ltf = std::move(ltf)](){
                UploadLUTsPrivate(std::move(ltf));
            };

            _spMessageQueue->FlushMessages();
            _spMessageQueue->Add(new SwUtils::SingleCallbackCmd(doUploadLuts));

            ret = true;
        }

        return ret;
    }
} // namespace SwApi