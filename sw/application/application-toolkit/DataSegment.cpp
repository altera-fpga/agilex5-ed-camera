/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "DataSegment.h"
#include <cstring>

DataSegment::DataSegment(uint8_t* data, uint32_t length, bool isText)
:	_isText(isText)
{
    _spData.reset(new ByteArray(length));
    AtUtils::CopyMemoryBuffer(_spData->data(), _spData->size(), data, length);
}

DataSegment::DataSegment(std::shared_ptr<ByteArray>& data, bool isText)
:	_spData(data)
,	_isText(isText)
{
}

/////

DataSendRequest::DataSendRequest(std::vector<DataSegment>& dataSegments)
:	_dataSegments(dataSegments)
{
}

DataSendRequest::DataSendRequest(std::shared_ptr<ByteArray> spBinaryData)
{
    _dataSegments.push_back(DataSegment(spBinaryData));
}

DataSendRequest::~DataSendRequest()
{
}

void DataSendRequest::Write(AtUtils::IFile* pFile)
{
    for (auto& dataSegment : _dataSegments)
        pFile->Write(dataSegment._spData->data(), dataSegment._spData->size());
}

std::vector<std::shared_ptr<ByteArray>> DataSendRequest::GetData(bool& isText)
{
    std::vector<std::shared_ptr<ByteArray>> data;

    if (!_dataSegments.empty())
        isText = _dataSegments[0]._isText;

    for (auto& dataSegment : _dataSegments)
        data.push_back(dataSegment._spData);

    return data;
}
