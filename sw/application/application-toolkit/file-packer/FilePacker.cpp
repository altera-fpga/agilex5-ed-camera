/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <iostream>
#include <fstream>
#include <cstring>
#include <filesystem>
#include <vector>

// To convert the output of this app to an object for linking:
// $LD -r -b binary -o WebServerHome.o WebServerHome.bin

class FileBlobHeader
{
public:
    uint32_t _size;
    char _relativeFileName[128];
};

int main(int argumentCount, char* argumentArray[])
{
	if (argumentCount != 3)
	{
		std::cout << "Usage: " << argumentArray[0] << " <source-folder> <output-file>\n";
		return -1;
	}

	size_t bigBlobSize = 0;
	size_t fileBlobHeaderSize = sizeof(FileBlobHeader);
	std::string startFolderStr = argumentArray[1];
	std::string outputFilename = argumentArray[2];
	std::filesystem::path startFolder = std::filesystem::absolute(startFolderStr);
	std::filesystem::recursive_directory_iterator directoryIterator(argumentArray[1]);

	std::vector<std::filesystem::directory_entry> fileList;

	// Iterate all the files in the start folder
	for (auto& filePath : directoryIterator)
	{
		if (filePath.is_regular_file())
		{
			size_t blobSize = filePath.file_size() + fileBlobHeaderSize;
			bigBlobSize += blobSize;

			fileList.push_back(filePath);
		}
	}

	// Allocate a blob to contain all the files, with their headers
	std::vector<uint8_t> bigBlob(bigBlobSize);
	uint8_t* pWriteBuffer = bigBlob.data();

	bool result = true;
	size_t filenameSize = sizeof(FileBlobHeader::_relativeFileName);

	for (auto& filePath : fileList)
	{
		size_t fileSize = filePath.file_size();
		size_t blobSize = fileSize + fileBlobHeaderSize;

		FileBlobHeader* pBlobHeader = (FileBlobHeader*)pWriteBuffer;
		pBlobHeader->_size = (uint32_t)blobSize;
		memset(pBlobHeader->_relativeFileName, 0, filenameSize);

		std::filesystem::path fullPath = filePath.path();
		std::filesystem::path absolutePath = std::filesystem::absolute(fullPath);

		// Strip front of the path so it is relative to the start folder
		auto pathPartIt = absolutePath.begin();
		for (auto& startPathPart : startFolder)
			pathPartIt++;

		std::string relativePath;
		while (pathPartIt != absolutePath.end())
		{
			std::string part = pathPartIt->generic_u8string();
			pathPartIt++;

			if (pathPartIt != absolutePath.end())
				relativePath += (part + "/");
			else
				relativePath += part;
		}

		// Copy the relative path name into the header
		strncpy(pBlobHeader->_relativeFileName, relativePath.c_str(), filenameSize);

		// Read each file into the blob
		std::fstream inputStream;
		inputStream.open(filePath.path(), std::ios_base::in | std::ios_base::binary);
		if (!inputStream.good())
		{
			std::cout << "Failed to read file '" << filePath.path() << "'\n";
			result = false;
			break;
		}

		pWriteBuffer += fileBlobHeaderSize;
		inputStream.read((char*)pWriteBuffer, fileSize);
		pWriteBuffer += fileSize;
	}

	if (result)
	{
		std::fstream outputStream;
		outputStream.open(outputFilename.c_str(), std::ios_base::out | std::ios_base::binary);
		if (outputStream.good())
		{
			outputStream.write((char*)bigBlob.data(), bigBlob.size());
		}
		else
		{
			std::cout << "Failed to write to file '" << outputFilename << "'\n";
			result = false;
		}
	}

	return result ? 0 : -1;
}
