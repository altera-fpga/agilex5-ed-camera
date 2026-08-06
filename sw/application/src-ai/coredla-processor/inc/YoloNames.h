/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef INC_YOLONAMES_H_
#define INC_YOLONAMES_H_

#include <string>
#include <vector>

class YoloNames : public std::vector<std::string>
{
public:
	YoloNames(const char* const NamesFile);
};

#endif /* INC_YOLONAMES_H_ */
