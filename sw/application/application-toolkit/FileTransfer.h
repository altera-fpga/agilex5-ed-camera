/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <filesystem>
#include <mutex>
#include "IFileImportDestination.h"
#include "CommonTypes.h"

class WebSocketFileTransfer;
class FileTransferWebSocketCommandProcessor;
class ONamedParamList;
class IFileImportDestination;

class FileTransferItem
{
public:
    FileTransferItem(WebSocketFileTransfer* pOwner, std::string& fileName,
                     size_t fileSize, size_t index);
    ~FileTransferItem();

    FileTransferItem(const FileTransferItem& other) = delete;
    FileTransferItem& operator=(const FileTransferItem& other) = delete;
    FileTransferItem(FileTransferItem&& other) = delete;
    FileTransferItem& operator=(FileTransferItem&& other) = delete;

    bool		ReceiveBinaryData(std::vector<std::shared_ptr<ByteArray>>& binaryData, bool& finalFrame);
    bool		SaveFile();
    bool		IsAggregateFile();
    std::filesystem::path GetFileName() { return _fileName; }
    int			GetTransferId();
    int			GetPatternId();
    bool		IsLocalTransfer();
    size_t		GetSize() { return _fileSize; }
    void		Cancel();
    uint32_t	GetClientID();

    // Utilities for OAggregateFile access
    int64_t		GetPosition()	{ return _position; }
    void		Seek(int64_t position, int fileOffsetBase);
    bool		Read(uint8_t* buffer, size_t size, size_t* pNumRead = nullptr);
    bool		Read(int& value);
    bool		Read(uint16_t& value);
    bool		Read(uint32_t& value);
    bool		Read(uint64_t& value);
    void		UpdateFileDetails(std::filesystem::path fileName, size_t fileSize);

private:
    IFileImportDestination* GetFileImportDestination();
    void LoadLocalFile();

    std::filesystem::path _fileName;
    bool		_isSaved;
    size_t		_fileSize;
    std::recursive_mutex _unsavedDataCS;
    std::vector<std::shared_ptr<ByteArray>> _unsavedData;
    WebSocketFileTransfer*	_pOwner; // The pOwner has a smart pointer to us

    int64_t		_position;
    size_t		_positionSegment;
    size_t		_positionSegmentOffset;
    size_t		_index;
    int			_patternId;
    bool		_localFileLoaded;
    uint64_t	_receivedSoFar;
};

class WebSocketFileTransfer : public std::vector<std::shared_ptr<FileTransferItem>>,
                              public std::enable_shared_from_this<WebSocketFileTransfer>
{
    friend class FileTransferWebSocketCommandProcessor;

public:
    WebSocketFileTransfer(FileTransferWebSocketCommandProcessor* pOwner,
                          ONamedParamList& parameterList);
    ~WebSocketFileTransfer();

    WebSocketFileTransfer(const WebSocketFileTransfer& other) = delete;
    WebSocketFileTransfer& operator=(const WebSocketFileTransfer& other) = delete;
    WebSocketFileTransfer(WebSocketFileTransfer&& other) = delete;
    WebSocketFileTransfer& operator=(WebSocketFileTransfer&& other) = delete;

    void Cancel();

    void		TransferFileStart(ONamedParamList& parameterList);
    bool		ReceiveBinaryData(std::vector<std::shared_ptr<ByteArray>>& binaryData);
    int			GetTransferId();
    void		AddLocalFile(std::string localFilename);
    bool		IsLocalTransfer()						{ return _isLocalTransfer; }
    IFileImportDestination* GetFileImportDestination()	{ return _pFileImportDestination; }
    size_t		GetNumFiles()							{ return _numFiles; }
    std::filesystem::path	GetSequenceFolder()			{ return _sequenceFolder; }
    void ImportFileTransfer();
    uint32_t	GetClientID();

private:
    void RequestNextFile();
    void TransferComplete();
    std::filesystem::path GetImportFolder(bool& status);
    std::string CreateNewFolderName(std::string folder, std::string suppliedTitle);

    FileTransferWebSocketCommandProcessor*	_pOwner;
    size_t					_numFiles;
    size_t					_currentFileIndex;
    std::filesystem::path	_sequenceFolder = "";
    int						_transferId;
    uint32_t				_clientId;
    std::shared_ptr<FileTransferItem>	_spCurrentFile;
    bool					_isLocalTransfer;
    IFileImportDestination*	_pFileImportDestination;
    bool					_transferComplete;
    uint32_t				_itemId;
    std::vector<std::string> _fileTitles;
};
