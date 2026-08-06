/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "TiffImageReader.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <limits>

namespace {

enum class tiff_endian_t
{
    little,
    big
};

struct ifd_entry_t
{
    uint16_t tag = 0;
    uint16_t type = 0;
    uint32_t count = 0;
    uint32_t valueOffset = 0;
    uint32_t entryOffset = 0;
};

constexpr uint16_t TIFF_TAG_IMAGE_WIDTH = 256;
constexpr uint16_t TIFF_TAG_IMAGE_LENGTH = 257;
constexpr uint16_t TIFF_TAG_BITS_PER_SAMPLE = 258;
constexpr uint16_t TIFF_TAG_COMPRESSION = 259;
constexpr uint16_t TIFF_TAG_PHOTOMETRIC = 262;
constexpr uint16_t TIFF_TAG_STRIP_OFFSETS = 273;
constexpr uint16_t TIFF_TAG_SAMPLES_PER_PIXEL = 277;
constexpr uint16_t TIFF_TAG_ROWS_PER_STRIP = 278;
constexpr uint16_t TIFF_TAG_STRIP_BYTE_COUNTS = 279;
constexpr uint16_t TIFF_TAG_PLANAR_CONFIGURATION = 284;

constexpr uint16_t TIFF_TYPE_BYTE = 1;
constexpr uint16_t TIFF_TYPE_SHORT = 3;
constexpr uint16_t TIFF_TYPE_LONG = 4;

constexpr uint16_t TIFF_COMPRESSION_NONE = 1;
constexpr uint16_t TIFF_PHOTOMETRIC_WHITE_IS_ZERO = 0;
constexpr uint16_t TIFF_PHOTOMETRIC_BLACK_IS_ZERO = 1;
constexpr uint16_t TIFF_PHOTOMETRIC_RGB = 2;

bool AddWouldOverflow(size_t a, size_t b)
{
    return b > (std::numeric_limits<size_t>::max() - a);
}

bool MulWouldOverflow(size_t a, size_t b)
{
    if(a == 0 || b == 0)
        return false;

    return a > (std::numeric_limits<size_t>::max() / b);
}

uint16_t DecodeU16(const uint8_t* p, tiff_endian_t endian)
{
    if(endian == tiff_endian_t::little)
        return static_cast<uint16_t>(p[0]) | static_cast<uint16_t>(p[1] << 8);

    return static_cast<uint16_t>(p[0] << 8) | static_cast<uint16_t>(p[1]);
}

uint32_t DecodeU32(const uint8_t* p, tiff_endian_t endian)
{
    if(endian == tiff_endian_t::little)
    {
        return static_cast<uint32_t>(p[0]) |
               (static_cast<uint32_t>(p[1]) << 8) |
               (static_cast<uint32_t>(p[2]) << 16) |
               (static_cast<uint32_t>(p[3]) << 24);
    }

    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8) |
           static_cast<uint32_t>(p[3]);
}

bool ReadAt(std::ifstream& in, size_t fileSize, size_t offset, void* dst, size_t count)
{
    if(count == 0)
        return true;

    if(AddWouldOverflow(offset, count) || (offset + count) > fileSize)
        return false;

    in.seekg(static_cast<std::streamoff>(offset), std::ios::beg);
    if(!in)
        return false;

    in.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(count));
    return static_cast<size_t>(in.gcount()) == count;
}

bool ReadU16At(std::ifstream& in, size_t fileSize, size_t offset, tiff_endian_t endian, uint16_t& out)
{
    uint8_t buf[2];
    if(!ReadAt(in, fileSize, offset, buf, sizeof(buf)))
        return false;
    out = DecodeU16(buf, endian);
    return true;
}

bool ReadU32At(std::ifstream& in, size_t fileSize, size_t offset, tiff_endian_t endian, uint32_t& out)
{
    uint8_t buf[4];
    if(!ReadAt(in, fileSize, offset, buf, sizeof(buf)))
        return false;
    out = DecodeU32(buf, endian);
    return true;
}

