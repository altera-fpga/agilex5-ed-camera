/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "OWarpFile.h"
#include "SwUtils.h"

namespace SwApi
{
    OWarpFile::OWarpFile()
    {
    }

    OWarpFile::OWarpFile(OwfMeshType meshType, OwfResolution resolution,
                            uint8_t horizontalAspect, uint8_t verticalAspect,
                            uint16_t numRows, uint16_t numColumns,
                            uint8_t sampleSize, uint8_t sampleFormat,
                            const PointsArrayPtr& spPointsArray)
    :   _meshType(meshType)
    ,   _resolution(resolution)
    ,   _horizontalAspect(horizontalAspect)
    ,   _verticalAspect(verticalAspect)
    ,   _numRows(numRows)
    ,   _numColumns(numColumns)
    ,   _sampleSize(sampleSize)
    ,   _sampleFormat(sampleFormat)
    ,   _spPointsArray(spPointsArray)                                                 
    {
        _bitsScale = static_cast<float>(1 << _sampleFormat);
    }                                                 

    uint32_t OWarpFile::ToUint32(float value)
    {
        return static_cast<uint32_t>(value * _bitsScale);
    }

    float OWarpFile::FromUint32(uint32_t value)
    {
        return static_cast<float>(value) / _bitsScale;
    }

    bool OWarpFile::Save(std::filesystem::path filePath)
    {
        if ((_numRows == 0) || (_numColumns == 0) || (_spPointsArray == nullptr))
            return false;

        SwUtils::File outputFile;
        if (!outputFile.Open(filePath, SwUtils::FileOpenFlags::WRITE, true))
            return false;

        std::vector<uint8_t> header;
        header.push_back('O');
        header.push_back('W');
        header.push_back('F');

        // File format version
        header.push_back(1);

        // Mesh File Type
        uint8_t byte_value = 0;
        if (_meshType == OwfMeshType::INVERSE)
            byte_value |= (1 << 0);
        if (_resolution == OwfResolution::INDEPENDENT)
            byte_value |= (1 << 1);
        header.push_back(byte_value);

        // Number of warp meshes in the file
        header.push_back(1);

        // Applicable colour channel
        header.push_back(0xf0);
        header.push_back(0x00);

        // Horizontal grid resolution
        header.push_back(0x00);
        header.push_back(_horizontalAspect);

        // Vertical grid resolution
        header.push_back(0x00);
        header.push_back(_verticalAspect);

        // Horizontal grid points
        header.push_back((uint8_t)(_numColumns & 0xff));
        header.push_back((uint8_t)(_numColumns >> 8));

        // Vertical grid points
        header.push_back((uint8_t)(_numRows & 0xff));
        header.push_back((uint8_t)(_numRows >> 8));

        // Mesh sample size
        header.push_back(_sampleSize);

        // Mesh sample format
        header.push_back(_sampleFormat);

        // Reserved
        for (size_t i = 0; i < 12; i++)
            header.push_back(0x00);

        uint16_t checksum = 0;
        if (header.size() != 30)
            return false;
        uint16_t* pData16 = (uint16_t*)header.data();
        for (size_t i = 0; i < 15; i++)
            checksum += pData16[i];

        uint16_t checksum_value = -checksum;
        header.push_back(0);
        header.push_back(0);
        pData16 = (uint16_t*)header.data();
        pData16[15] = checksum_value;

        // Write the header
        outputFile.Write(header.data(), header.size());

        // Write the payload
        uint8_t* pData = (uint8_t*)_spPointsArray->data();
        size_t dataSize = _spPointsArray->size() * sizeof(uint32_t);
        outputFile.Write(pData, dataSize);

        return true;
    }

    bool OWarpFile::Load(std::filesystem::path filePath)
    {
        auto spData = SwUtils::File::Load(filePath);
        if (!spData)
            return false;

        if (spData->size() < 32)
            return false;

        size_t i = 0;
        uint8_t* pData = spData->data();

        if (pData[i++] != 'O')
            return false;
        if (pData[i++] != 'W')
            return false;
        if (pData[i++] != 'F')
            return false;
            
        // File format version
        if (pData[i++] != 1)
            return false;

        // Mesh File Type
        uint8_t byteValue = pData[i++];
        _meshType = ((byteValue & (1 << 0)) != 0) ? OwfMeshType::INVERSE : OwfMeshType::FORWARD;
        _resolution = ((byteValue & (1 << 1)) != 0) ? OwfResolution::INDEPENDENT : OwfResolution::FIXED;

        // Number of warp meshes in the file
        if (pData[i++] != 1)
            return false;

        // Applicable colour channel
        if (pData[i++] != 0xf0)
            return false;
        if (pData[i++] != 0x00)
            return false;

        // Horizontal grid resolution
        if (pData[i++] != 0x00)
            return false;
        _horizontalAspect = pData[i++];

        // Vertical grid resolution
        if (pData[i++] != 0x00)
            return false;
        _verticalAspect = pData[i++];

        // Horizontal grid points
        _numColumns = *((uint16_t*)(pData + i));
        i += 2;

        // Vertical grid points
        _numRows = *((uint16_t*)(pData + i));
        i += 2;

        // Mesh sample size
        _sampleSize = pData[i++];

        // Mesh sample format
        _sampleFormat = pData[i++];

        _bitsScale = static_cast<float>(1 << _sampleFormat);

        // Reserved
        for (size_t j = 0; j < 12; j++)
        {
            if (pData[i++] != 0x00)
                return false;
        }

        // Checksum
        i += 2;

        // Validate checksum
        uint16_t checksum = 0;
        uint16_t* pData16 = (uint16_t*)pData;
        for (size_t checksumIndex = 0; checksumIndex < 16; checksumIndex++)
            checksum += pData16[checksumIndex];

        if (checksum != 0)
            return false;

        uint32_t* payload = (uint32_t*)(pData + i);

        size_t numBytes = spData->size();
        size_t payloadSize = numBytes - 32;
        size_t numPoints = payloadSize / sizeof(uint32_t);
        _spPointsArray = std::make_shared<PointsArray>(numPoints);
            
        uint32_t* pPoints = (uint32_t*)_spPointsArray->data();
        for (size_t payloadIndex = 0; payloadIndex < numPoints; payloadIndex++)
            *pPoints++ = *payload++;

        return true;
    }

    uint32_t OWarpFile::GetNumRows()
    {
        return _numRows;
    }

    uint32_t OWarpFile::GetNumColumns()
    {
        return _numColumns;
    }

    uint32_t OWarpFile::GetSampleSize()
    {
        return _sampleSize;
    }

    uint32_t OWarpFile::GetSampleFormat()
    {
        return _sampleFormat;
    }

    PointsArrayPtr OWarpFile::GetPointsArray()
    {
        return _spPointsArray;
    }
} // namespace SwApi
