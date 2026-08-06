/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>
#include <vector>

namespace SwUtils
{
    enum class FileOpenFlags
    {
        READ,
        WRITE,
        READ_WRITE,
        APPEND
    };

    class IFile
    {
    public:
        virtual ~IFile() {}
        virtual bool Open(const std::filesystem::path& filename, FileOpenFlags fileFlags, bool isBinary) = 0;
        virtual bool Close() = 0;
        virtual bool IsOpen() = 0;
        virtual bool ReadLine(std::string& line) = 0;
        virtual bool Read(uint8_t* buffer, size_t size, size_t* pNumRead = nullptr) = 0;
        virtual bool Read(int32_t& value) = 0;
        virtual bool Read(int8_t& value) = 0;
        virtual bool Read(uint8_t& value) = 0;
        virtual bool Read(uint16_t& value) = 0;
        virtual bool Read(uint32_t& value) = 0;
        virtual bool Read(uint64_t& value) = 0;
        virtual bool WriteLine(std::string line) = 0;
        virtual bool WriteLine(const char* line, size_t length = 0) = 0;
        virtual bool Write(const uint8_t* buffer, size_t size) = 0;
        virtual bool Write(uint8_t value) = 0;
        virtual bool Write(int8_t value) = 0;
        virtual bool Write(int32_t value) = 0;
        virtual bool Write(uint16_t value) = 0;
        virtual bool Write(uint32_t value) = 0;
        virtual bool Write(uint64_t value) = 0;
        virtual std::shared_ptr<std::istream> GetIStream() = 0;
        virtual size_t GetSize() = 0;
        virtual size_t GetPosition() = 0;
        static std::shared_ptr<std::vector<uint8_t>> Load(const std::filesystem::path& filename);
        static std::shared_ptr<std::vector<std::string>> LoadStrings(const std::filesystem::path& filename);
    };

    class IFileSystem
    {
    public:
        virtual ~IFileSystem() {};
        virtual bool Exists(const std::filesystem::path& filename) = 0;
        virtual bool IsDirectory(const std::filesystem::path& filename) = 0;
        virtual bool Delete(const std::filesystem::path& filename) = 0;
        virtual std::shared_ptr<std::vector<uint8_t>> Load(const std::filesystem::path& filename) = 0;
        virtual std::shared_ptr<std::vector<std::string>> LoadStrings(const std::filesystem::path& filename) = 0;
        virtual bool FileHasSymbolicLink(const std::filesystem::path& filename) = 0;
        virtual std::shared_ptr<IFile> MakeFile() = 0;
    };

    class File : public IFile
    {
    public:
        File() : _filename("") {}
        bool Open(const std::filesystem::path& filename, FileOpenFlags fileFlags, bool isBinary) override;
        bool Close() override;
        bool IsOpen() override;
        bool ReadLine(std::string& line) override;
        bool Read(uint8_t* buffer, size_t size, size_t* pNumRead = nullptr) override;
        bool Read(int32_t& value) override;
        bool Read(int8_t& value) override;
        bool Read(uint8_t& value) override;
        bool Read(uint16_t& value) override;
        bool Read(uint32_t& value) override;
        bool Read(uint64_t& value) override;
        bool WriteLine(std::string line) override;
        bool WriteLine(const char* line, size_t length = 0) override;
        bool Write(const uint8_t* buffer, size_t size) override;
        bool Write(uint8_t value) override;
        bool Write(int8_t value) override;
        bool Write(int32_t value) override;
        bool Write(uint16_t value) override;
        bool Write(uint32_t value) override;
        bool Write(uint64_t value) override;
        std::shared_ptr<std::istream> GetIStream() override;
        size_t GetSize() override;
        size_t GetPosition() override;

    protected:
        std::filesystem::path		_filename;
        std::shared_ptr<std::fstream> _spFilestream;
        bool 			_binary = false;
        FileOpenFlags	_fileOpenFlags = FileOpenFlags::READ;
    };

    class FileSystem : public IFileSystem
    {
    public:
        bool Exists(const std::filesystem::path& filename) override;
        bool IsDirectory(const std::filesystem::path& filename) override;
        bool Delete(const std::filesystem::path& filename) override;
        std::shared_ptr<std::vector<uint8_t>> Load(const std::filesystem::path& filename) override;
        std::shared_ptr<std::vector<std::string>> LoadStrings(const std::filesystem::path& filename) override;
        bool FileHasSymbolicLink(const std::filesystem::path& filename) override;
        std::shared_ptr<IFile> MakeFile() override;
    };

} // namespace SwUtils

#ifdef __linux__
#define FILE_BEGIN 0
#define FILE_CURRENT 1
#define FILE_END 2
#endif
