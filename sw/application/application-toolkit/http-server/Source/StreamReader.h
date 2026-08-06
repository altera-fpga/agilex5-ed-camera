/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "AtUtils.h"

namespace OHttpServer
{

    class StreamReader
    {
    public:
        StreamReader(const std::filesystem::path& filename);
        ~StreamReader();

        bool ReadLine(std::string& output_line);

    private:
        AtUtils::File _input_file;
    };

    class BinaryStreamReader
    {
    public:
        BinaryStreamReader(std::shared_ptr<AtUtils::IFile> spFile, const std::filesystem::path& filename);
        std::shared_ptr<std::vector<uint8_t>> GetData();

    private:
        std::shared_ptr<std::vector<uint8_t>>	_data;
    };

} // namespace OHttpServer