size_t GetTypeSize(uint16_t type)
{
    switch(type)
    {
        case TIFF_TYPE_BYTE:
            return 1;
        case TIFF_TYPE_SHORT:
            return 2;
        case TIFF_TYPE_LONG:
            return 4;
        default:
            return 0;
    }
}

const ifd_entry_t* FindEntry(const std::vector<ifd_entry_t>& entries, uint16_t tag)
{
    const auto it = std::find_if(entries.begin(), entries.end(), [tag](const ifd_entry_t& e){ return e.tag == tag; });
    return (it == entries.end()) ? nullptr : &(*it);
}

bool GetEntryDataRange(size_t fileSize, const ifd_entry_t& entry, size_t& dataOffset, size_t& dataSize)
{
    const size_t typeSize = GetTypeSize(entry.type);
    if(typeSize == 0)
        return false;

    if(MulWouldOverflow(typeSize, static_cast<size_t>(entry.count)))
        return false;

    dataSize = typeSize * static_cast<size_t>(entry.count);
    if(dataSize == 0)
        return false;

    if(dataSize <= 4)
    {
        dataOffset = static_cast<size_t>(entry.entryOffset) + 8;
    }
    else
    {
        dataOffset = static_cast<size_t>(entry.valueOffset);
    }

    if(AddWouldOverflow(dataOffset, dataSize) || (dataOffset + dataSize) > fileSize)
        return false;

    return true;
}

bool ReadEntryValueU32(std::ifstream& in, size_t fileSize, const ifd_entry_t& entry, tiff_endian_t endian, size_t index, uint32_t& out)
{
    size_t dataOffset = 0;
    size_t dataSize = 0;
    if(!GetEntryDataRange(fileSize, entry, dataOffset, dataSize))
        return false;

    const size_t typeSize = GetTypeSize(entry.type);
    if(typeSize == 0 || index >= entry.count)
        return false;

    const size_t itemOffset = dataOffset + index * typeSize;
    switch(entry.type)
    {
        case TIFF_TYPE_BYTE:
        {
            uint8_t b = 0;
            if(!ReadAt(in, fileSize, itemOffset, &b, 1))
                return false;
            out = b;
            return true;
        }
        case TIFF_TYPE_SHORT:
        {
            uint16_t tmp = 0;
            if(!ReadU16At(in, fileSize, itemOffset, endian, tmp))
                return false;
            out = tmp;
            return true;
        }
        case TIFF_TYPE_LONG:
        {
            uint32_t tmp = 0;
            if(!ReadU32At(in, fileSize, itemOffset, endian, tmp))
                return false;
            out = tmp;
            return true;
        }
        default:
            return false;
    }
}

bool ReadEntryValuesU32(std::ifstream& in, size_t fileSize, const ifd_entry_t& entry, tiff_endian_t endian, std::vector<uint32_t>& out)
{
    out.clear();

    size_t dataOffset = 0;
    size_t dataSize = 0;
    if(!GetEntryDataRange(fileSize, entry, dataOffset, dataSize))
        return false;

    const size_t typeSize = GetTypeSize(entry.type);
    if(typeSize == 0)
        return false;

    // Read the entire value block once (typically a few bytes to a few KB), then decode in-memory.
    std::vector<uint8_t> buf(dataSize);
    if(!ReadAt(in, fileSize, dataOffset, buf.data(), dataSize))
        return false;

    out.resize(entry.count);
    for(size_t i = 0; i < entry.count; ++i)
    {
        const uint8_t* p = buf.data() + i * typeSize;
        switch(entry.type)
        {
            case TIFF_TYPE_BYTE:
                out[i] = *p;
                break;
            case TIFF_TYPE_SHORT:
                out[i] = DecodeU16(p, endian);
                break;
            case TIFF_TYPE_LONG:
                out[i] = DecodeU32(p, endian);
                break;
            default:
                return false;
        }
    }

    return true;
}

