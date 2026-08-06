/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#include "PgmImageReader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>

namespace {

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

bool SkipWhitespaceAndComments(std::ifstream& in,
                               const size_t fileSize,
                               const size_t pixelDataSize = 0)
{
    while(true)
    {
        const std::streampos pos = in.tellg();
        if(pos < 0)
            return false;

        const size_t offset = static_cast<size_t>(pos);
        if(offset >= fileSize)
            return false;

        const int next = in.peek();
        if(next == std::char_traits<char>::eof())
            return false;

        const uint8_t c = static_cast<uint8_t>(next);

        if(c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
            // If only the expected pixel payload remains, do not consume further bytes.
            if(pixelDataSize != 0 && (fileSize - offset) <= pixelDataSize)
            {
                return true;
            }

            in.get();
            continue;
        }

        if(c == '#')
        {
            in.get();
            while(true)
            {
                const int ch = in.get();
                if(ch == std::char_traits<char>::eof())
                    return false;
                if(ch == '\n')
                    break;
            }
            continue;
        }

        return true;
    }

    return false;
}

bool ReadDecimalNumber(std::ifstream& in, const size_t fileSize, uint32_t& result)
{
    if(!SkipWhitespaceAndComments(in, fileSize))
        return false;

    result = 0;
    bool foundDigit = false;

    while(true)
    {
        const int next = in.peek();
        if(next == std::char_traits<char>::eof())
            break;

        if(!std::isdigit(static_cast<unsigned char>(next)))
            break;

        const uint8_t digit = static_cast<uint8_t>(next - '0');

        if(result > (std::numeric_limits<uint32_t>::max() / 10u))
            return false;

        in.get();
        result = result * 10u + digit;
        foundDigit = true;
    }

    return foundDigit;
}

/**
 * @brief Normalize 8-bit or 16-bit PGM sample values to 8-bit range
 * @param sample Sample value from file
 * @param maxval Maximum value (e.g., 255 or 65535)
 * @return Normalized 8-bit value
 */
uint8_t NormalizeSampleToU8(uint32_t sample, uint32_t maxval)
{
    if(maxval == 0)
        return 0;

    if(maxval == 255)
        return static_cast<uint8_t>(sample & 0xFFu);

    sample = std::min(sample, maxval);

    // Rounded normalization: (sample * 255 + maxval/2) / maxval
    const uint32_t value = (sample * 255u + (maxval / 2u)) / maxval;
    return static_cast<uint8_t>(value > 255u ? 255u : value);
}

#if 0
bool LoadPgmImage8Streamed(const std::filesystem::path& path, SwApi::VfrImage& outImage, std::string& error)
{
    outImage = {};

    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if(!in)
    {
        error = "failed to open file";
        return false;
    }

    const std::streamsize streamSize = in.tellg();
    if(streamSize <= 0)
    {
        error = "file is empty";
        return false;
    }

    const size_t fileSize = static_cast<size_t>(streamSize);
    in.seekg(0, std::ios::beg);

    char magic0 = 0;
    char magic1 = 0;
    if(!in.get(magic0) || !in.get(magic1))
    {
        error = "file is too small for PGM header";
        return false;
    }

    if(magic0 != 'P' || magic1 != '5')
    {
        error = "invalid PGM magic number (expected P5 binary format)";
        return false;
    }

    uint32_t width = 0;
    if(!ReadDecimalNumber(in, fileSize, width) || width == 0)
    {
        error = "invalid or missing image width";
        return false;
    }

    uint32_t height = 0;
    if(!ReadDecimalNumber(in, fileSize, height) || height == 0)
    {
        error = "invalid or missing image height";
        return false;
    }

    uint32_t maxval = 0;
    if(!ReadDecimalNumber(in, fileSize, maxval))
    {
        error = "invalid or missing maxval";
        return false;
    }

    if(maxval == 0 || maxval > 65535u)
    {
        error = "unsupported maxval (must be 1 to 65535)";
        return false;
    }

    if(width > 3840u || height > 2160u)
    {
        error = "image dimensions exceed maximum (3840x2160)";
        return false;
    }

    if(width < 32u || height < 32u)
    {
        error = "image dimensions below minimum (32x32)";
        return false;
    }

    if((width % 2u) != 0u || (height % 2u) != 0u)
    {
        error = "image width and height must be even (required for Bayer pattern)";
        return false;
    }

    const size_t bytesPerSample = (maxval <= 255u) ? 1u : 2u;
    const size_t widthSize = static_cast<size_t>(width);
    const size_t heightSize = static_cast<size_t>(height);

    if(MulWouldOverflow(widthSize, heightSize))
    {
        error = "pixel count overflow";
        return false;
    }

    const size_t pixelCount = widthSize * heightSize;
    if(MulWouldOverflow(pixelCount, bytesPerSample))
    {
        error = "pixel count overflow";
        return false;
    }

    const size_t requiredPixelBytes = pixelCount * bytesPerSample;

    if(!SkipWhitespaceAndComments(in, fileSize, requiredPixelBytes))
    {
        error = "unexpected EOF before pixel data";
        return false;
    }

    const std::streampos dataPos = in.tellg();
    if(dataPos < 0)
    {
        error = "failed to locate pixel data";
        return false;
    }

    const size_t dataOffset = static_cast<size_t>(dataPos);
    if(AddWouldOverflow(dataOffset, requiredPixelBytes) || (dataOffset + requiredPixelBytes) > fileSize)
    {
        error = "insufficient pixel data in file";
        return false;
    }

    outImage = SwApi::VfrImage{width, height, 1u, 8u};
    uint8_t* dst = outImage.Data();

    if(bytesPerSample == 1u)
    {
        if(maxval == 255u)
        {
            if(!in.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(pixelCount)))
            {
                error = "failed to read pixel data";
                return false;
            }
            return true;
        }

        std::vector<uint8_t> readBuffer(4096u);
        size_t written = 0;
        while(written < pixelCount)
        {
            const size_t remaining = pixelCount - written;
            const size_t chunkSize = std::min(readBuffer.size(), remaining);
            if(!in.read(reinterpret_cast<char*>(readBuffer.data()), static_cast<std::streamsize>(chunkSize)))
            {
                error = "failed to read pixel data";
                return false;
            }

            for(size_t i = 0; i < chunkSize; ++i)
                dst[written + i] = NormalizeSampleToU8(readBuffer[i], maxval);

            written += chunkSize;
        }

        return true;
    }

