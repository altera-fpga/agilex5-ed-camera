/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "AiResultsRenderer.h"
#include <chrono>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>


#include <cstdlib>
#include <iomanip>

#include <sys/time.h>
#include <lvgl.h>
#include "lv_labelled_box.h"
#include "lv_skeleton.h"
#include "HapiCoreDLA.h"

#include "AtUtils.h"
#include "CommonTypes.h"

namespace SwApi 
{
    std::shared_ptr<AiResultsRenderer> AiResultsRenderer::Create(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor)
    {
        return std::make_shared<AiResultsRenderer>(spCoreDLAProcessor);
    }

    AiResultsRenderer::AiResultsRenderer(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor)
    : _keypointThreshold(0.5F),
      _renderResults(true),
      _spOverlayHelper(IOverlayHelper::GetIOverlayHelper()),
      _spCoreDLAProcessor(spCoreDLAProcessor)
    {
    }

    AiResultsRenderer::~AiResultsRenderer()
    {
    }

    void AiResultsRenderer::Initialise()
    {
        if(_spCoreDLAProcessor != nullptr)
        {
            _spCoreDLAProcessor->RegisterResultsCallback([this](ResultsType resultsType, std::shared_ptr<YoloClassificationResult> results)->void {
                this->ResultsHandler(resultsType, std::move(results));
            });
        }
    }

    void AiResultsRenderer::SetKeypointThreshold(float keypointThreshold)
    {
        _keypointThreshold = keypointThreshold;
    }

    void AiResultsRenderer::RenderResults(bool enable)
    {
        _renderResults = enable;
    }

    bool AiResultsRenderer::ConnectWebSocketService(IWebSocketService* web_socket)
    {
        std::lock_guard<std::recursive_mutex> lock(_cs_wshandler);
        auto wshandler = std::make_shared<AIResultsWebsocketHandler>(web_socket);

        if(wshandler != nullptr)
        {
            std::shared_ptr<IWebSocketCallback> wscallback = wshandler;
            std::weak_ptr<AIResultsWebsocketHandler> wshandler_weak(wshandler);
            web_socket->SetCallback(wscallback);
            _wsHandlers.emplace_back(wshandler_weak);
        }

        return (wshandler != nullptr);
    }


