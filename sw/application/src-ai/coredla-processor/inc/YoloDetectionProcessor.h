/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __YOLODETECTIONPROCESSOR_H__
#define __YOLODETECTIONPROCESSOR_H__

#include "InferenceTypes.h"
#include "IResultProcessor.h"

class YoloDetectionProcessor : public IResultProcessor
{
public:
    YoloDetectionProcessor(const ModelPtr& model, bool bypass_output_layout_transform);
    virtual ~YoloDetectionProcessor();

    // IResultProcessor
    std::shared_ptr<YoloClassificationResult> ProcessResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput) override;
    void SetDetectionThreshold(float threshold) override;
    void SetIOUThreshold(float threshold) override;
private:
    std::shared_ptr<YoloClassificationResult> GetClassificationResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput);
    static float IntersectionOverUnion(const YoloDetectionObject &box_1, const YoloDetectionObject &box_2);
    void ParseYOLOOutput(const std::vector<ov::Tensor>& output_tensors);
    void ParseYOLOOutputTensor(const ov::Tensor& output_tensor_boxes, const ov::Tensor& output_tensor_predictions, int prediction);
    void ParseYOLOOutputTensorCvec(const ov::Tensor& output_tensor_boxes, const ov::Tensor& output_tensor_predictions, int prediction);

private:
    bool _bypass_output_layout_transform;
    YoloParams _params;
    float _detection_threshold;
    float _iou_threshold;
    uint32_t _input_tensor_h;
    uint32_t _input_tensor_w;
    std::vector<YoloDetectionObject> _objects;
    std::multimap<float, size_t, std::greater<float>> _confidenceMap;
};

#endif // __YOLODETECTIONPROCESSOR_H__
