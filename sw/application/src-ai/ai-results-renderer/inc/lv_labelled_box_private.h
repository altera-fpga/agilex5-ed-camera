/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#ifndef LV_LABELLED_BOX_PRIVATE_H
#define LV_LABELLED_BOX_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_labelled_box.h"
#include <src/core/lv_obj_private.h>

struct _lv_labelled_box_t {
    lv_obj_t obj;
    lv_style_t style;
    lv_obj_t * box;
    lv_obj_t * text;
    lv_point_precise_t cords[5];
    char sBoxText[32];
};
typedef struct _lv_labelled_box_t lv_labelled_box_t;


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_LABELLED_BOX_PRIVATE_H*/