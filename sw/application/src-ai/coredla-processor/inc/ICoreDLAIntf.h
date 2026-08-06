/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#pragma once

#include <cstdint>
#include <functional>
#include <vector>
#include <memory>
#include <string>
#include "HapiOCS.h"
#include "YoloClassificationResult.h"
#include "IInputCallback.h"
#include "stream_controller_userMessages.h"
#include "ICoreDlaRuntime.h"
#include "InferenceTypes.h"

namespace SwApi 
{
    struct CoreDLABox
    {
        uint32_t x0;
        uint32_t y0;
        uint32_t width;
        uint32_t height;

        std::string text;
    };

    enum class ResultsType{
        RESULTS,
        OUT_OF_INFERENCES,
        NO_NETWORK
    };
    using ICoreDLAResultsCallback = std::function<void(ResultsType, std::shared_ptr<YoloClassificationResult>)>;
    using ICoreDLABufferAddressCallback = std::function<void(uint32_t, uint32_t)>;
    
    class ICoreDLAIntf: public ICoreDlaRuntime
    {
        public:
            static std::shared_ptr<ICoreDLAIntf> Create(std::shared_ptr<Hapi::IHapi> spHapi, NetworkType network_type = NetworkType::YOLOV8N);

            virtual void LoadNetwork() = 0;
            virtual void Start() = 0;
            virtual void Stop() = 0;
            
            virtual void RegisterResultsCallback(ICoreDLAResultsCallback resultsCallback) = 0;

            virtual bool SendUserMessage(UserMessageType messageType, void* pPayload=nullptr, uint32_t size=0, UserPayload* pUserPayloadResults=nullptr) = 0;

            virtual std::string GetStatus() = 0;
    };

} // namespace SwApi
