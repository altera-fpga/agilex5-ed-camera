/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "EmbeddedFile.h"
#include "AtUtils.h"
#include <iostream>
#include <fstream>

EmbeddedFile::EmbeddedFile(std::string relativeFileName, size_t dataSize, uint8_t* pData)
:   _relativeFileName(std::move(relativeFileName))
,   _dataSize(dataSize)
,   _pData(pData)
{
}

std::shared_ptr<std::vector<uint8_t>> EmbeddedFile::CopyData()
{
    auto spData = std::make_shared<std::vector<uint8_t>>(_dataSize);
    if (spData)
        memcpy(spData->data(), _pData, _dataSize);
    return spData;
}

EmbeddedFiles::EmbeddedFiles(char* pBinaryBlobStart, char* pBinarayBlobEnd)
{
    EmbeddedFileHeader* fileHeader = (EmbeddedFileHeader*)pBinaryBlobStart;
    EmbeddedFileHeader* endFileHeader = (EmbeddedFileHeader*)pBinarayBlobEnd;

    while (fileHeader != endFileHeader)
    {
        uint32_t blobSize = fileHeader->_size;
        std::string name = fileHeader->_relativeFileName;
        uint8_t* pData = (uint8_t*)(fileHeader + 1);
        size_t fileSize = blobSize - sizeof(EmbeddedFileHeader);

        auto spEmbeddedFile = std::make_shared<EmbeddedFile>(name, fileSize, pData);

        // Check we don't have this already
        std::shared_ptr<EmbeddedFile> spExistingFile;
        if (AtUtils::Lookup<std::string, std::shared_ptr<EmbeddedFile>>(_embeddedFiles, name, spExistingFile))
            std::cout << "Duplicate embedded file name '" << name << "'\n";
        else
            _embeddedFiles[name] = std::move(spEmbeddedFile);

        fileHeader = (EmbeddedFileHeader*)((uint8_t*)fileHeader + blobSize);
    }
}

std::shared_ptr<EmbeddedFile> EmbeddedFiles::GetFile(std::string fileName)
{
    if (AtUtils::Left(fileName, 1) == "/")
        fileName = AtUtils::Mid(fileName, 1);

    std::shared_ptr<EmbeddedFile> spExistingFile;

    // Lock access to _embeddedFiles map
    std::lock_guard lock(_mapCS);
    if (!AtUtils::Lookup<std::string, std::shared_ptr<EmbeddedFile>>(_embeddedFiles, fileName, spExistingFile))
        std::cout << "Not found '" << fileName << "'\n";
    return spExistingFile;
}
