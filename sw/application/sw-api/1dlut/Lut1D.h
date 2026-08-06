/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <functional>
#include "HapiVvp1dLut.h"
#include "Lut1dUtils.h"
#include "SwUtils.h"

namespace SwApi
{

    using namespace Lut1dUtils;

    struct LutLayer
    {
        bool enable;
        Lut1dParams params;
    };

    class Lut1D
    {
    public:
        static std::shared_ptr<Lut1D> Create(Hapi::Vvp1dLutPtr sp1Dlut);

        Lut1D(Hapi::Vvp1dLutPtr spLut1D);
        ~Lut1D() = default;

        bool SetResolution(uint32_t width, uint32_t height);
        std::pair<uint32_t, uint32_t> GetResolution() const;

        bool SetBypass(bool bypass);
        bool GetBypass() const;

        void EnableLayer(const uint32_t& layer, const bool& enable);

        void SetKnots(const uint32_t& layer, const std::vector<std::pair<FloatT, FloatT>>& knots);
        void SetControlPoints(const uint32_t& layer, const std::vector<std::pair<FloatT, FloatT>>& controlPoints);
        void SetWhiteLum(const uint32_t& layer, const FloatT& whiteLum);
        void SetBlackLum(const uint32_t& layer, const FloatT& blackLum);
        void SetGamma(const uint32_t& layer, const FloatT& gamma);
        void SetTransferFunction(const uint32_t& layer, const TransferFunctionType& type);
        void SetTransferCurve(const uint32_t& layer, const TransferFunctionCurve& curve);

        bool UploadLUTs();

        bool GetFrameStats(uint32_t *stats_out) const;

    private:
        // Core specific
        uint32_t GetBitsIn() const;
        uint32_t GetBitsOut() const;
        uint32_t GetBitsLut() const;

        void AddLutLayer(const LutLayer& layer);
        void RemoveLutLayer(uint32_t layer);
        void Clear();
        uint32_t GetNumLayers();

        LutLayer GetLutLayer(uint32_t id);
        void SetLutLayer(const uint32_t& id, const LutLayer& layer);

        void UploadLUTsPrivate(ltf_t ltf);

        Hapi::Vvp1dLutPtr _spVvpLut1D;

        uint32_t _bitsIn;
        uint32_t _bitsOut;
        uint32_t _bitsLut;

        std::vector<LutLayer> _lutLayers;

        uint32_t _width;
        uint32_t _height;

        std::mutex _hwMutex;
        std::shared_ptr<SwUtils::MessageQueue> _spMessageQueue;
    };
} // namespace SwApi
