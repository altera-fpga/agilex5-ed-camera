/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <sstream>
#include <iomanip>
#include "YoloPoseProcessor.h"
#include "OpenVinoIE.h"
#include "FrameQueueConfig.h"
#include <math.h>

extern std::string _bin_directory;

YoloPoseProcessor::YoloPoseProcessor(const ModelPtr& network_model, bool bypass_output_layout_transform)
    : _bypass_output_layout_transform(bypass_output_layout_transform)
    , _detection_threshold(0.5f)
    , _iou_threshold(0.85f)
    , _input_tensor_h(0U)
    , _input_tensor_w(0U)
{
    _params._original_im_w = SwApi::FQ::OVERLAY_WIDTH;
    _params._original_im_h = SwApi::FQ::OVERLAY_HEIGHT;
    _params._coords = 4;
    _params._classes = 1;
    _params._keypoints = 17;

    if(network_model)
    {
        ov::Output<const ov::Node> input_port = network_model->_ovCompiledModel.input();
        
        _input_tensor_h = input_port.get_shape()[2];
        _input_tensor_w = input_port.get_shape()[3];

    }
    _params.init(network_model);
    _objects.resize(_params._output_tensor_predictions);
}

YoloPoseProcessor::~YoloPoseProcessor()
{
}

// Called on CoreProcessor thread
std::shared_ptr<YoloClassificationResult> YoloPoseProcessor::ProcessResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput)
{
    std::shared_ptr<YoloClassificationResult> classification_result = GetClassificationResults(spCoreDLAOutput);
#if 0
    const std::vector<YoloClassificationItem>& items = classification_result->GetItems();
    std::cout << "Num Results " << items.size() << std::endl;
    for (auto& item : items)
    {
        std::cout << "person:" << item._score << " x_min:" << item._x_min << " y_min:" << item._y_min << " x_max:" << item._x_max << " y_max:" << item._y_max << std::endl;
        for(auto& k : item._keypoints)
        {
            std::cout << "    " << ": x:" << k._x << " y:" << k._y << " v:" << k._v << std::endl;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
#endif
    return classification_result;
}

///////////////////////////////////////////////////////////////////////////////


void YoloPoseProcessor::SetDetectionThreshold(float threshold)
{
    _detection_threshold = threshold;
}

void YoloPoseProcessor::SetIOUThreshold(float threshold)
{
    _iou_threshold = threshold;
}

std::shared_ptr<YoloClassificationResult> YoloPoseProcessor::GetClassificationResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput)
{
    _confidenceMap.clear();
    ParseYOLOOutput(spCoreDLAOutput->output_tensors);
    // Filtering overlapping boxes
    auto itConfidenceMap = _confidenceMap.begin();
    while (itConfidenceMap != _confidenceMap.end())
    {
        auto itConfidenceMapCompare = itConfidenceMap;
        itConfidenceMapCompare++;
        while (itConfidenceMapCompare != _confidenceMap.end())
        {
            float iou = IntersectionOverUnion(_objects[itConfidenceMap->second], _objects[itConfidenceMapCompare->second]);
            if (iou >= _iou_threshold)
            {
                itConfidenceMapCompare = _confidenceMap.erase(itConfidenceMapCompare);
            }
            else
            {
                ++itConfidenceMapCompare;
            }
        }
        itConfidenceMap++;
    }

    std::shared_ptr<YoloClassificationResult> yoloClassificationResult = std::make_shared<YoloClassificationResult>(spCoreDLAOutput->network_handle, spCoreDLAOutput->network_type, spCoreDLAOutput->inference_count, spCoreDLAOutput->inference_buffer);
    for (const auto& itConfidenceMap : _confidenceMap)
    {
        if(itConfidenceMap.first > 0.00001F) {
            YoloClassificationItem item(_objects[itConfidenceMap.second]);

            yoloClassificationResult->_items.emplace_back(item);
        }
    }

    return yoloClassificationResult;
}


float YoloPoseProcessor::IntersectionOverUnion(const YoloDetectionObject &box_1, const YoloDetectionObject &box_2) {
    int width_of_overlap_area = std::min(box_1.xmax, box_2.xmax) - std::max(box_1.xmin, box_2.xmin);
    int height_of_overlap_area = std::min(box_1.ymax, box_2.ymax) - std::max(box_1.ymin, box_2.ymin);
    int area_of_overlap;
    if (width_of_overlap_area < 0 || height_of_overlap_area < 0)
        area_of_overlap = 0;
    else
        area_of_overlap = width_of_overlap_area * height_of_overlap_area;
    int box_1_area = (box_1.ymax - box_1.ymin)  * (box_1.xmax - box_1.xmin);
    int box_2_area = (box_2.ymax - box_2.ymin)  * (box_2.xmax - box_2.xmin);
    int area_of_union = box_1_area + box_2_area - area_of_overlap;
    return (float)area_of_overlap / (float)area_of_union;
}


void YoloPoseProcessor::ParseYOLOOutput(const std::vector<ov::Tensor>& output_tensors)
{
    int prediction = 0;
    
    for(size_t ot = 0; ot < output_tensors.size()/3; ++ot)
    {
        const ov::Tensor& output_tensor_boxes = output_tensors.at(ot);
        const ov::Tensor& output_tensor_predictions = output_tensors.at(ot + output_tensors.size()/3);
        const ov::Tensor& output_tensor_keypoints = output_tensors.at(ot + 2 * output_tensors.size()/3);
        if(_bypass_output_layout_transform)
        {
            ParseYOLOOutputTensorCvec(output_tensor_boxes,output_tensor_predictions, output_tensor_keypoints, prediction);
        }
        else
        {
            ParseYOLOOutputTensor(output_tensor_boxes, output_tensor_predictions, output_tensor_keypoints, prediction);
        }
        const int out_blob_predictions = static_cast<int>(output_tensor_predictions.get_shape()[2]) * static_cast<int>(output_tensor_predictions.get_shape()[3]);
        prediction += out_blob_predictions;
    }
}

void YoloPoseProcessor::ParseYOLOOutputTensor(const ov::Tensor& output_tensor_boxes, const ov::Tensor& output_tensor_predictions, const ov::Tensor& output_tensor_keypoints, int prediction)
{
    float scale = (float)_params._original_im_w / (float)_params._input_tensor_w;
    float yolo_h = _params._input_tensor_h * scale;
    float offset_y = (yolo_h - _params._original_im_h)/(2*scale);

    const int out_blob_predictions = static_cast<int>(output_tensor_predictions.get_shape()[2]) * static_cast<int>(output_tensor_predictions.get_shape()[3]);
    const size_t c_stride = 16 * out_blob_predictions;

    const float* p_boxes =  (const float *)output_tensor_boxes.data<float>();
    const float* p_prediction =  (const float *)output_tensor_predictions.data<float>();
    const float *p_keypoints =  (const float *)output_tensor_keypoints.data<float>();

    const float *output_box_blob_i = p_boxes;
    const float *output_box_blob_prob = p_prediction;
    for (int i = 0; i < out_blob_predictions; ++i, ++prediction) {       
        float prob = *output_box_blob_prob;
        // sigmoid
        prob = 1.0f/(1.0f+expf(-prob));
        if (prob > _detection_threshold && !std::isnan(prob) && (prob < 0.999f))
        {
            float coord[4];
            const float *output_box_blob_a = output_box_blob_i;
            // for each coordinate
            for(int a = 0; a < 4; a++)
            {
                float softmax_sum = 0.0f;
                float coord_sum = 0.0f;
                const float *output_box_blob_b = output_box_blob_a;
                float exp_values[16];

                for(int b = 0; b < 16; b++)
                {
                    float e = expf(*output_box_blob_b);
                    softmax_sum += e;
                    exp_values[b] = e;
                    output_box_blob_b += out_blob_predictions;
                }
                
                for(int b = 0; b < 16; b++)
                {
                    // Softmax 
                    float softmax = exp_values[b] / softmax_sum;
                    // scale and sum
                    coord_sum += softmax * static_cast<float>(b);
                }
                coord[a] = coord_sum;
                output_box_blob_a = output_box_blob_b;
            }
            // anchor adjust
            float tlx = 0.5f + _params._anchor_x[prediction] - coord[0];
            float tly = 0.5f + _params._anchor_y[prediction] - coord[1];
            float brx = 0.5f + _params._anchor_x[prediction] + coord[2];
            float bry = 0.5f + _params._anchor_y[prediction] + coord[3];
            float x = _params._anchor_mult[prediction]*((tlx + brx) / 2.0f);
            float y = _params._anchor_mult[prediction]*((tly + bry) / 2.0f);
            float width = _params._anchor_mult[prediction]*(brx - tlx);
            float height = _params._anchor_mult[prediction]*(bry - tly);
            
            // for each keypoint
            std::vector<struct YoloPoseKeypoint> keypoints;
            for(int a = 0; a < 17; a++)
            {
                struct YoloPoseKeypoint keypoint;
                int index = a * 3 * out_blob_predictions + i;
                keypoint.keypoint_id = a;
                keypoint._x = p_keypoints[index];
                keypoint._y = p_keypoints[index + out_blob_predictions];
                keypoint._v = p_keypoints[index + 2*out_blob_predictions];
                keypoint._x *= 2;
                keypoint._y *= 2;
                keypoint._x += _params._anchor_x[prediction];
                keypoint._y += _params._anchor_y[prediction];
                keypoint._x *= _params._anchor_mult[prediction];
                keypoint._y *= _params._anchor_mult[prediction];
                // sigmoid
                keypoint._v = 1.0f/(1.0f+expf(-keypoint._v));

                keypoints.emplace_back(keypoint);
            }

            _objects[prediction].Set(x, y, height, width, 0, prob, &keypoints, static_cast<int>(offset_y), scale, scale);
            _confidenceMap.emplace(prob, prediction);
        }
        output_box_blob_i++;
        output_box_blob_prob++;
    }
}

void YoloPoseProcessor::ParseYOLOOutputTensorCvec(const ov::Tensor& output_tensor_boxes, const ov::Tensor& output_tensor_predictions, const ov::Tensor& output_tensor_keypoints, int prediction)
{
    float scale = (float)_params._original_im_w / (float)_params._input_tensor_w;
    float yolo_h = _params._input_tensor_h * scale;
    float offset_y = (yolo_h - _params._original_im_h)/(2*scale);

    const int out_blob_predictions = static_cast<int>(output_tensor_predictions.get_shape()[2]) * static_cast<int>(output_tensor_predictions.get_shape()[3]);
    const size_t c_stride = 16 * out_blob_predictions;

    const __fp16* p_boxes =  (const __fp16 *)output_tensor_boxes.data<ov::float16>();
    const __fp16* p_prediction =  (const __fp16 *)output_tensor_predictions.data<ov::float16>();
    const __fp16 *p_keypoints =  (const __fp16 *)output_tensor_keypoints.data<ov::float16>();

    const __fp16 *output_box_blob_i = p_boxes;
    const __fp16 *output_box_blob_prob = p_prediction;
    for (int i = 0; i < out_blob_predictions; ++i, ++prediction) {       
        float prob = static_cast<float>(*output_box_blob_prob);
        // sigmoid
        prob = 1.0f/(1.0f+expf(-prob));
        if (prob > _detection_threshold && !std::isnan(prob) && (prob < 0.999f))
        {
            float coord[4];
            const __fp16 *output_box_blob_a = output_box_blob_i;
            // for each coordinate
            for(int a = 0; a < 4; a++)
            {
                float softmax_sum = 0.0f;
                float coord_sum = 0.0f;
                const __fp16 *output_box_blob_b = output_box_blob_a;
                for(int b = 0; b < 16; b++)
                {
                    float e = expf(*output_box_blob_b);
                    softmax_sum += e;
                    output_box_blob_b ++;
                }
                output_box_blob_b = output_box_blob_a;
                for(int b = 0; b < 16; b++)
                {
                    float e = expf(*output_box_blob_b);
                    // Softmax 
                    float softmax = e/softmax_sum;
                    // scale and sum
                    coord_sum += softmax * static_cast<float>(b);
                    output_box_blob_b ++;
                }
                coord[a] = coord_sum;
                output_box_blob_a += c_stride;
            }
            // anchor adjust
            float tlx = 0.5f + _params._anchor_x[prediction] - coord[0];
            float tly = 0.5f + _params._anchor_y[prediction] - coord[1];
            float brx = 0.5f + _params._anchor_x[prediction] + coord[2];
            float bry = 0.5f + _params._anchor_y[prediction] + coord[3];
            float x = _params._anchor_mult[prediction]*((tlx + brx) / 2.0f);
            float y = _params._anchor_mult[prediction]*((tly + bry) / 2.0f);
            float width = _params._anchor_mult[prediction]*(brx - tlx);
            float height = _params._anchor_mult[prediction]*(bry - tly);
           
            // for each keypoint
            std::vector<struct YoloPoseKeypoint> keypoints;
            for(int a = 0; a < 17; a++)
            {
                struct YoloPoseKeypoint keypoint;
                auto cvec_index = [](int a, int i, int o, int c_stride){ 
                    int index = a * 3 + o;
                    int index_c = index / 16;
                    int index_cvec = index % 16;
                    index = index_c * c_stride + i*16 + index_cvec;
                    return index;
                };
                keypoint.keypoint_id = a;
                keypoint._x = p_keypoints[cvec_index(a, i, 0, c_stride)];
                keypoint._y = p_keypoints[cvec_index(a, i, 1, c_stride)];
                keypoint._v = p_keypoints[cvec_index(a, i, 2, c_stride)];
                keypoint._x *= 2;
                keypoint._y *= 2;
                keypoint._x += _params._anchor_x[prediction];
                keypoint._y += _params._anchor_y[prediction];
                keypoint._x *= _params._anchor_mult[prediction];
                keypoint._y *= _params._anchor_mult[prediction];
                // sigmoid
                keypoint._v = 1.0f/(1.0f+expf(-keypoint._v));

                keypoints.emplace_back(keypoint);
            }

            _objects[prediction].Set(x, y, height, width, 0, prob, &keypoints, static_cast<int>(offset_y), scale, scale);
            _confidenceMap.emplace(prob, prediction);
        }
        output_box_blob_i+=16;
        output_box_blob_prob+=16;
    }
}

