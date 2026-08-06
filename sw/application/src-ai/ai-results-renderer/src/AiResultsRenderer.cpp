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

namespace SwApi 
{
    std::shared_ptr<AiResultsRenderer> AiResultsRenderer::Create(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor)
    {
        return std::make_shared<AiResultsRenderer>(spCoreDLAProcessor);
    }

    AiResultsRenderer::AiResultsRenderer(const std::shared_ptr<SwApi::ICoreDLAIntf>& spCoreDLAProcessor)
    : _keypointThreshold(0.5F),
      _renderResults(true),
      _spDrmHelper(IDrmHelper::GetIDrmHelper()),
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

    void AiResultsRenderer::ResultsHandler(ResultsType resultsType, std::shared_ptr<YoloClassificationResult> results)
    {
        std::lock_guard<std::recursive_mutex> lock(IDrmHelper::GetLVGLMutex());
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
                    _spDrmHelper->FlushPrimary();
                    
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
                    _spDrmHelper->FlushPrimary();
                    
                    lv_obj_delete(out_of_inferences_string);
                }
                break;
            case ResultsType::RESULTS:
                if((results != nullptr) && (_spDrmHelper != nullptr))
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
                    _spDrmHelper->FlushPrimary();
                    
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

} // namespace SwApi
