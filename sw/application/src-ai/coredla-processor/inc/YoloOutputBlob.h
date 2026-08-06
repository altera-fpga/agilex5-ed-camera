/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef INC_YOLOOUTPUTBLOB_H_
#define INC_YOLOOUTPUTBLOB_H_

#include <cstring>

#include "InferenceTypes.h"
#include "YoloParams.h"

class YoloOutputBlob
{
public:
    YoloOutputBlob(
        const YoloParams& params,
        const std::string & output_name,
        const ov::Tensor &output_tensor
    )
    : _params(params)
    , _output_name(output_name)
    , _output_tensor(output_tensor)
    {}

    const YoloParams& _params;
    const std::string & _output_name;
    const ov::Tensor &_output_tensor;
};

#endif /* INC_YOLOOUTPUTBLOB_H_ */
