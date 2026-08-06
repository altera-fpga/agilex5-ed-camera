/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>

class EmbeddedFileHeader
{
public:
    uint32_t _size;
    char _relativeFileName[128];
};

class EmbeddedFile
{
public:
    EmbeddedFile(std::string relativeFileName, size_t dataSize, uint8_t* pData);
    std::shared_ptr<std::vector<uint8_t>> CopyData();

    std::string _relativeFileName;
    size_t _dataSize;
    uint8_t* _pData;
};

class EmbeddedFiles
{
public:
    EmbeddedFiles(char* pBinaryBlobStart, char* pBinarayBlobEnd);
    std::shared_ptr<EmbeddedFile> GetFile(std::string fileName);

private:
    std::recursive_mutex _mapCS;
    std::unordered_map<std::string, std::shared_ptr<EmbeddedFile>> _embeddedFiles;
};
