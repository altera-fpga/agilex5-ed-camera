/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "IHttpServer.h"
#include "CommonTypes.h"
#include "AtUtils.h"

class DataSegment
{
public:
    DataSegment(uint8_t* data = 0, uint32_t length = 0, bool isText = false);
    DataSegment(std::shared_ptr<ByteArray>& data, bool isText = false);

    std::shared_ptr<ByteArray> _spData;
    bool		_isText;
};

class DataSendRequest
{
    friend class DataSendRequestCmd;

public:
    DataSendRequest(std::vector<DataSegment>& dataSegments);
    DataSendRequest(std::shared_ptr<ByteArray> spBinaryData);
    ~DataSendRequest();

    void Write(AtUtils::IFile* pFile);
    std::vector<std::shared_ptr<ByteArray>> GetData(bool& isText);

protected:
    std::vector<DataSegment> _dataSegments;
};