    void AiResultsRenderer::ResultsHandler(ResultsType resultsType, std::shared_ptr<YoloClassificationResult> results)
    {
        {
            std::lock_guard<std::recursive_mutex> lock(_cs_wshandler);
            if(!_wsHandlers.empty())
            {
                auto resultJson = AtUtils::IJson::Create();
                auto resultObj = resultJson->RootObject();
                resultObj->AddValue("resultsType", static_cast<int>(resultsType));
                if(results != nullptr)
                {
                    resultObj->AddValue("networkHandle", results->GetNetworkHandle());
                    resultObj->AddValue("networkType", static_cast<int>(results->GetNetworkType()));
                    resultObj->AddValue("inferenceCount", results->GetInferenceCount());
                    resultObj->AddValue("inferenceBuffer", results->GetInferenceBuffer());
                    auto itemsArray = resultObj->AddArray("items");

                    switch(resultsType)
                    {
                    case ResultsType::RESULTS:
                        const std::vector<YoloClassificationItem>& items = results->GetItems();

                        if(results->GetNetworkType() == NetworkType::YOLOV8N)
                        {
                            resultObj->AddValue("networkTypeString", "YOLOV8N");
                            for (const auto& item : items)
                            {
                                auto itemValObj = itemsArray->AddElement();
                                auto itemObj = itemValObj->AddObject();
                                itemObj->AddValue("categoryIndex", item._category_index);
                                itemObj->AddValue("categoryName", item._category_name.c_str());
                                std::ostringstream colour;
                                colour << '#' << std::uppercase << std::hex << std::setfill('0')
                                    << std::setw(2) << static_cast<unsigned int>(item._colour[2])
                                    << std::setw(2) << static_cast<unsigned int>(item._colour[1])
                                    << std::setw(2) << static_cast<unsigned int>(item._colour[0]);
                                itemObj->AddValue("colour", colour.str().c_str());
                                itemObj->AddValue("score", item._score);
                                itemObj->AddValue("xMin", item._x_min);
                                itemObj->AddValue("yMin", item._y_min);
                                itemObj->AddValue("xMax", item._x_max);
                                itemObj->AddValue("yMax", item._y_max);
                            }
                        }
                        else
                        {
                            resultObj->AddValue("networkTypeString", "YOLOV8N_POSE");
                            for (auto& item : items)
                            {
                                auto itemValObj = itemsArray->AddElement();
                                auto itemObj = itemValObj->AddObject();
                                itemObj->AddValue("categoryIndex", item._category_index);
                                itemObj->AddValue("categoryName", item._category_name.c_str());
                                std::ostringstream colour;
                                colour << '#' << std::uppercase << std::hex << std::setfill('0')
                                    << std::setw(2) << static_cast<unsigned int>(item._colour[2])
                                    << std::setw(2) << static_cast<unsigned int>(item._colour[1])
                                    << std::setw(2) << static_cast<unsigned int>(item._colour[0]);
                                itemObj->AddValue("colour", colour.str().c_str());
                                itemObj->AddValue("score", item._score);
                                itemObj->AddValue("xMin", item._x_min);
                                itemObj->AddValue("yMin", item._y_min);
                                itemObj->AddValue("xMax", item._x_max);
                                itemObj->AddValue("yMax", item._y_max);
                                auto keypointsArray = itemObj->AddArray("keypoints");
                                for(auto& keypoint : item._keypoints)
                                {
                                    auto keypointValObj = keypointsArray->AddElement();
                                    auto keypointObj = keypointValObj->AddObject();
                                    keypointObj->AddValue("x", keypoint._x);
                                    keypointObj->AddValue("y", keypoint._y);
                                    keypointObj->AddValue("visibility", keypoint._v);
                                }
                            }
                        }
                    }
                }
                auto resultJsonStr = resultJson->ToString();
                size_t handler_index = 0;
                while( handler_index < _wsHandlers.size())
                {
                    if(auto spHandler = _wsHandlers[handler_index].lock())
                    {
                        spHandler->SendResults(resultJsonStr);
                        ++handler_index;
                    }
                    else
                    {
                        _wsHandlers.erase(_wsHandlers.begin() + handler_index);
                    }
                }
            }
        }
        {
            std::lock_guard<std::recursive_mutex> lock(IOverlayHelper::GetLVGLMutex());
            if(lv_screen_active() == nullptr)
            {
                return;
            }

            switch(resultsType)
            {
                case ResultsType::NO_NETWORK:
                    {
                        lv_obj_t* no_networks_found_string;
                        no_networks_found_string = lv_label_create(lv_screen_active());
                        lv_label_set_text(no_networks_found_string, "No AI models found!\nFollow instructions to compile AI models and update SD card.");

                        lv_color_t color;
                        color.red = 0xFFU;
                        color.green = 0;
                        color.blue = 0;

                        lv_color_t bg_color;
                        bg_color.red = 0x40U;
                        bg_color.green = 0x40U;
                        bg_color.blue = 0x40U;

                        static lv_style_t style;
                        lv_style_init(&style);
                        lv_style_set_bg_opa(&style, LV_OPA_COVER);
                        lv_style_set_bg_color(&style, bg_color);
                        lv_style_set_text_color(&style, color);
                        lv_style_set_text_font(&style, &lv_font_montserrat_20);
                        lv_obj_add_style(no_networks_found_string, &style, 0);
                        lv_obj_center(no_networks_found_string);
                        _spOverlayHelper->FlushPrimary();
                        
                        lv_obj_delete(no_networks_found_string);
                    }
                    break;
                case ResultsType::OUT_OF_INFERENCES:
                    {
                        lv_obj_t* out_of_inferences_string;
                        out_of_inferences_string = lv_label_create(lv_screen_active());
                        lv_label_set_text(out_of_inferences_string, "Unlicensed FPGA AI suite\nOut of free inferences on this IP until reboot.");

                        lv_color_t color;
                        color.red = 0xFFU;
                        color.green = 0;
                        color.blue = 0;

                        lv_color_t bg_color;
                        bg_color.red = 0x40U;
                        bg_color.green = 0x40U;
                        bg_color.blue = 0x40U;

                        static lv_style_t style;
                        lv_style_init(&style);
                        lv_style_set_bg_opa(&style, LV_OPA_COVER);
                        lv_style_set_bg_color(&style, bg_color);
                        lv_style_set_text_color(&style, color);
                        lv_style_set_text_font(&style, &lv_font_montserrat_20);
                        lv_obj_add_style(out_of_inferences_string, &style, 0);
                        lv_obj_center(out_of_inferences_string);
                        _spOverlayHelper->FlushPrimary();
                        
                        lv_obj_delete(out_of_inferences_string);
                    }
                    break;
                case ResultsType::RESULTS:
                    if((results != nullptr) && (_spOverlayHelper != nullptr))
                    {
                        const std::vector<YoloClassificationItem>& items = results->GetItems();
                        size_t count = 0;
                        size_t lv_count = 0;
                        if(results->GetNetworkType() == NetworkType::YOLOV8N)
                        {
                            lv_count = std::min(static_cast<size_t>(100U), items.size());
                        }
                        else
                        {
                            lv_count = std::min(static_cast<size_t>(25U), items.size());
                        }
                        std::vector<lv_obj_t*> lv_objs(lv_count);

                        if(_renderResults)
                        {
                            if(results->GetNetworkType() == NetworkType::YOLOV8N)
                            {
                                for (const auto& item : items)
                                {
                                    lv_obj_t*& box = lv_objs[count];
                                    box = labelled_box_create(lv_screen_active(), item);

                                    // std::cout << item._category_name.c_str() << ":" << item._score << " x_min:" << item._x_min << " y_min:" << item._y_min << " x_max:" << item._x_max << " y_max:" << item._y_max << std::endl;

                                    count++;
                                    if(count == lv_count)
                                    {
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                for (auto& item : items)
                                {
                                    lv_obj_t*& s = lv_objs[count];
                                    s = lv_skeleton_create(lv_screen_active(), item, _keypointThreshold);
                                    count++;
                                    if(count == lv_count)
                                    {
                                        break;
                                    }
                                }
                            }
                        }
                        _spOverlayHelper->FlushPrimary();
                        
                        for(size_t i = 0U; i < count; i++)
                        {
                            lv_obj_delete(lv_objs[i]);
                        }
                        lv_objs.clear();
                    }
                    break;
                default:
                    break;
            }
        }
    }

    AIResultsWebsocketHandler::AIResultsWebsocketHandler(IWebSocketService* pIWebSocket)
    :	IWebSocketCallback(pIWebSocket)
    ,	_inDestructor(false)
    {
    }

    AIResultsWebsocketHandler::~AIResultsWebsocketHandler()
    {
    }

    void AIResultsWebsocketHandler::Shutdown()
    {
        {
            std::lock_guard lock(_destructorLock);
            _inDestructor = true;
        }
    }

    void AIResultsWebsocketHandler::Ready()
    {
    }

    void AIResultsWebsocketHandler::SendResults(std::string& resultsJsonStr)
    {
        std::lock_guard lock(_destructorLock);
        if(!_inDestructor)
        {
            SendToClient(new TextWebSocketMessage(resultsJsonStr.c_str(), resultsJsonStr.length()));
        }
    }

    void AIResultsWebsocketHandler::ReceiveWebSocketMessage(IWebSocketMessage* message)
    {
        {
            std::lock_guard lock(_destructorLock);
            if (_inDestructor)
            {
                std::cout << "AIResultsWebsocketHandler::ReceiveWebSocketMessage received when ~AIResultsWebsocketHandler called\n";
                return;
            }
        }

        std::shared_ptr<IWebSocketCallback> spBase = shared_from_this();
        std::shared_ptr<AIResultsWebsocketHandler> spMe = std::dynamic_pointer_cast<AIResultsWebsocketHandler>(spBase);
        std::weak_ptr<AIResultsWebsocketHandler> wpMe(spMe);

        bool isText = true;
        bool finalFrame = true;
        std::vector<std::shared_ptr<ByteArray>> data = message->GetData(isText, finalFrame);
        if (data.empty())
            return;

        if (isText)
        {
            for (auto spData : data)
            {
                std::string text = (char*)spData->data();
                auto spCommandDoc = AtUtils::IJson::Create(text);

                if (!spCommandDoc)
                    continue;

                auto spJsonObject = spCommandDoc->Parse();

                // If the document is empty skip it
                if (!spJsonObject)
                    continue;

                // process json data
            }
        }
        else
        {
            // Received binary block
            // Your derived class implements Process. Do this on the CoreProcessor thread
        }
    }

} // namespace SwApi
