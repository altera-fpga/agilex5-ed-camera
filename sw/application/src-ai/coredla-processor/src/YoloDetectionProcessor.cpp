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
#include "YoloDetectionProcessor.h"
#include "OpenVinoIE.h"
#include "FrameQueueConfig.h"
#include <math.h>
#include <algorithm>
#include <cmath>
#include <unistd.h>
#include <sys/fcntl.h>

#ifdef __ARM_NEON
#ifdef __ARM_FEATURE_FP16_VECTOR_ARITHMETIC
#include <arm_neon.h>
#include <arm_fp16.h>
#define USE_SIMD
#endif /* __ARM_FEATURE_FP16_VECTOR_ARITHMETIC */
#endif /* __ARM_NEON__ */

extern std::string _bin_directory;

YoloDetectionProcessor::YoloDetectionProcessor(const ModelPtr& network_model, bool bypass_output_layout_transform)
    : _bypass_output_layout_transform(bypass_output_layout_transform)
    , _detection_threshold(0.25f)
    , _iou_threshold(0.85f)
    , _input_tensor_h(0U)
    , _input_tensor_w(0U)
{
    _params._original_im_w = SwApi::FQ::OVERLAY_WIDTH;
    _params._original_im_h = SwApi::FQ::OVERLAY_HEIGHT;
    _params._coords = 4;
    _params._classes = 80;
    if(network_model)
    {
        ov::Output<const ov::Node> input_port = network_model->_ovCompiledModel.input();
        
        _input_tensor_h = input_port.get_shape()[2];
        _input_tensor_w = input_port.get_shape()[3];

        _params.init(network_model);
    }
    _objects.resize(_params._output_tensor_predictions);
}

YoloDetectionProcessor::~YoloDetectionProcessor()
{
}


// Called on CoreProcessor thread
std::shared_ptr<YoloClassificationResult> YoloDetectionProcessor::ProcessResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput)
{
    std::shared_ptr<YoloClassificationResult> classification_result = GetClassificationResults(spCoreDLAOutput);
#if 0
    const std::vector<YoloClassificationItem>& items = classification_result->GetItems();
    std::cout << "Num Results " << items.size() << std::endl;
    for (const auto& item : items)
    {
        std::cout << item._category_name.c_str() << ":" << item._score << " x_min:" << item._x_min << " y_min:" << item._y_min << " x_max:" << item._x_max << " y_max:" << item._y_max << std::endl;
    }
#endif
    return classification_result;
}

///////////////////////////////////////////////////////////////////////////////


void YoloDetectionProcessor::SetDetectionThreshold(float threshold)
{
    _detection_threshold = threshold;
}

void YoloDetectionProcessor::SetIOUThreshold(float threshold)
{
    _iou_threshold = threshold;
}

std::shared_ptr<YoloClassificationResult> YoloDetectionProcessor::GetClassificationResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput)
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


