/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SettingsFileFuzzer.h"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <random>

int main(int nParameters, char* parameters[])
{
    SettingsFileFuzzer fuzzer(nParameters, parameters);
    return fuzzer.Run();
}

SettingsFileFuzzer::SettingsFileFuzzer(int nParameters, char* parameters[])
{
    for (int i = 0; i < nParameters; i++)
        _parameters.push_back(parameters[i]);
}

int SettingsFileFuzzer::Run()
{
    if (_parameters.size() < 4)
        return 1;

#if 0
    std::cout << "Program: " << _parameters[0] << "\n";
    std::cout << "File to fuzz: " << _parameters[1] << "\n";
    std::cout << "Fuzz percentage: " << _parameters[2] << "\n";
    std::cout << "Iteration: " << _parameters[3] << "\n\n";
#endif
    
    _fuzzFilename = _parameters[1];
    _fuzzPercent = std::stoul(_parameters[2]);
    _iteration = std::stoul(_parameters[3]);

    std::error_code errorCode{};
    uintmax_t fileSize = std::filesystem::file_size(_fuzzFilename, errorCode);
    if (fileSize == 0)
        return 1;

    std::vector<uint8_t> fileData(fileSize);

    {
        std::ifstream fuzzFile(_fuzzFilename, std::fstream::binary);
        fuzzFile.read((char*)fileData.data(), fileSize);

        uintmax_t nErrors = (fileSize * _fuzzPercent) / 100;

        std::random_device randomDevice;
        std::mt19937 randomNumberGenerator(randomDevice());

        for (uintmax_t i = 0; i < nErrors; i++)
        {
            int index = randomNumberGenerator() % fileSize;
            if ((uintmax_t)index < fileSize)
            {
                uint8_t randomByte = randomNumberGenerator() % 255;
                fileData[index] = randomByte;
            }
        }
    }

    // Write out the modified file
    std::ofstream outputFile(_fuzzFilename, std::fstream::binary);
    outputFile.write((char*)fileData.data(), fileData.size());

    return 0;
}
