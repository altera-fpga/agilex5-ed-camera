/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __IAiResultsRenderer_H__
#define __IAiResultsRenderer_H__

namespace SwApi 
{
    class IAiResultsRenderer
    {
        public:
            IAiResultsRenderer() {}
            virtual ~IAiResultsRenderer() {}

            virtual void SetKeypointThreshold(float keypointThreshold) = 0;
            virtual void RenderResults(bool enable) = 0;
    };
} // namespace SwApi

#endif //__IAiResultsRenderer_H__

