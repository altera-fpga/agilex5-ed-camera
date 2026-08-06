/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <vector>
#include <filesystem>
#include <cstdint>

namespace SwApi
{
    enum class OwfMeshType
    {
        FORWARD,
        INVERSE
    };

    enum class OwfResolution
    {
        FIXED,
        INDEPENDENT
    };

    using PointsArray = std::vector<uint32_t>;
    using PointsArrayPtr = std::shared_ptr<PointsArray>;

    class OWarpFile
    {
    public:
        OWarpFile();
        OWarpFile(OwfMeshType meshType,
                    OwfResolution resolution,
                    uint8_t horizontalAspect,
                    uint8_t vericalAspect,
                    uint16_t numRows,
                    uint16_t numColumns,
                    uint8_t sampleSize,
                    uint8_t sampleFormat,
                    const PointsArrayPtr& spPointsArray);

        uint32_t ToUint32(float value);
        float FromUint32(uint32_t);

        bool Save(std::filesystem::path filePath);
        bool Load(std::filesystem::path filePath);

        uint32_t GetNumRows();
        uint32_t GetNumColumns();
        uint32_t GetSampleSize();
        uint32_t GetSampleFormat();
        PointsArrayPtr GetPointsArray();            

    private:
        OwfMeshType _meshType = OwfMeshType::FORWARD;
        OwfResolution _resolution = OwfResolution::INDEPENDENT;
        uint8_t _horizontalAspect = 16;
        uint8_t _verticalAspect = 9;
        uint16_t _numRows = 0;
        uint16_t _numColumns = 0;
        uint8_t _sampleSize = 0;
        uint8_t _sampleFormat = 0;
        PointsArrayPtr _spPointsArray;
        float _bitsScale = 1.0f;
    };
} // namespace SwApi