    std::vector<uint8_t> readBuffer(4096u);
    if((readBuffer.size() % 2u) != 0u)
        readBuffer.pop_back();

    size_t written = 0;
    while(written < pixelCount)
    {
        const size_t remainingPixels = pixelCount - written;
        const size_t maxPixelsPerChunk = readBuffer.size() / 2u;
        const size_t chunkPixels = std::min(maxPixelsPerChunk, remainingPixels);
        const size_t chunkBytes = chunkPixels * 2u;

        if(!in.read(reinterpret_cast<char*>(readBuffer.data()), static_cast<std::streamsize>(chunkBytes)))
        {
            error = "failed to read pixel data";
            return false;
        }

        for(size_t i = 0; i < chunkPixels; ++i)
        {
            const size_t idx = i * 2u;
            const uint16_t sample = (static_cast<uint16_t>(readBuffer[idx]) << 8) |
                                    static_cast<uint16_t>(readBuffer[idx + 1u]);
            dst[written + i] = NormalizeSampleToU8(sample, maxval);
        }

        written += chunkPixels;
    }

    return true;
}
#endif /* 0 */

} // namespace

namespace SwApi
{

bool IsPgmFileExtension(const std::filesystem::path& path)
{
    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return (ext == ".pgm");
}

// bool LoadBasicPgmImage8(const std::filesystem::path& path, VfrImage& outImage, std::string& error)
// {
//     return LoadPgmImage8Streamed(path, outImage, error);
// }


VfrImage LoadPgmImage(const std::filesystem::path& path, std::string& error)
{
    VfrImage outImage = {};

    std::ifstream in(path, std::ios::binary | std::ios::ate);

    if(!in)
    {
        error = "failed to open file";
        return outImage;
    }

    const std::streamsize streamSize = in.tellg();

    if(streamSize <= 0)
    {
        error = "file is empty";
        return outImage;
    }

    const size_t fileSize = static_cast<size_t>(streamSize);
    in.seekg(0, std::ios::beg);

    char magic0 = 0;
    char magic1 = 0;

    if(!in.get(magic0) || !in.get(magic1))
    {
        error = "file is too small for PGM header";
        return outImage;
    }

    if(magic0 != 'P' || magic1 != '5')
    {
        error = "invalid PGM magic number (expected P5 binary format)";
        return outImage;
    }

    uint32_t width = 0;
    if(!ReadDecimalNumber(in, fileSize, width) || width == 0)
    {
        error = "invalid or missing image width";
        return outImage;
    }

    uint32_t height = 0;
    if(!ReadDecimalNumber(in, fileSize, height) || height == 0)
    {
        error = "invalid or missing image height";
        return outImage;
    }

    uint32_t maxval = 0;
    if(!ReadDecimalNumber(in, fileSize, maxval))
    {
        error = "invalid or missing maxval";
        return outImage;
    }

    if(maxval == 0 || maxval > 65535u)
    {
        error = "unsupported maxval (must be 1 to 65535)";
        return outImage;
    }    

    const size_t bytesPerSample = (maxval <= 255u) ? 1u : 2u;
    const size_t widthSize = static_cast<size_t>(width);
    const size_t heightSize = static_cast<size_t>(height);

    if(MulWouldOverflow(widthSize, heightSize))
    {
        error = "pixel count overflow";
        return outImage;
    }

    const size_t pixelCount = widthSize * heightSize;

    if(MulWouldOverflow(pixelCount, bytesPerSample))
    {
        error = "pixel count overflow";
        return outImage;
    }

    const size_t requiredPixelBytes = pixelCount * bytesPerSample;

    if(!SkipWhitespaceAndComments(in, fileSize, requiredPixelBytes))
    {
        error = "unexpected EOF before pixel data";
        return outImage;
    }

    const std::streampos dataPos = in.tellg();

    if(dataPos < 0)
    {
        error = "failed to locate pixel data";
        return outImage;
    }

    const size_t dataOffset = static_cast<size_t>(dataPos);

    if(AddWouldOverflow(dataOffset, requiredPixelBytes) || (dataOffset + requiredPixelBytes) > fileSize)
    {
        error = "insufficient pixel data in file";
        return outImage;
    }


    // Note maxval previously checked to be > 0
    const uint32_t bps = 32u - __builtin_clz(maxval);

    outImage = VfrImage{width, height, 1u, bps};
    uint8_t* dst = outImage.Data();

    in.seekg(dataPos, std::ios::beg);

    if(!in.read(reinterpret_cast<char*>(dst), static_cast<std::streamsize>(pixelCount * bytesPerSample)))
    {
        error = "failed to read pixel data";
        outImage = VfrImage{};
    }

    return outImage;
}

} // namespace SwApi
