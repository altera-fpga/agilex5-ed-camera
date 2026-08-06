/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

class SelectEvent
{
public:
    SelectEvent();
    ~SelectEvent();
    SelectEvent(const SelectEvent& other) = delete;
    SelectEvent& operator=(const SelectEvent& other) = delete;
    SelectEvent(SelectEvent&& other) = delete;
    SelectEvent& operator=(SelectEvent&& other) = delete;
    operator int() ;
    void Set();

private:
    int	_fds[2];
    bool _valid;
};