uint8_t NormalizeSampleToU8(uint32_t sample, uint32_t bitsPerSample)
{
    if(bitsPerSample == 8)
        return static_cast<uint8_t>(sample & 0xFFu);

    const uint32_t maxValue = ((1u << bitsPerSample) - 1u);

    if(maxValue == 0)
        return 0;

    sample = std::min(sample, maxValue);

    // Rounded normalization gives less bias than plain right-shift for 10/12-bit input.
    const uint32_t value = (sample * 255u + (maxValue / 2u)) / maxValue;
    return static_cast<uint8_t>(std::min(255u, value));
}

bool ParseIfd(std::ifstream& in, size_t fileSize, tiff_endian_t endian, uint32_t ifdOffset, std::vector<ifd_entry_t>& out, std::string& error)
{
    uint16_t entryCount = 0;
    if(!ReadU16At(in, fileSize, ifdOffset, endian, entryCount))
    {
        error = "invalid IFD entry count";
        return false;
    }

    const size_t entriesStart = static_cast<size_t>(ifdOffset) + 2;
    const size_t entriesBytes = static_cast<size_t>(entryCount) * 12;
    if(AddWouldOverflow(entriesStart, entriesBytes) || (entriesStart + entriesBytes) > fileSize)
    {
        error = "IFD table exceeds file bounds";
        return false;
    }

    // Read the entire entry table once (typically <1 KB) and decode in-memory.
    std::vector<uint8_t> table(entriesBytes);
    if(entriesBytes != 0 && !ReadAt(in, fileSize, entriesStart, table.data(), entriesBytes))
    {
        error = "failed to parse IFD entry";
        return false;
    }

    out.clear();
    out.reserve(entryCount);

    for(uint16_t i = 0; i < entryCount; ++i)
    {
        const size_t recOff = static_cast<size_t>(i) * 12;
        const uint8_t* rec = table.data() + recOff;
        const uint16_t tag = DecodeU16(rec, endian);
        const uint16_t type = DecodeU16(rec + 2, endian);
        const uint32_t count = DecodeU32(rec + 4, endian);
        const uint32_t valueOffset = DecodeU32(rec + 8, endian);
        const uint32_t entryFileOffset = static_cast<uint32_t>(entriesStart + recOff);

        out.push_back(ifd_entry_t{tag, type, count, valueOffset, entryFileOffset});
    }

    return true;
}

} // namespace

