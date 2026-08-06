/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include <lvgl.h>
#include "lv_labelled_box.h"
#include "lv_labelled_box_private.h"
#include "FrameQueueConfig.h"
#include <src/core/lv_obj_class_private.h>

const lv_obj_class_t labelled_box_class = {
    .base_class = &lv_obj_class,
    .name = "labelled_box",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(lv_labelled_box_t),
};

lv_obj_t * labelled_box_create(lv_obj_t * parent, const YoloClassificationItem& item)
{
    lv_obj_t * obj = lv_obj_class_create_obj(&labelled_box_class, parent);
    lv_obj_class_init_obj(obj);

    lv_labelled_box_t * labelled_box = (lv_labelled_box_t *)obj;
    lv_style_init(&labelled_box->style);

    lv_color_t color;
    color.red = item._colour[2];
    color.green = item._colour[1];
    color.blue = item._colour[0];

    lv_style_set_line_color(&labelled_box->style, color);
    lv_style_set_line_width(&labelled_box->style, 2);
    lv_style_set_line_rounded(&labelled_box->style, false);

    uint32_t xmin = (item._x_min >= 0) ? item._x_min : 0;
    uint32_t ymin = (item._y_min >= 0) ? item._y_min : 0;
    uint32_t xmax = (item._x_max < SwApi::FQ::OVERLAY_WIDTH) ? item._x_max : SwApi::FQ::OVERLAY_WIDTH-1;
    uint32_t ymax = (item._y_max < SwApi::FQ::OVERLAY_HEIGHT) ? item._y_max : SwApi::FQ::OVERLAY_HEIGHT-1;

    labelled_box->box = lv_line_create(obj);
    labelled_box->cords[0].x = xmin;
    labelled_box->cords[0].y = ymin;
    labelled_box->cords[1].x = xmax;
    labelled_box->cords[1].y = ymin;
    labelled_box->cords[2].x = xmax;
    labelled_box->cords[2].y = ymax;
    labelled_box->cords[3].x = xmin;
    labelled_box->cords[3].y = ymax;
    labelled_box->cords[4].x = xmin;
    labelled_box->cords[4].y = ymin;
    lv_line_set_points(labelled_box->box, labelled_box->cords, 5);
    lv_obj_add_style(labelled_box->box, &labelled_box->style, 0);

    snprintf(labelled_box->sBoxText, sizeof(labelled_box->sBoxText), "%s: %.1f%%", item._category_name.c_str(), 100*item._score);
    labelled_box->text = lv_label_create(obj);
    lv_label_set_text(labelled_box->text, labelled_box->sBoxText);
    lv_obj_set_style_text_color(labelled_box->text, color, 0);
    uint32_t line_height = (uint32_t)lv_font_get_line_height(lv_font_default());
    lv_obj_set_pos(labelled_box->text, xmin, (ymin >= line_height) ? ymin - line_height : 0);

    return obj;
}
