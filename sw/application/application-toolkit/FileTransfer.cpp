/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AtUtils.h"
#include "FileTransfer.h"
#include "ParamList.h"
#include "FileTransferWebSocketCommandProcessor.h"
#include "CommonUiUpdate.h"
#include "CommonApplicationBase.h"

WebSocketFileTransfer::WebSocketFileTransfer(FileTransferWebSocketCommandProcessor* pOwner,
                                             ONamedParamList& parameterList)
:	_pOwner(pOwner)
,	_numFiles(0)
,	_currentFileIndex(0)
,	_transferId(0)
,	_clientId(0)
,	_isLocalTransfer(false)
,	_pFileImportDestination(nullptr)
,	_transferComplete(false)
,	_itemId(0)
{
    AtUtils::FromString(parameterList.GetParam("NumFiles"), _numFiles);
    AtUtils::FromString(parameterList.GetParam("TransferId"), _transferId);
    AtUtils::FromString(parameterList.GetParam("ClientId"), _clientId);

    if (_numFiles > 100)
        _numFiles = 100;

    if (pOwner)
    {
        for (size_t i = 0; i < _numFiles; i++)
        {
            std::string paramId = AtUtils::FormatString("FileName%zu", i);
            std::string fileName = parameterList.GetParam(paramId.c_str());
            std::cout << "Pending file tranfer " << fileName << std::endl;
            _fileTitles.push_back(std::move(fileName));
        }
    }

    _pFileImportDestination = CommonApplicationBase::Get();

    if (pOwner)
        RequestNextFile();
}

WebSocketFileTransfer::~WebSocketFileTransfer()
{
    _spCurrentFile = nullptr;
}

void WebSocketFileTransfer::Cancel()
{
    if (_transferComplete)
        return;

    std::filesystem::remove_all(_sequenceFolder);
    TransferComplete();
}

void WebSocketFileTransfer::RequestNextFile()
{
    if (_currentFileIndex >= _fileTitles.size())
        return;

    _spCurrentFile = std::make_shared<FileTransferItem>(this, _fileTitles[_currentFileIndex++], 0, size());
    _itemId = _spCurrentFile->GetPatternId();

    if (_pOwner)
    {
        JsonDOM jsonDOM("SendMeFile");
        AtUtils::IJsonObjectPtr spRootObject = jsonDOM.GetRoot();
        spRootObject->AddValue("transfer_id", _transferId);
        spRootObject->AddValue("file_index", (uint32_t)size());
        spRootObject->AddValue("item_id", _itemId);
        std::shared_ptr<std::string> spXmlString = jsonDOM.ToString();
        _pOwner->IWebSocketCallback::SendToClient(new TextWebSocketMessage(std::move(spXmlString)));
    }
}

void WebSocketFileTransfer::TransferComplete()
{
    _transferComplete = true;

    if (_pOwner)
    {
        JsonDOM jsonDOM("TransferComplete");
        AtUtils::IJsonObjectPtr spRootObject = jsonDOM.GetRoot();
        spRootObject->AddValue("transfer_id", _transferId);
        std::shared_ptr<std::string> spXmlString = jsonDOM.ToString();
        _pOwner->IWebSocketCallback::SendToClient(new TextWebSocketMessage(std::move(spXmlString)));
    }
}

std::string WebSocketFileTransfer::CreateNewFolderName(std::string folder, std::string suppliedTitle)
{
    std::string folderTitle = suppliedTitle;
    AtUtils::Replace(folderTitle, ".", "_");
    if (suppliedTitle.empty())
        folderTitle = AtUtils::FormatString("New Sequence");

    std::string testFolder;
    std::string testTitle = folderTitle;

    for (int i = 2; i < 10000; i++)
    {
        testFolder = folder + testTitle;

        std::filesystem::path path(testFolder);
        if (!std::filesystem::exists(path))
            break;

        testTitle = AtUtils::FormatString("%s (%d)", folderTitle.c_str(), i);
    }

    return testFolder;
}

void WebSocketFileTransfer::TransferFileStart(ONamedParamList& parameterList)
{
    std::string filename = parameterList.GetParam("Filename");

    bool status = false;
    std::filesystem::path currentFilename = GetImportFolder(status);
    if (!status)
        return;
    currentFilename /= filename;

    int fileSize = 0;
    AtUtils::FromString(parameterList.GetParam("FileSize"), fileSize);

    if (_spCurrentFile)
    {
        // Update the transfer item with the filename and its size
        _spCurrentFile->UpdateFileDetails(std::move(currentFilename), fileSize);
    }

    int numSegment = 0;
    AtUtils::FromString(parameterList.GetParam("NumSegments"), numSegment);

    int fileIndex = 0;
    AtUtils::FromString(parameterList.GetParam("FileIndex"), fileIndex);

    assert(fileIndex == static_cast<int>(size()));

    push_back(_spCurrentFile);
}

