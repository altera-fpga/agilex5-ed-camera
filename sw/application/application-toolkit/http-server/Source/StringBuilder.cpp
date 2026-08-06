/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "StringBuilder.h"

using namespace OHttpServer;

StringBuilder::StringBuilder()
{
}

StringBuilder::~StringBuilder()
{
}

StringBuilder::operator std::string()
{
    return _string;
}

void StringBuilder::Append(std::string& text)
{
    _string += text;
}

void StringBuilder::Append(const char* text)
{
    _string += text;
}
