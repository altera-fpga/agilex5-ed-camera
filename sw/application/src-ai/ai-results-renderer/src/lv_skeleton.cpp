/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include <lvgl.h>
#include "lv_skeleton.h"
#include "lv_skeleton_private.h"
#include "FrameQueueConfig.h"
#include <src/core/lv_obj_class_private.h>

const lv_obj_class_t lv_skeleton_class = {
    .base_class = &lv_obj_class,
    .name = "skeleton",
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .instance_size = sizeof(lv_skeleton_t),
};

lv_obj_t * lv_skeleton_create(lv_obj_t * parent, const YoloClassificationItem& item, const float keypoint_threshold)
{
    lv_obj_t * obj = lv_obj_class_create_obj(&lv_skeleton_class, parent);
    lv_obj_class_init_obj(obj);

    lv_skeleton_t * skeleton = (lv_skeleton_t *)obj;
    static lv_style_t style;
    lv_style_init(&style);

    lv_style_set_line_color(&style, lv_palette_main(LV_PALETTE_RED));
    lv_style_set_line_width(&style, 3);
    lv_style_set_line_rounded(&style, true);

#ifdef DRAW_BOX
    uint32_t xmin = (item._x_min >= 0) ? item._x_min : 0;
    uint32_t ymin = (item._y_min >= 0) ? item._y_min : 0;
    uint32_t xmax = (item._x_max < SwApi::FQ::OVERLAY_WIDTH) ? item._x_max : SwApi::FQ::OVERLAY_WIDTH-1;
    uint32_t ymax = (item._y_max < SwApi::FQ::OVERLAY_HEIGHT) ? item._y_max : SwApi::FQ::OVERLAY_HEIGHT-1;

    skeleton->box = lv_line_create(obj);
    skeleton->boxcords[0].x = xmin;
    skeleton->boxcords[0].y = ymin;
    skeleton->boxcords[1].x = xmax;
    skeleton->boxcords[1].y = ymin;
    skeleton->boxcords[2].x = xmax;
    skeleton->boxcords[2].y = ymax;
    skeleton->boxcords[3].x = xmin;
    skeleton->boxcords[3].y = ymax;
    skeleton->boxcords[4].x = xmin;
    skeleton->boxcords[4].y = ymin;
    lv_line_set_points(skeleton->box, skeleton->boxcords, 5);
    lv_obj_add_style(skeleton->box, &style, 0);
#endif

    if((item._keypoints[static_cast<uint32_t>(COCOBody::left_ear)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::left_eye)]._v >= keypoint_threshold))
    {
        skeleton->left_ear_eye_obj = lv_line_create(obj);
        skeleton->left_ear_eye_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_ear)]._x;
        skeleton->left_ear_eye_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_ear)]._y;
        skeleton->left_ear_eye_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_eye)]._x;
        skeleton->left_ear_eye_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_eye)]._y;
        lv_line_set_points(skeleton->left_ear_eye_obj, skeleton->left_ear_eye_pts, 2);
        lv_obj_add_style(skeleton->left_ear_eye_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::left_eye)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._v >= keypoint_threshold))
    {
        skeleton->left_eye_nose_obj = lv_line_create(obj);
        skeleton->left_eye_nose_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_eye)]._x;
        skeleton->left_eye_nose_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_eye)]._y;
        skeleton->left_eye_nose_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._x;
        skeleton->left_eye_nose_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._y;
        lv_line_set_points(skeleton->left_eye_nose_obj, skeleton->left_eye_nose_pts, 2);
        lv_obj_add_style(skeleton->left_eye_nose_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::right_ear)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::right_eye)]._v >= keypoint_threshold))
    {
        skeleton->right_ear_eye_obj = lv_line_create(obj);
        skeleton->right_ear_eye_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_ear)]._x;
        skeleton->right_ear_eye_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_ear)]._y;
        skeleton->right_ear_eye_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_eye)]._x;
        skeleton->right_ear_eye_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_eye)]._y;
        lv_line_set_points(skeleton->right_ear_eye_obj, skeleton->right_ear_eye_pts, 2);
        lv_obj_add_style(skeleton->right_ear_eye_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::right_eye)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._v >= keypoint_threshold))
    {
        skeleton->right_eye_nose_obj = lv_line_create(obj);
        skeleton->right_eye_nose_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_eye)]._x;
        skeleton->right_eye_nose_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_eye)]._y;
        skeleton->right_eye_nose_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._x;
        skeleton->right_eye_nose_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._y;
        lv_line_set_points(skeleton->right_eye_nose_obj, skeleton->right_eye_nose_pts, 2);
        lv_obj_add_style(skeleton->right_eye_nose_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._v >= keypoint_threshold))
    {
        skeleton->nose_left_shoulder_obj = lv_line_create(obj);
        skeleton->nose_left_shoulder_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._x;
        skeleton->nose_left_shoulder_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._y;
        skeleton->nose_left_shoulder_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._x;
        skeleton->nose_left_shoulder_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._y;
        lv_line_set_points(skeleton->nose_left_shoulder_obj, skeleton->nose_left_shoulder_pts, 2);
        lv_obj_add_style(skeleton->nose_left_shoulder_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._v >= keypoint_threshold))
    {
        skeleton->nose_right_shoulder_obj = lv_line_create(obj);
        skeleton->nose_right_shoulder_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._x;
        skeleton->nose_right_shoulder_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::nose)]._y;
        skeleton->nose_right_shoulder_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._x;
        skeleton->nose_right_shoulder_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._y;
        lv_line_set_points(skeleton->nose_right_shoulder_obj, skeleton->nose_right_shoulder_pts, 2);
        lv_obj_add_style(skeleton->nose_right_shoulder_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::left_elbow)]._v >= keypoint_threshold))
    {
        skeleton->left_shoulder_elbow_obj = lv_line_create(obj);
        skeleton->left_shoulder_elbow_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._x;
        skeleton->left_shoulder_elbow_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._y;
        skeleton->left_shoulder_elbow_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_elbow)]._x;
        skeleton->left_shoulder_elbow_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_elbow)]._y;
        lv_line_set_points(skeleton->left_shoulder_elbow_obj, skeleton->left_shoulder_elbow_pts, 2);
        lv_obj_add_style(skeleton->left_shoulder_elbow_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::right_elbow)]._v >= keypoint_threshold))
    {
        skeleton->right_shoulder_elbow_obj = lv_line_create(obj);
        skeleton->right_shoulder_elbow_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._x;
        skeleton->right_shoulder_elbow_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._y;
        skeleton->right_shoulder_elbow_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_elbow)]._x;
        skeleton->right_shoulder_elbow_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_elbow)]._y;
        lv_line_set_points(skeleton->right_shoulder_elbow_obj, skeleton->right_shoulder_elbow_pts, 2);
        lv_obj_add_style(skeleton->right_shoulder_elbow_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::left_elbow)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::left_wrist)]._v >= keypoint_threshold))
    {
        skeleton->left_elbow_wrist_obj = lv_line_create(obj);
        skeleton->left_elbow_wrist_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_elbow)]._x;
        skeleton->left_elbow_wrist_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_elbow)]._y;
        skeleton->left_elbow_wrist_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_wrist)]._x;
        skeleton->left_elbow_wrist_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_wrist)]._y;
        lv_line_set_points(skeleton->left_elbow_wrist_obj, skeleton->left_elbow_wrist_pts, 2);
        lv_obj_add_style(skeleton->left_elbow_wrist_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::right_elbow)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::right_wrist)]._v >= keypoint_threshold))
    {
        skeleton->right_elbow_wrist_obj = lv_line_create(obj);
        skeleton->right_elbow_wrist_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_elbow)]._x;
        skeleton->right_elbow_wrist_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_elbow)]._y;
        skeleton->right_elbow_wrist_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_wrist)]._x;
        skeleton->right_elbow_wrist_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_wrist)]._y;
        lv_line_set_points(skeleton->right_elbow_wrist_obj, skeleton->right_elbow_wrist_pts, 2);
        lv_obj_add_style(skeleton->right_elbow_wrist_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._v >= keypoint_threshold))
    {
        skeleton->left_shoulder_hip_obj = lv_line_create(obj);
        skeleton->left_shoulder_hip_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._x;
        skeleton->left_shoulder_hip_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_shoulder)]._y;
        skeleton->left_shoulder_hip_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._x;
        skeleton->left_shoulder_hip_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._y;
        lv_line_set_points(skeleton->left_shoulder_hip_obj, skeleton->left_shoulder_hip_pts, 2);
        lv_obj_add_style(skeleton->left_shoulder_hip_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._v >= keypoint_threshold))
    {
        skeleton->right_shoulder_hip_obj = lv_line_create(obj);
        skeleton->right_shoulder_hip_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._x;
        skeleton->right_shoulder_hip_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_shoulder)]._y;
        skeleton->right_shoulder_hip_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._x;
        skeleton->right_shoulder_hip_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._y;
        lv_line_set_points(skeleton->right_shoulder_hip_obj, skeleton->right_shoulder_hip_pts, 2);
        lv_obj_add_style(skeleton->right_shoulder_hip_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._v >= keypoint_threshold))
    {
        skeleton->left_hip_right_hip_obj = lv_line_create(obj);
        skeleton->left_hip_right_hip_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._x;
        skeleton->left_hip_right_hip_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._y;
        skeleton->left_hip_right_hip_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._x;
        skeleton->left_hip_right_hip_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._y;
        lv_line_set_points(skeleton->left_hip_right_hip_obj, skeleton->left_hip_right_hip_pts, 2);
        lv_obj_add_style(skeleton->left_hip_right_hip_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::left_knee)]._v >= keypoint_threshold))
    {
        skeleton->left_hip_knee_obj = lv_line_create(obj);
        skeleton->left_hip_knee_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._x;
        skeleton->left_hip_knee_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_hip)]._y;
        skeleton->left_hip_knee_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_knee)]._x;
        skeleton->left_hip_knee_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_knee)]._y;
        lv_line_set_points(skeleton->left_hip_knee_obj, skeleton->left_hip_knee_pts, 2);
        lv_obj_add_style(skeleton->left_hip_knee_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::right_knee)]._v >= keypoint_threshold))
    {
        skeleton->right_hip_knee_obj = lv_line_create(obj);
        skeleton->right_hip_knee_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._x;
        skeleton->right_hip_knee_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_hip)]._y;
        skeleton->right_hip_knee_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_knee)]._x;
        skeleton->right_hip_knee_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_knee)]._y;
        lv_line_set_points(skeleton->right_hip_knee_obj, skeleton->right_hip_knee_pts, 2);
        lv_obj_add_style(skeleton->right_hip_knee_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::left_knee)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::left_ankle)]._v >= keypoint_threshold))
    {
        skeleton->left_knee_ankle_obj = lv_line_create(obj);
        skeleton->left_knee_ankle_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_knee)]._x;
        skeleton->left_knee_ankle_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_knee)]._y;
        skeleton->left_knee_ankle_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::left_ankle)]._x;
        skeleton->left_knee_ankle_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::left_ankle)]._y;
        lv_line_set_points(skeleton->left_knee_ankle_obj, skeleton->left_knee_ankle_pts, 2);
        lv_obj_add_style(skeleton->left_knee_ankle_obj, &style, 0);
    }

    if((item._keypoints[static_cast<uint32_t>(COCOBody::right_knee)]._v >= keypoint_threshold) &&
       (item._keypoints[static_cast<uint32_t>(COCOBody::right_ankle)]._v >= keypoint_threshold))
    {
        skeleton->right_knee_ankle_obj = lv_line_create(obj);
        skeleton->right_knee_ankle_pts[0].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_knee)]._x;
        skeleton->right_knee_ankle_pts[0].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_knee)]._y;
        skeleton->right_knee_ankle_pts[1].x = item._keypoints[static_cast<uint32_t>(COCOBody::right_ankle)]._x;
        skeleton->right_knee_ankle_pts[1].y = item._keypoints[static_cast<uint32_t>(COCOBody::right_ankle)]._y;
        lv_line_set_points(skeleton->right_knee_ankle_obj, skeleton->right_knee_ankle_pts, 2);
        lv_obj_add_style(skeleton->right_knee_ankle_obj, &style, 0);
    }

    return obj;
}
