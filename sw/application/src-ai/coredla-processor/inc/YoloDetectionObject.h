/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef INC_YOLODETECTIONOBJECT_H_
#define INC_YOLODETECTIONOBJECT_H_

#include <cmath>
#include <vector>

struct YoloPoseKeypoint
{
    uint32_t keypoint_id;
    float _x;
    float _y;
    float _v; // visibility / confidence
};

struct YoloDetectionObject {
    int xmin, ymin, xmax, ymax, class_id;
    float confidence;
    std::vector<struct YoloPoseKeypoint> keypoints;

    YoloDetectionObject() {
        Set(0, 0, 0, 0, 0, 0.0f, nullptr, 0, 0.0f, 0.0f);
    }
    
    YoloDetectionObject(float x, float y, float h, float w, int class_id, float confidence, std::vector<struct YoloPoseKeypoint>* keypoints, int offset_y, float h_scale, float w_scale) {
        Set(x, y, h, w, class_id, confidence, keypoints, offset_y, h_scale, w_scale);
    }

    void Set(float x, float y, float h, float w, int class_id, float confidence, std::vector<struct YoloPoseKeypoint>* keypoints, int offset_y, float h_scale, float w_scale) {
        this->xmin = std::max(static_cast<int>((x - w / 2) * w_scale), 0);
        this->ymin = std::max(static_cast<int>((y - h / 2) * h_scale), 0);
        this->xmax = static_cast<int>((x + w / 2) * w_scale);
        this->ymax = static_cast<int>((y + h / 2) * h_scale);

        this->ymin -= offset_y * h_scale;
        this->ymax -= offset_y * h_scale;

        this->class_id = class_id;
        this->confidence = confidence;

        this->keypoints.clear();
        if(keypoints != nullptr)
        {
            for(const auto& k : *keypoints)
            {
                YoloPoseKeypoint keypoint;   
                keypoint.keypoint_id = k.keypoint_id;
                keypoint._x = static_cast<int>(k._x * w_scale);
                keypoint._y = static_cast<int>(k._y * h_scale);
                keypoint._v = k._v;

                keypoint._y -= offset_y * h_scale;

                this->keypoints.emplace_back(keypoint);
            }
        }
    }

    bool operator <(const YoloDetectionObject &s2) const {
        return this->confidence < s2.confidence;
    }
    bool operator >(const YoloDetectionObject &s2) const {
        return this->confidence > s2.confidence;
    }
};

#endif /* INC_YOLODETECTIONOBJECT_H_ */