// 'me' is a smart pointer to this
bool WebSocketFileTransfer::ReceiveBinaryData(std::vector<std::shared_ptr<ByteArray>>& binaryData)
{
    if (!_spCurrentFile)
    {
        std::cout << "Failed to open file so delete transfer\n";
        return true; // Failed to open file so delete transfer
    }

    bool finalFrame = false;
    if (!_spCurrentFile->ReceiveBinaryData(binaryData, finalFrame))
    {
        std::cout << "Failed to open file so delete transfer\n";
        return true;  // Failed to open file so delete transfer
    }

    bool finished = false;
    if (finalFrame)
    {
        std::cout << "File transfer final frame\n";

        std::shared_ptr<FileTransferItem> spTransferItem = at(0);
        std::string transferFilename = spTransferItem->GetFileName().string();
        NewInfoLabel("File upload, save in progress '%s'", transferFilename.c_str());

        if (!_spCurrentFile->SaveFile())
        {
            std::cout << "Failed to open file so delete transfer\n";
            return true;  // Failed to open file so delete transfer
        }

        // Close the file and move to the next if any
        _spCurrentFile = nullptr;

        finished = (size() >= _numFiles);

        if (finished)
        {
            ImportFileTransfer();
            TransferComplete();
        }
        else
        {
            RequestNextFile();
        }
    }

    return finished;
}

void WebSocketFileTransfer::AddLocalFile(std::string localFileName)
{
    assert(0);
}

void WebSocketFileTransfer::ImportFileTransfer()
{
    CommonApplicationBase::Get()->ImportFile(shared_from_this());
}

std::filesystem::path WebSocketFileTransfer::GetImportFolder(bool& status)
{
    std::filesystem::path importFolder = std::filesystem::current_path();
    importFolder /= "WebToolkit/import-files";
    if (std::filesystem::exists(importFolder))
        status = true;
    else
        status = std::filesystem::create_directories(importFolder);
    return importFolder;
}

int WebSocketFileTransfer::GetTransferId()
{
    return _transferId;
}

uint32_t WebSocketFileTransfer::GetClientID()
{
    return _clientId;
}

///////////////////////////////////////////////////////////////////////////////

FileTransferItem::FileTransferItem(WebSocketFileTransfer* pOwner,
                                   std::string& fileName,
                                   size_t fileSize,
                                   size_t index)
:	_fileName(fileName)
,	_isSaved(false)
,	_fileSize(fileSize)
,	_pOwner(pOwner)
,	_position(0)
,	_positionSegment(0)
,	_positionSegmentOffset(0)
,	_index(index)
,	_patternId(0)
,	_localFileLoaded(false)
,	_receivedSoFar(0)
{
    uint32_t clientId = pOwner->GetClientID();
    _patternId = GetFileImportDestination()->NewFileImportStarted(fileName, index,
                                                                  pOwner->GetNumFiles(),
                                                                  false, clientId);
}

FileTransferItem::~FileTransferItem()
{
    if (IsAggregateFile() && IsLocalTransfer())
    {
        // We don't need to keep the copied aggregate file
		std::error_code ec;
        std::filesystem::remove(_fileName, ec);
    }
}

int FileTransferItem::GetTransferId()
{
    return _pOwner->GetTransferId();
}

int FileTransferItem::GetPatternId()
{
    return _patternId;
}

bool FileTransferItem::IsLocalTransfer()
{
    return _pOwner->IsLocalTransfer();
}

uint32_t FileTransferItem::GetClientID()
{
    return _pOwner->GetClientID();
}

IFileImportDestination* FileTransferItem::GetFileImportDestination()
{
    return _pOwner->GetFileImportDestination();
}

bool FileTransferItem::IsAggregateFile()
{
    return (_fileName.extension() == "oaf");
}

bool FileTransferItem::ReceiveBinaryData(std::vector<std::shared_ptr<ByteArray>>& binaryData,
                                         bool& finalFrame)
{
    std::lock_guard lock(_unsavedDataCS);
    _unsavedData.insert(_unsavedData.end(), binaryData.begin(), binaryData.end());

    for (size_t i = 0; i < binaryData.size(); i++)
        _receivedSoFar += binaryData[i]->size();

    size_t nFiles = _pOwner->GetNumFiles();
    (void)nFiles;

    double receivedPercent = (100.0 * (double)_receivedSoFar) / (double)_fileSize;
    (void)receivedPercent;

    finalFrame = (static_cast<size_t>(_receivedSoFar) == _fileSize);
    return true;
}

