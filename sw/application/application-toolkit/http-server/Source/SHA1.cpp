/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SHA1.h"
#include <bitset>

SHA1::SHA1()
:   _logicalFunctions(4)
,   _constants{0x5a827999, 0x6ed9eba1, 0x8f1bbcdc, 0xca62c1d6}
{
    _logicalFunctions[0] = [](uint32_t B, uint32_t C, uint32_t D)
    {
        return (B & C) | (~B & D);
    };

    _logicalFunctions[1] = [](uint32_t B, uint32_t C, uint32_t D)
    {
        return (B ^ C ^ D);
    };

    _logicalFunctions[2] = [](uint32_t B, uint32_t C, uint32_t D)
    {
        return (B & C) | (B & D) | (C & D);
    };

    _logicalFunctions[3] = [](uint32_t B, uint32_t C, uint32_t D)
    {
        return (B ^ C ^ D);
    };
}

SHA1Hash SHA1::CalculateHash(const std::string& inputValue)
{
    SHA1Hash hash;
    uint32_t hashValues[5] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };
    uint32_t words[80];

    size_t inputLength = inputValue.length();
    size_t messageLength = inputValue.length() * 8;
    size_t nBits = (inputLength * 8) + 1 + 64;
    if (nBits > 1024)
        return hash;

    std::bitset<1024> paddedMessage;

    size_t nChars = inputLength;
    for (size_t i = 0; i < nChars; i++)
    {
        uint8_t byteValue = (uint8_t)inputValue[i];
        std::bitset<8> valueBits{byteValue};

        size_t bitIndex = (nChars - i - 1) * 8;
        for (size_t j = 0; j < 8; j++)
            paddedMessage[bitIndex++] = valueBits[j];
    }

    // Append 1
    paddedMessage <<= 1;
    paddedMessage[0] = true;

    // Append 0's
    size_t nBlocks = nBits / 512;
    if ((nBits % 512) > 0)
        nBlocks++;

    size_t shift = (nBlocks * 512) - nBits;
    paddedMessage <<= shift;

    // Use 32 bit message length
    paddedMessage <<= 32;
    std::bitset<32> lengthBits{(uint32_t)messageLength};
    paddedMessage <<= 32;
    for (size_t i = 0; i < 32; i++)
        paddedMessage[i] = lengthBits[i];

    for (size_t b = 0; b < nBlocks; b++)
    {
        std::bitset<512> blockBits;
        size_t messageIndex = 512 * ((nBlocks - 1) - b);
        for (size_t i = 0; i < 512; i++)
            blockBits[i] = paddedMessage[messageIndex++];

        // Convert to 16 32-bit words
        for (size_t i = 0; i < 16; i++)
        {
            size_t blockStartIndex = ((15 - i) * 32);
            std::bitset<32> wordBits;
            for (size_t j = 0; j < 32; j++)
                wordBits[j] = blockBits[blockStartIndex + j];

            words[i] = wordBits.to_ulong();
        }

        for (size_t i = 16; i < 80; i++)
            words[i] = RotateLeft(words[i - 3] ^ words[i - 8] ^ words[i - 14] ^ words[i - 16], 1);

        uint32_t A = hashValues[0];
        uint32_t B = hashValues[1];
        uint32_t C = hashValues[2];
        uint32_t D = hashValues[3];
        uint32_t E = hashValues[4];

        for (size_t i = 0; i < 80; i++)
        {
            size_t functionIndex = i / 20;
            uint32_t temp = RotateLeft(A, 5) +
                            _logicalFunctions[functionIndex](B, C, D) +
                            E +
                            words[i] +
                            _constants[functionIndex];
            E = D;
            D = C;
            C = RotateLeft(B, 30);
            B = A;
            A = temp;
        }

        hashValues[0] += A;
        hashValues[1] += B;
        hashValues[2] += C;
        hashValues[3] += D;
        hashValues[4] += E;
    }

    size_t j = 0;
    for (size_t i = 0; i < 5; i++)
    {
        hash._data[j++] = ((hashValues[i] & 0xff000000) >> 24);
        hash._data[j++] = ((hashValues[i] & 0x00ff0000) >> 16);
        hash._data[j++] = ((hashValues[i] & 0x0000ff00) >>  8);
        hash._data[j++] = ((hashValues[i] & 0x000000ff) >>  0);
    }

    return hash;
}