namespace SwApi
{

bool IsTiffFileExtension(const std::filesystem::path& path)
{
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return (ext == ".tif") || (ext == ".tiff");
}

VfrImage LoadBasicTiffImage8(const std::filesystem::path& path, std::string& error)
{
    VfrImage outImage{};

    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if(!in)
    {
        error = "failed to open file";
        return {};
    }

    const std::streamsize streamSize = in.tellg();
    if(streamSize <= 0)
    {
        error = "file is empty";
        return {};
    }

    const size_t fileSize = static_cast<size_t>(streamSize);

    if(fileSize < 8)
    {
        error = "file is too small for TIFF header";
        return {};
    }

    uint8_t header[8];
    if(!ReadAt(in, fileSize, 0, header, sizeof(header)))
    {
        error = "failed to read TIFF header";
        return {};
    }

    tiff_endian_t endian{};
    if(header[0] == 'I' && header[1] == 'I')
        endian = tiff_endian_t::little;
    else if(header[0] == 'M' && header[1] == 'M')
        endian = tiff_endian_t::big;
    else
    {
        error = "invalid TIFF byte order marker";
        return {};
    }

    const uint16_t magic = DecodeU16(header + 2, endian);
    if(magic != 42)
    {
        error = "invalid TIFF magic number";
        return {};
    }

    const uint32_t ifdOffset = DecodeU32(header + 4, endian);
    if(ifdOffset >= fileSize)
    {
        error = "invalid first IFD offset";
        return {};
    }

    std::vector<ifd_entry_t> entries;
    if(!ParseIfd(in, fileSize, endian, ifdOffset, entries, error))
        return {};

    const ifd_entry_t* widthEntry = FindEntry(entries, TIFF_TAG_IMAGE_WIDTH);
    const ifd_entry_t* heightEntry = FindEntry(entries, TIFF_TAG_IMAGE_LENGTH);
    const ifd_entry_t* bitsPerSampleEntry = FindEntry(entries, TIFF_TAG_BITS_PER_SAMPLE);
    const ifd_entry_t* compressionEntry = FindEntry(entries, TIFF_TAG_COMPRESSION);
    const ifd_entry_t* photometricEntry = FindEntry(entries, TIFF_TAG_PHOTOMETRIC);
    const ifd_entry_t* stripOffsetsEntry = FindEntry(entries, TIFF_TAG_STRIP_OFFSETS);
    const ifd_entry_t* samplesPerPixelEntry = FindEntry(entries, TIFF_TAG_SAMPLES_PER_PIXEL);
    const ifd_entry_t* stripByteCountsEntry = FindEntry(entries, TIFF_TAG_STRIP_BYTE_COUNTS);
    const ifd_entry_t* planarEntry = FindEntry(entries, TIFF_TAG_PLANAR_CONFIGURATION);

    if(!widthEntry || !heightEntry || !compressionEntry || !photometricEntry || !stripOffsetsEntry || !stripByteCountsEntry)
    {
        error = "missing required TIFF tags";
        return {};
    }

    uint32_t width = 0;
    uint32_t height = 0;
    if(!ReadEntryValueU32(in, fileSize, *widthEntry, endian, 0, width) ||
       !ReadEntryValueU32(in, fileSize, *heightEntry, endian, 0, height))
    {
        error = "invalid image dimensions";
        return {};
    }

    if(width == 0 || height == 0)
    {
        error = "invalid zero-sized image";
        return {};
    }

    uint32_t compression = TIFF_COMPRESSION_NONE;
    if(!ReadEntryValueU32(in, fileSize, *compressionEntry, endian, 0, compression) || compression != TIFF_COMPRESSION_NONE)
    {
        error = "unsupported TIFF compression (only uncompressed is supported)";
        return {};
    }

    uint32_t samplesPerPixel = 1;
    if(samplesPerPixelEntry && !ReadEntryValueU32(in, fileSize, *samplesPerPixelEntry, endian, 0, samplesPerPixel))
    {
        error = "invalid SamplesPerPixel tag";
        return {};
    }

    if(samplesPerPixel != 1 && samplesPerPixel != 3)
    {
        error = "unsupported SamplesPerPixel value (only grayscale or RGB are supported)";
        return {};
    }

    uint32_t planarConfig = 1;
    if(planarEntry && !ReadEntryValueU32(in, fileSize, *planarEntry, endian, 0, planarConfig))
    {
        error = "invalid PlanarConfiguration tag";
        return {};
    }

    if(planarConfig != 1)
    {
        error = "unsupported planar TIFF (only contiguous planar layout is supported)";
        return {};
    }

    uint32_t photometric = 0;
    if(!ReadEntryValueU32(in, fileSize, *photometricEntry, endian, 0, photometric))
    {
        error = "invalid PhotometricInterpretation tag";
        return {};
    }

    if(samplesPerPixel == 1)
    {
        if(photometric != TIFF_PHOTOMETRIC_BLACK_IS_ZERO && photometric != TIFF_PHOTOMETRIC_WHITE_IS_ZERO)
        {
            error = "unsupported grayscale photometric interpretation";
            return {};
        }
    }
    else
    {
        if(photometric != TIFF_PHOTOMETRIC_RGB)
        {
            error = "unsupported RGB photometric interpretation";
            return {};
        }
    }

    uint32_t bitsPerSample = 1;
    std::vector<uint32_t> bitsValues;
    if(bitsPerSampleEntry)
    {
        if(!ReadEntryValuesU32(in, fileSize, *bitsPerSampleEntry, endian, bitsValues) || bitsValues.empty())
        {
            error = "invalid BitsPerSample tag";
            return {};
        }

        if(bitsValues.size() == 1)
        {
            bitsPerSample = bitsValues[0];
        }
        else
        {
            if(bitsValues.size() != samplesPerPixel)
            {
                error = "unsupported BitsPerSample channel count";
                return {};
            }

            bitsPerSample = bitsValues[0];
            for(uint32_t b : bitsValues)
            {
                if(b != bitsPerSample)
                {
                    error = "mixed per-channel bit depth is not supported";
                    return {};
                }
            }
        }
    }

    if(bitsPerSample != 8 && bitsPerSample != 10 && bitsPerSample != 12 && bitsPerSample != 16)
    {
        error = "unsupported BitsPerSample (expected 8, 10, 12, or 16)";
        return {};
    }

    const size_t bytesPerSample = (bitsPerSample <= 8) ? 1u : 2u;
    const size_t channels = static_cast<size_t>(samplesPerPixel);

    if(MulWouldOverflow(static_cast<size_t>(width), static_cast<size_t>(height)) ||
       MulWouldOverflow(static_cast<size_t>(width) * static_cast<size_t>(height), channels))
    {
        error = "image dimensions overflow supported memory size";
        return {};
    }

    const size_t sampleCount = static_cast<size_t>(width) * static_cast<size_t>(height) * channels;

    std::vector<uint32_t> stripOffsets;
    std::vector<uint32_t> stripByteCounts;
    if(!ReadEntryValuesU32(in, fileSize, *stripOffsetsEntry, endian, stripOffsets) ||
       !ReadEntryValuesU32(in, fileSize, *stripByteCountsEntry, endian, stripByteCounts))
    {
        error = "invalid strip metadata";
        return {};
    }

    if(stripOffsets.empty() || stripOffsets.size() != stripByteCounts.size())
    {
        error = "invalid strip count metadata";
        return {};
    }

    outImage = VfrImage(static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(channels), 8u);
    uint8_t* const dst = outImage.Data();

    // Stream strip data straight into the output buffer. For 16-bit input we use a single
    // reusable temp buffer sized to the largest strip and decode in place.
    std::vector<uint8_t> tmp;
    if(bytesPerSample == 2)
    {
        size_t maxStripBytes = 0;
        for(uint32_t c : stripByteCounts)
        {
            if(c > maxStripBytes)
                maxStripBytes = c;
        }
        tmp.resize(maxStripBytes);
    }

    size_t dstSampleIndex = 0;
    for(size_t i = 0; i < stripOffsets.size(); ++i)
    {
        const size_t stripOffset = static_cast<size_t>(stripOffsets[i]);
        const size_t stripSize = static_cast<size_t>(stripByteCounts[i]);

        if(stripSize == 0)
            continue;

        if(AddWouldOverflow(stripOffset, stripSize) || (stripOffset + stripSize) > fileSize)
        {
            error = "strip data exceeds file bounds";
            return {};
        }

        if((stripSize % bytesPerSample) != 0)
        {
            error = "strip size is not a multiple of sample size";
            return {};
        }

        const size_t stripSamples = stripSize / bytesPerSample;
        if(AddWouldOverflow(dstSampleIndex, stripSamples) || (dstSampleIndex + stripSamples) > sampleCount)
        {
            error = "strip data exceeds image sample count";
            return {};
        }

        if(bytesPerSample == 1)
        {
            if(!ReadAt(in, fileSize, stripOffset, dst + dstSampleIndex, stripSize))
            {
                error = "failed to read strip data";
                return {};
            }
        }
        else
        {
            if(!ReadAt(in, fileSize, stripOffset, tmp.data(), stripSize))
            {
                error = "failed to read strip data";
                return {};
            }

            for(size_t s = 0; s < stripSamples; ++s)
            {
                const uint16_t sample = DecodeU16(tmp.data() + s * 2, endian);
                dst[dstSampleIndex + s] = NormalizeSampleToU8(sample, bitsPerSample);
            }
        }

        dstSampleIndex += stripSamples;
    }

    if(dstSampleIndex < sampleCount)
    {
        error = "insufficient uncompressed strip data";
        return {};
    }

    if(samplesPerPixel == 1 && photometric == TIFF_PHOTOMETRIC_WHITE_IS_ZERO)
    {
        uint8_t* p = outImage.Data();
        for(uint32_t i = 0; i < outImage.SizeBytes(); ++i)
            p[i] = static_cast<uint8_t>(255u - p[i]);
    }

    return outImage;
}

} // namespace SwApi
