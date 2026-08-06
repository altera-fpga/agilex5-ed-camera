/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include <math.h>
#include <cstdint>
#include "YoloClassificationItem.h"

float YoloClassificationItem::_colors[6][3] = {
        { 255.0f,   0.0f, 255.0f},
        {   0.0f,   0.0f, 255.0f},
        {   0.0f, 255.0f, 255.0f},
        {   0.0f, 255.0f,   0.0f},
        { 255.0f, 255.0f,   0.0f},
        { 255.0f,   0.0f,   0.0f}
};

std::shared_ptr<YoloNames> YoloClassificationItem::_category_names;

void YoloClassificationItem::InitialiseCategoryNames(const char* const categoryNamesFile)
{
    _category_names = std::make_shared<YoloNames>(categoryNamesFile);
}

void YoloClassificationItem::GetCategoryColor(uint32_t class_index, uint8_t colour[4]) {
    if (_category_names)
    {
        uint32_t num_classes = _category_names->size();

        uint32_t offset = class_index * 123457 % num_classes;

        float ratio = ((float)offset / (float)num_classes) * 5.0f; // 0 to 5
        int i = (int)floorf(ratio); // 0 to 4
        int j = (int)ceilf(ratio); // 1 to 5
        ratio -= (float)i; // 0 to 1

        // blend color i and j by ratio
        colour[0] = uint8_t((1 - ratio) * _colors[i][2]) + (uint8_t)(ratio * _colors[j][2]);
        colour[1] = uint8_t((1 - ratio) * _colors[i][1]) + (uint8_t)(ratio * _colors[j][1]);
        colour[2] = uint8_t((1 - ratio) * _colors[i][0]) + (uint8_t)(ratio * _colors[j][0]);
        colour[3] = 0xFF;
    }
}

const char* YoloClassificationItem::GetCategoryString(uint32_t category_index)
{
    const char* categoryName = "unknown";
    if (_category_names)
    {
        if (category_index < _category_names->size())
            categoryName = (*_category_names)[category_index].c_str();
    }

    return categoryName;
}

