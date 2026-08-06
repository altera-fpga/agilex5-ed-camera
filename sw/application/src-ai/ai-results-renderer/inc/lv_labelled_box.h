/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#ifndef LV_LABELLED_BOX_H
#define LV_LABELLED_BOX_H

#include "YoloClassificationItem.h"

#ifdef __cplusplus
extern "C" {
#endif

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t labelled_box_class;

lv_obj_t * labelled_box_create(lv_obj_t * parent, const YoloClassificationItem& item);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LABELLED_BOX_H*/