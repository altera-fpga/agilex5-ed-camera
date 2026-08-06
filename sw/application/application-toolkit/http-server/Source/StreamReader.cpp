/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "StreamReader.h"

namespace OHttpServer
{

    StreamReader::StreamReader(const std::filesystem::path& filename)
    {
        _input_file.Open(filename, AtUtils::FileOpenFlags::READ, false);
    }

    StreamReader::~StreamReader()
    {
    }

    bool StreamReader::ReadLine(std::string& output_line)
    {
        return _input_file.ReadLine(output_line);
    }

    /////////////////////////


    BinaryStreamReader::BinaryStreamReader(std::shared_ptr<AtUtils::IFile> spFile, const std::filesystem::path& filename)
    {
        if (spFile->Open(filename, AtUtils::FileOpenFlags::READ, true))
        {
            size_t size = spFile->GetSize();
            _data.reset(new std::vector<uint8_t>(size));
            if (size > 0)
                spFile->Read(_data->data(), (uint32_t)size);
        }
    }

    std::shared_ptr<std::vector<uint8_t>> BinaryStreamReader::GetData()
    {
        return _data;
    }

} // namespace OHttpServer
