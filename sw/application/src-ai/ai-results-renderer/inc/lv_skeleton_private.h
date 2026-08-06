/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#ifndef LV_SKELETON_PRIVATE_H
#define LV_SKELETON_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_skeleton.h"
#include <src/core/lv_obj_private.h>

enum class COCOBody
{
    nose,
    left_eye,
    right_eye,
    left_ear,
    right_ear,
    left_shoulder,
    right_shoulder,
    left_elbow,
    right_elbow,
    left_wrist,
    right_wrist,
    left_hip,
    right_hip,
    left_knee,
    right_knee,
    left_ankle,
    right_ankle
};

struct _lv_skeleton_t {
    lv_obj_t obj;
    lv_obj_t * box;
    lv_point_precise_t boxcords[5];
    lv_obj_t * left_ear_eye_obj;
    lv_point_precise_t left_ear_eye_pts[2];
    lv_obj_t * left_eye_nose_obj;
    lv_point_precise_t left_eye_nose_pts[2];
    lv_obj_t * right_ear_eye_obj;
    lv_point_precise_t right_ear_eye_pts[2];
    lv_obj_t * right_eye_nose_obj;
    lv_point_precise_t right_eye_nose_pts[2];
    lv_obj_t * nose_left_shoulder_obj;
    lv_point_precise_t nose_left_shoulder_pts[2];
    lv_obj_t * nose_right_shoulder_obj;
    lv_point_precise_t nose_right_shoulder_pts[2];
    lv_obj_t * left_shoulder_elbow_obj;
    lv_point_precise_t left_shoulder_elbow_pts[2];
    lv_obj_t * right_shoulder_elbow_obj;
    lv_point_precise_t right_shoulder_elbow_pts[2];
    lv_obj_t * left_elbow_wrist_obj;
    lv_point_precise_t left_elbow_wrist_pts[2];
    lv_obj_t * right_elbow_wrist_obj;
    lv_point_precise_t right_elbow_wrist_pts[2];
    lv_obj_t * left_shoulder_hip_obj;
    lv_point_precise_t left_shoulder_hip_pts[2];
    lv_obj_t * right_shoulder_hip_obj;
    lv_point_precise_t right_shoulder_hip_pts[2];
    lv_obj_t * left_hip_right_hip_obj;
    lv_point_precise_t left_hip_right_hip_pts[2];
    lv_obj_t * left_hip_knee_obj;
    lv_point_precise_t left_hip_knee_pts[2];
    lv_obj_t * right_hip_knee_obj;
    lv_point_precise_t right_hip_knee_pts[2];
    lv_obj_t * left_knee_ankle_obj;
    lv_point_precise_t left_knee_ankle_pts[2];
    lv_obj_t * right_knee_ankle_obj;
    lv_point_precise_t right_knee_ankle_pts[2];
};
typedef struct _lv_skeleton_t lv_skeleton_t;


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_SKELETON_PRIVATE_H*/