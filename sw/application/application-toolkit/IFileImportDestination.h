/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <string.h>
#include <stdint.h>
#include "FileTransfer.h"

class WebSocketFileTransfer;

class IFileImportDestination
{
public:
    virtual int NewFileImportStarted(std::string& filename, size_t index,
                                     size_t numFiles, bool isSequence, uint32_t clientID) = 0;

    virtual void ImportFile(std::shared_ptr<WebSocketFileTransfer> spWsFileTransfer) = 0;
};