float YoloDetectionProcessor::IntersectionOverUnion(const YoloDetectionObject &box_1, const YoloDetectionObject &box_2) {
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


void YoloDetectionProcessor::ParseYOLOOutput(const std::vector<ov::Tensor>& output_tensors)
{
    int prediction = 0;
    
    for(size_t ot = 0; ot < output_tensors.size(); ++ot)
    {
        const ov::Tensor& output_tensor = output_tensors.at(ot);
        if(_bypass_output_layout_transform)
        {
            ParseYOLOOutputTensorCvec(output_tensor, prediction);
        }
        else
        {
            ParseYOLOOutputTensor(output_tensor, prediction);
        }
        const int out_blob_predictions = static_cast<int>(output_tensor.get_shape()[2]) * static_cast<int>(output_tensor.get_shape()[3]);
        prediction += out_blob_predictions;
    }
}

void YoloDetectionProcessor::ParseYOLOOutputTensor(const ov::Tensor& output_tensor, int prediction)
{
    float scale = (float)_params._original_im_w / (float)_params._input_tensor_w;
    float yolo_h = _params._input_tensor_h * scale;
    float offset_y = (yolo_h - _params._original_im_h) / (2.0f * scale);
    float inv_detection_threshold = - logf((1.0f - _detection_threshold) / _detection_threshold);

    const int out_blob_predictions = static_cast<int>(output_tensor.get_shape()[2]) * static_cast<int>(output_tensor.get_shape()[3]);
    const float *output_blob =  output_tensor.data<const float>();
    const float *output_blob_i = output_blob;

    //// For each prediction find the class with the highest probability
    const float* pc0 = output_blob_i + out_blob_predictions * 64;
    const float* pc1 = pc0 + out_blob_predictions;

    std::vector<int32_t> max_prob_class(out_blob_predictions, 0);
    std::vector<float> max_prob_value(pc0, pc0 + out_blob_predictions);

    float* maxval = max_prob_value.data();
    int32_t* maxidx = (int32_t*)max_prob_class.data();

    // For each prediction find the class with the highest probability
    for (int r = 1; r < _params._classes; ++r)
    {
        int c = 0;

#ifdef USE_SIMD
        const int simd_end = (out_blob_predictions / 4) * 4;
        
        for (; c < simd_end; c += 4)
        {
            // Load existing max values and current row
            float32x4_t v_old = vld1q_f32(maxval + c);
            float32x4_t v_new = vld1q_f32(pc1 + c);

            // Compare
            uint32x4_t mask = vcgtq_f32(v_new, v_old);

            // Blend updated max values
            float32x4_t v_updated = vbslq_f32(mask, v_new, v_old);

            // Check if values changed
            uint32x4_t changed = vmvnq_u32(vceqq_f32(v_updated, v_old));

            // If any lane changed, store values and indices
            if(vmaxvq_u32(changed))  // ARMv8 reduction OR
            {
                vst1q_f32(maxval + c, v_updated);

                int32x4_t idx_old = vld1q_s32(maxidx + c);
                int32x4_t idx_new = vdupq_n_s32((int)r);
                int32x4_t idx_out = vbslq_s32(mask, idx_new, idx_old);
                vst1q_s32(maxidx + c, idx_out);
            }
        }
#endif /* USE_SIMD */

        // Remaining data from the vectorized loop (non-multiple of 4)
        // Or the scalar path if SIMD is not used
        for (; c < out_blob_predictions; c++)
        {
            const float val = pc1[c];

            if (val > maxval[c])
            {
                maxval[c] = val;
                maxidx[c] = r;
            }
        }

        pc1 += out_blob_predictions;
    }
    
    for (int i = 0; i < out_blob_predictions; ++i, ++prediction) {

        const float max_prob_index = max_prob_class[i];
        float max_prob = max_prob_value[i];

        // Process if the class probability is higher than threshold
        if(max_prob > inv_detection_threshold)
        {
            // sigmoid
            max_prob = 1.0f / (1.0f + expf(-max_prob));

            float coord[4];
            const float *output_blob_a = output_blob_i;           
            
            // for each coordinate
            for(int a = 0; a < 4; a++)
            {
                float softmax_sum = 0.0f;
                float coord_sum = 0.0f;
                const float *output_blob_b = output_blob_a;

                float exp_values[16];

                for(int b = 0; b < 16; b++)
                {
                    float e = expf(*output_blob_b);
                    softmax_sum += e;
                    exp_values[b] = e;
                    output_blob_b += out_blob_predictions;
                }
                
                for(int b = 0; b < 16; b++)
                {
                    // Softmax 
                    float softmax = exp_values[b] / softmax_sum;
                    // scale and sum
                    coord_sum += softmax * static_cast<float>(b);
                }

                coord[a] = coord_sum;
                output_blob_a = output_blob_b;
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

            _objects[prediction].Set(x, y, height, width, max_prob_index, max_prob, nullptr, static_cast<int>(offset_y), scale, scale);
            _confidenceMap.emplace(max_prob, prediction);
        }

        output_blob_i++;
    }
}


void YoloDetectionProcessor::ParseYOLOOutputTensorCvec(const ov::Tensor& output_tensor, int prediction)
{
    float scale = (float)_params._original_im_w / (float)_params._input_tensor_w;
    float yolo_h = _params._input_tensor_h * scale;
    float offset_y = (yolo_h - _params._original_im_h)/(2.0f * scale);
    float inv_detection_threshold = - logf((1.0f - _detection_threshold)/_detection_threshold);

    const int out_blob_predictions = static_cast<int>(output_tensor.get_shape()[2]) * static_cast<int>(output_tensor.get_shape()[3]);
    const size_t c_stride = 16 * out_blob_predictions;

    const __fp16* p_prediction =  (const __fp16 *)output_tensor.data<ov::float16>();
    const size_t prob_class_offset = out_blob_predictions * 64;

    const size_t total_classes = _params._classes;

    for (int i = 0; i < out_blob_predictions; ++i, ++prediction)
    {      
        float max_prob = std::numeric_limits<float>::lowest();
        int max_prob_index = 0;
        size_t class_index = 0;

        const __fp16* p = p_prediction + prob_class_offset;

#ifdef USE_SIMD
        // Max probability values
        float16x8_t vmp0 = vld1q_f16((const float16_t*)p);
        float16x8_t vmp1 = vld1q_f16((const float16_t*)(p + 8));

        // Max probability indices
        const uint16x8_t increment = {0, 1, 2, 3, 4, 5, 6, 7};
        uint16x8_t vi0 = vaddq_u16(vdupq_n_u16(class_index), increment);
        uint16x8_t vi1 = vaddq_u16(vdupq_n_u16(class_index + 8), increment);

        p += c_stride;
        class_index += 16;

        for(size_t c = 1; c < (total_classes / 16); ++c)
        {
            // Load current probability values
            const float16x8_t v2 = vld1q_f16((const float16_t*)p);
            const float16x8_t v3 = vld1q_f16((const float16_t*)(p + 8));

            // Generate current indices
            const uint16x8_t vi2 = vaddq_u16(vdupq_n_u16(class_index), increment);
            const uint16x8_t vi3 = vaddq_u16(vdupq_n_u16(class_index + 8), increment);                                     

            // Compare and store max values and indices
            const uint16x8_t mask0 = vcgtq_f16(v2, vmp0);
            const uint16x8_t mask1 = vcgtq_f16(v3, vmp1);

            vmp0 = vbslq_f16(mask0, v2, vmp0);
            vmp1 = vbslq_f16(mask1, v3, vmp1);
            
            vi0 = vbslq_u16(mask0, vi2, vi0);
            vi1 = vbslq_u16(mask1, vi3, vi1);

            p += c_stride;
            class_index += 16;
        }

        uint16x8_t mask = vcgtq_f16(vmp0, vmp1);
        vmp0 = vbslq_f16(mask, vmp0, vmp1);
        vi0 = vbslq_u16(mask, vi0, vi1);

        // Horizontal reduction to find overall max using pairwise operations
        float16x8_t vshift = vextq_f16(vmp0, vmp0, 4);
        uint16x8_t ishift = vextq_u16(vi0, vi0, 4);
        mask = vcgtq_f16(vshift, vmp0);
        vmp0 = vbslq_f16(mask, vshift, vmp0);
        vi0 = vbslq_u16(mask, ishift, vi0);

        vshift = vextq_f16(vmp0, vmp0, 2);
        ishift = vextq_u16(vi0, vi0, 2);
        mask = vcgtq_f16(vshift, vmp0);
        vmp0 = vbslq_f16(mask, vshift, vmp0);
        vi0 = vbslq_u16(mask, ishift, vi0);

        vshift = vextq_f16(vmp0, vmp0, 1);
        ishift = vextq_u16(vi0, vi0, 1);
        mask = vcgtq_f16(vshift, vmp0);
        vmp0 = vbslq_f16(mask, vshift, vmp0);
        vi0 = vbslq_u16(mask, ishift, vi0);                

        // Extract max probability and index
        max_prob = vgetq_lane_f16(vmp0, 0);
        max_prob_index = vgetq_lane_u16(vi0, 0);
#endif /* USE_SIMD */
        
        for(; class_index < total_classes; ++class_index)
        {
            const size_t c_vec = class_index % 16;
            const float prob = p[c_vec];

            if(prob > max_prob)
            {
                max_prob = prob;
                max_prob_index = class_index;
            }

            if(c_vec == 15)
                p += c_stride;
        }

        if(max_prob > inv_detection_threshold)
        {
            // sigmoid
            max_prob = 1.0f/(1.0f+expf(-max_prob));
            float coord[4];
            const __fp16* p_coord = p_prediction;

            // for each coordinate
            for(size_t a = 0; a < 4; a++)
            {
                float softmax_sum = 0.0f;
                float coord_sum = 0.0f;
                const __fp16* p_val = p_coord;

                float exp_values[16];

                for(size_t b = 0; b < 16; b++)
                {
                    const float e = expf(p_val[b]);
                    softmax_sum += e;
                    exp_values[b] = e;
                }
                
                for(size_t b = 0; b < 16; b++)
                {
                    // Softmax 
                    const float softmax = exp_values[b] / softmax_sum;
                    // scale and sum
                    coord_sum += softmax * static_cast<float>(b);
                }

                coord[a] = coord_sum;
                p_coord += c_stride;
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

            _objects[prediction].Set(x, y, height, width, max_prob_index, max_prob, nullptr, static_cast<int>(offset_y), scale, scale);
            _confidenceMap.emplace(max_prob, prediction);
        }
        
        p_prediction += 16;
    }
}
