/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AtUtils.h"
#include <string>
#include <cstring>

namespace AtUtils
{

    bool File::Close()
    {
        if (_spFilestream)
        {
            _spFilestream = nullptr;
            return true;
        }
        else
            return false;
    }

    bool File::Open(const std::filesystem::path& filename, FileOpenFlags fileOpenFlags, bool isBinary)
    {
        _filename = filename;
        _fileOpenFlags = fileOpenFlags;
        _binary = isBinary;

        _spFilestream.reset(new std::fstream());

        std::ios_base::openmode mode = (std::ios_base::openmode)0;

        if (fileOpenFlags == FileOpenFlags::READ)
            mode = std::ios_base::in;
        else if (fileOpenFlags == FileOpenFlags::WRITE)
            mode = std::ios_base::out;
        else if (fileOpenFlags == FileOpenFlags::APPEND)
            mode = std::ios_base::out | std::ios_base::app;
        else if (fileOpenFlags == FileOpenFlags::READ_WRITE)
            mode = std::ios_base::in | std::ios_base::out;

        if (_binary)
            mode |= std::ios_base::binary;

        _spFilestream->open(_filename, mode);

        if (_spFilestream->good())
        {
            if (!_binary && (fileOpenFlags == FileOpenFlags::READ))
            {
                uint8_t bom[3];
                bom[0] = bom[1] = bom[2] = 0;

                _spFilestream->read((char*)bom, 3);

                if ((bom[0] != 0xef) || (bom[1] != 0xbb) || (bom[2] != 0xbf))
                {
                    _spFilestream->seekg(0, std::ios_base::beg);
                }
            }

            return true;
        }
        else
            return false;
    }

    bool File::IsOpen()
    {
        if (_spFilestream)
            return _spFilestream->good();
        else
            return false;
    }

    bool File::ReadLine(std::string& line)
    {
        if (_spFilestream)
            return (getline(*_spFilestream, line)) ? true : false;
        else
            return false;
    }

    bool File::Read(uint8_t* buffer, size_t size, size_t* pNumRead)
    {
        if (_spFilestream)
        {
            _spFilestream->read((char*)buffer, size);
            if (pNumRead)
                *pNumRead = size;
            return true;
        }
        else
            return false;
    }

    bool File::Read(int32_t& value)
    {
        size_t numRead = 0;
        return Read((uint8_t*)&value, sizeof(value), &numRead);
    }

    bool File::Read(int8_t& value)
    {
        size_t numRead = 0;
        return Read((uint8_t*)&value, sizeof(value), &numRead);
    }

    bool File::Read(uint8_t& value)
    {
        size_t numRead = 0;
        return Read(&value, sizeof(value), &numRead);
    }

    bool File::Read(uint32_t& value)
    {
        size_t numRead = 0;
        return Read((uint8_t*)&value, sizeof(value), &numRead);
    }

    bool File::Read(uint64_t& value)
    {
        size_t numRead = 0;
        return Read((uint8_t*)&value, sizeof(value), &numRead);
    }

    bool File::Read(uint16_t& value)
    {
        size_t numRead = 0;
        return Read((uint8_t*)&value, sizeof(value), &numRead);
    }

    bool File::WriteLine(std::string line)
    {
        if (_binary || !_spFilestream)
            return false;

        bool written = (*_spFilestream << line << std::endl) ? true : false;
        return written;
    }

    bool File::WriteLine(const char* line, size_t length)
    {
        if (_binary || !_spFilestream)
            return false;

        if (length == 0)
            length = (uint32_t)strlen(line);

        bool written = (*_spFilestream << line << std::endl) ? true : false;
        return written;
    }

    bool File::Write(const uint8_t* buffer, size_t size)
    {
        if (!_spFilestream)
            return false;

        return _spFilestream->write((char*)buffer, size) ? true : false;
    }

    bool File::Write(int32_t value)
    {
        return Write((uint8_t*)&value, sizeof(value));
    }

    bool File::Write(uint32_t value)
    {
        return Write((uint8_t*)&value, sizeof(value));
    }

    bool File::Write(uint8_t value)
    {
        return Write((uint8_t*)&value, sizeof(value));
    }

    bool File::Write(int8_t value)
    {
        return Write((uint8_t*)&value, sizeof(value));
    }

    bool File::Write(uint64_t value)
    {
        return Write((uint8_t*)&value, sizeof(value));
    }

    bool File::Write(uint16_t value)
    {
        return Write((uint8_t*)&value, sizeof(value));
    }

    std::shared_ptr<std::istream> File::GetIStream()
    {
        return _spFilestream;
    }

    size_t File::GetSize()
    {
        if (_spFilestream)
        {
            std::streampos currentPosition = _spFilestream->tellg();
            _spFilestream->seekg(0, std::ios::beg);
            std::streampos startPosition = _spFilestream->tellg();
            _spFilestream->seekg(0, std::ios::end);
            std::streampos endPosition = _spFilestream->tellg();
            _spFilestream->seekg(currentPosition, std::ios::beg);
            uint64_t size = endPosition - startPosition;
            return size;
        }
        else
            return 0;
    }

    size_t File::GetPosition()
    {
        if (_spFilestream)
        {
            std::streampos currentPosition = _spFilestream->tellg();
            return currentPosition;
        }
        else
            return 0;
    }

    bool FileSystem::Exists(const std::filesystem::path& filename)
    {
        return std::filesystem::exists(filename);
    }

    bool FileSystem::IsDirectory(const std::filesystem::path& filename)
    {
        return std::filesystem::is_directory(filename);
    }

    bool FileSystem::Delete(const std::filesystem::path& filename)
    {
        std::error_code ec;
        return std::filesystem::remove(filename, ec);
    }

    std::shared_ptr<std::vector<uint8_t>> FileSystem::Load(const std::filesystem::path& filename)
    {
        std::shared_ptr<std::vector<uint8_t>> spData;

        File file;
        if (file.Open(filename, FileOpenFlags::READ, true))
        {
            spData = std::make_shared<std::vector<uint8_t>>(file.GetSize());
            file.Read(spData->data(), spData->size());
        }

        return spData;
    }

    std::shared_ptr<std::vector<std::string>> FileSystem::LoadStrings(const std::filesystem::path& filename)
    {
        std::shared_ptr<std::vector<std::string>> spStrings;

        File file;
        if (file.Open(filename, FileOpenFlags::READ, false))
        {
            spStrings = std::make_shared<std::vector<std::string>>();

            while (true)
            {
                std::string line;
                if (!file.ReadLine(line))
                    break;

                spStrings->push_back(std::move(line));
            }
        }

        return spStrings;
    }

    bool FileSystem::FileHasSymbolicLink(const std::filesystem::path& inFilename)
    {
        std::filesystem::path filename(inFilename);
        while (true)
        {
            if (std::filesystem::is_symlink(filename))
                return true;

            if (!filename.has_relative_path())
                return false;

            filename = filename.parent_path();
        }
    }

    std::shared_ptr<IFile> FileSystem::MakeFile()
    {
        return std::make_shared<File>();
    }

} // namespace AtUtils
