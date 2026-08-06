/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "VvpIspDemo.h"
#include <stdexcept>
#include <iostream>

std::string _bin_directory;

int main(int argumentCount, char* argumentArray[])
{
    std::string appliction_filename(argumentArray[0]);
    _bin_directory = appliction_filename.substr(0, appliction_filename.find_last_of("/"));

    VvpIspDemo VvpIspDemo(argumentCount, argumentArray);

    try{
        return VvpIspDemo.Run();
    }
    catch(std::runtime_error& runtime_error)
    {
        std::cerr << "Run-time error occurred in VvpIspDemo::Run()." << std::endl << runtime_error.what() << std::endl;
        return -1;
    }
    catch(...)
    {
        std::cerr << "Run-time error occurred in VvpIspDemo::Run()." << std::endl;
        return -1;
    }
}
