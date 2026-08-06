/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include <vector>
#include <string>
#include <filesystem>

class SettingsFileFuzzer
{
public:
    SettingsFileFuzzer(int nParameters, char* parameters[]);
    int Run();

private:
    std::vector<std::string> _parameters;
    std::filesystem::path _fuzzFilename;
    uint32_t _fuzzPercent = 2;
    uint32_t _iteration = 0;
};