bool FileTransferItem::SaveFile()
{
    AtUtils::File output_file;
    if (output_file.Open(_fileName, AtUtils::FileOpenFlags::WRITE, true))
    {
        _isSaved = true;
        size_t nItems = _unsavedData.size();
        size_t buffer_size = 0x20000 * 4;
        std::vector<uint8_t> concatenatedBuffer(buffer_size);
        size_t bufferPosition = 0;
        size_t inputPosition = 0;

        for (size_t i = 0; i < nItems; /* nothing */)
        {
            size_t sourceSize = _unsavedData[i]->size() - inputPosition;
            size_t available = buffer_size - bufferPosition;
            size_t nToCopy = std::min(sourceSize, available);

            uint8_t* data = &_unsavedData[i]->at(inputPosition);
			size_t dmax = buffer_size - bufferPosition;
            AtUtils::CopyMemoryBuffer(&concatenatedBuffer.at(bufferPosition), dmax, data, nToCopy);

            bufferPosition += nToCopy;

            if ((bufferPosition == buffer_size) || (i == (nItems - 1)))
            {
                //SaveItemProgressUiUpdate save_item_update(this, i, nItems, _index, nFiles);
                //save_item_update.Notify();

                if (output_file.Write(&concatenatedBuffer.at(0), bufferPosition))
                    bufferPosition = 0;
                else
                    return false;
            }

            if (nToCopy < sourceSize)
            {
                // There's more in the source buffer
                inputPosition += nToCopy;
            }
            else
            {
                inputPosition = 0;

                // Move to the next input buffer
                i++;
            }
        }

        // Release the memory
        _unsavedData.clear();

        return true;
    }
    else
        return false;
}

void FileTransferItem::Seek(int64_t position, int fileOffsetBase)
{
    std::lock_guard lock(_unsavedDataCS);

    if (fileOffsetBase == FILE_BEGIN)
        _position = position;
    else if (fileOffsetBase == FILE_CURRENT)
        _position += position;
    else if (fileOffsetBase == FILE_END)
        assert(0);

    _positionSegment = 0;
    _positionSegmentOffset = 0;
    size_t absPosition = (size_t)_position;

    for (size_t i = 0; i < _unsavedData.size(); i++)
    {
        size_t segmentSize = _unsavedData[i]->size();

        if (absPosition < segmentSize)
        {
            _positionSegment = i;
            _positionSegmentOffset = position;
            return;
        }

        absPosition -= segmentSize;
    }
}

void FileTransferItem::LoadLocalFile()
{
    assert(0);
}

bool FileTransferItem::Read(uint8_t* buffer, size_t size, size_t* num_read)
{
    std::lock_guard lock(_unsavedDataCS);
    size_t numRead = 0;

    while (size > 0)
    {
        if (IsLocalTransfer() && !_localFileLoaded)
            LoadLocalFile();

        if (_positionSegment >= _unsavedData.size())
            return false;

        size_t segmentSize = _unsavedData[_positionSegment]->size();
        size_t segmentSpace = segmentSize - _positionSegmentOffset;

        if (size <= segmentSpace)
        {
            AtUtils::CopyMemoryBuffer(buffer, size, &_unsavedData[_positionSegment]->at(_positionSegmentOffset), size);
            _positionSegmentOffset += size;
            buffer += size;
            numRead += size;
            _position += size;
            if (_positionSegmentOffset == segmentSize)
            {
                _positionSegmentOffset = 0;
                _positionSegment++;
            }
            size = 0;
        }
        else
        {
            if (segmentSpace > 0)
            {
                AtUtils::CopyMemoryBuffer(buffer, size, &_unsavedData[_positionSegment]->at(_positionSegmentOffset), segmentSpace);
                size -= segmentSpace;
                buffer += segmentSpace;
                numRead += segmentSpace;
                _position += segmentSpace;
            }
            _positionSegmentOffset = 0;
            _positionSegment++;
        }
    }

    if (num_read)
        *num_read = numRead;

    return true;
}

bool FileTransferItem::Read(int& value)
{
    return Read((uint8_t*)&value, sizeof(value));
}

bool FileTransferItem::Read(uint16_t& value)
{
    return Read((uint8_t*)&value, sizeof(value));
}

bool FileTransferItem::Read(uint32_t& value)
{
    return Read((uint8_t*)&value, sizeof(value));
}

bool FileTransferItem::Read(uint64_t& value)
{
    return Read((uint8_t*)&value, sizeof(value));
}

void FileTransferItem::Cancel()
{
    if (_isSaved)
	{
		std::error_code ec;
        std::filesystem::remove(_fileName, ec);
	}
}

void FileTransferItem::UpdateFileDetails(std::filesystem::path fileName, size_t fileSize)
{
    _fileName = fileName;
    _fileSize = fileSize;
}
