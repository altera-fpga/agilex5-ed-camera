/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef SRC_YOLOCLASSIFICATIONITEM_H_
#define SRC_YOLOCLASSIFICATIONITEM_H_

#include <cstdint>
#include <memory>
#include "YoloDetectionObject.h"
#include "YoloNames.h"

class YoloClassificationItem
{
public:
    YoloClassificationItem(YoloDetectionObject& object)
        : _category_index(object.class_id)
        , _score(object.confidence)
        , _x_min(object.xmin)
        , _y_min(object.ymin)
        , _x_max(object.xmax)
        , _y_max(object.ymax)
        , _category_name(GetCategoryString(object.class_id))
    {
        for(YoloPoseKeypoint keypoint: object.keypoints)
        {
            _keypoints.emplace_back(keypoint);
        }
        GetCategoryColor(object.class_id, _colour);
    }

    static void InitialiseCategoryNames(const char* const categoryNamesFile);
    static void GetCategoryColor(uint32_t class_index, uint8_t colour[4]);
    static const char* GetCategoryString(uint32_t category_index);

    // The category index
    int        _category_index;

    // The score, from 0.0 to 100.0
    float    _score;

    // true if result contains valid region information
    float    _x_min;
    float    _y_min;
    float    _x_max;
    float    _y_max;
    uint8_t _colour[4]; // RGBA colour hint for box drawing
    std::vector<YoloPoseKeypoint> _keypoints;

    // The name of the classification, e.g. 'Persian cat',
    // from a table look-up of _category_index
    std::string    _category_name;

    static std::shared_ptr<YoloNames> _category_names;
    static float _colors[6][3];
public:
    bool operator<(YoloClassificationItem& other) { return _score > other._score; }
};

#endif /* SRC_YOLOCLASSIFICATIONITEM_H_ */
