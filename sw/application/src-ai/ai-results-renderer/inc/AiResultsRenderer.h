/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __AiResultsRenderer_H__
#define __AiResultsRenderer_H__

#include "HapiVvpVfw.h"
#include "HapiVvpVfr.h"
#include "HapiVvpSwitch.h"
#include "ICoreDLAIntf.h"
#include "IspCommon.h"

#include <cstdint>
#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <lvgl.h>

#include <mutex>
#include <condition_variable>
#include <chrono>

#include "IDrmHelper.h"
#include "IAiResultsRenderer.h"

namespace SwApi 
{

    class AiResultsRenderer : public IAiResultsRenderer
    {
        public:
            static std::shared_ptr<AiResultsRenderer> Create(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor);
            AiResultsRenderer(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor);
            virtual ~AiResultsRenderer();
            AiResultsRenderer(const AiResultsRenderer&) = delete;
            AiResultsRenderer(AiResultsRenderer&) = delete;
            AiResultsRenderer& operator=(const AiResultsRenderer&) = delete;

            void Initialise();

            virtual void SetKeypointThreshold(float keypointThreshold) override;
            virtual void RenderResults(bool enable) override;

            void ResultsHandler(ResultsType resultsType, std::shared_ptr<YoloClassificationResult> results);

        private:
            float _keypointThreshold;
            bool _renderResults;

            std::shared_ptr<IDrmHelper> _spDrmHelper;
            std::shared_ptr<SwApi::ICoreDLAIntf> _spCoreDLAProcessor;
    };
} // namespace SwApi

#endif //__AiResultsRenderer_H__

