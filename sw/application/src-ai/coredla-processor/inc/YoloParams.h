/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef INC_YOLOPARAMS_H_
#define INC_YOLOPARAMS_H_

#include <vector>
#include "InferenceTypes.h"

class YoloParams {
public:
    int _original_im_h = 0;
    int _original_im_w = 0;
    int _num = 0;
    int _classes = 0;
    int _coords = 0;
    int _keypoints = 0;
    int _input_tensor_w = 0;
    int _input_tensor_h = 0;
    int _output_tensor_predictions = 0;

    std::vector<float> _anchor_x;
    std::vector<float> _anchor_y;
    std::vector<float> _anchor_mult;

    void init(const ModelPtr& network_model) {
        if(network_model)
        {
            ov::Output<const ov::Node> input_port = network_model->_ovCompiledModel.input();
            const std::vector<ov::Output<const ov::Node>>& output_ports = network_model->_ovCompiledModel.outputs();
        
            _input_tensor_h = input_port.get_shape()[2];
            _input_tensor_w = input_port.get_shape()[3];

            _output_tensor_predictions = 0;
            for(size_t op = 0U; op < 3U; op++)
            {
                _output_tensor_predictions += output_ports[op].get_shape()[2] * output_ports[op].get_shape()[3];
            }

            _anchor_x.resize(_output_tensor_predictions);
            _anchor_y.resize(_output_tensor_predictions);
            _anchor_mult.resize(_output_tensor_predictions);

            int output_h = _input_tensor_h / 32;
            int output_w = _input_tensor_w / 32;

            int l1 = 4*output_h*4*output_w;
            int l2 = l1 + 2*output_h*2*output_w;

            for(int i = 0; i < _output_tensor_predictions; i++)
            {
                int anchor_size_x;
                int anchor_size_y;
                int anchor_offset;

                if( i < l1) {
                    anchor_size_x = 4*output_w;
                    anchor_size_y = 4*output_h;
                    anchor_offset = 0;
                    _anchor_mult[i] = 8.0f;
                }
                else if(i < l2) {
                    anchor_size_x = 2*output_w;
                    anchor_size_y = 2*output_h;
                    anchor_offset = l1;
                    _anchor_mult[i] = 16.0f;
                }
                else {
                    anchor_size_x = output_w;
                    anchor_size_y = output_h;
                    anchor_offset = l2;
                    _anchor_mult[i] = 32.0f;
                }
                (void)anchor_size_y;
                int ax = (i-anchor_offset) % anchor_size_x;
                int ay = (i-anchor_offset) / anchor_size_x;
                _anchor_x[i] = static_cast<float>(ax);
                _anchor_y[i] = static_cast<float>(ay);
            }
        }
    }

};
#endif /* INC_YOLOPARAMS_H_ */
