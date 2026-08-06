/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __IRESULTSPROCESS_H__
#define __IRESULTSPROCESS_H__

#include "InferenceTypes.h"
#include "YoloClassificationResult.h"
#include "IResultsCallback.h"

class IResultProcessor
{
public:
    virtual std::shared_ptr<YoloClassificationResult> ProcessResults(const std::shared_ptr<tCoreDLAOutput>& spCoreDLAOutput) = 0;

    virtual void SetDetectionThreshold(float threshold) { static_cast<void>(threshold); }
    virtual void SetIOUThreshold(float threshold) { static_cast<void>(threshold); }
};

#endif //__IRESULTSPROCESS_H__
