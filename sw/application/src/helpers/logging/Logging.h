/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <iostream>
#include <cassert>
#include <csignal>
#include <stdint.h>


/// Main logging macros.
/// Usage:
///
///   TRACE << "Hello, this is a trace level message.\n";
///   TRACE << "This becomes a seperate message.\n";
///
///   auto& CounterMsg = INFO << "Counter: \n";
///   CounterMsg << "1\n";
///   CounterMsg << "2\n";
///   CounterMsg << "3\n";
///   CounterMsg << "These are all in one block!\n";

#define TRACE Logging::LogTraceImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
//#define TRACE Logging::LogNothingImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
#define INFO Logging::LogInfoImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
#define WARN Logging::LogWarnImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
#define ERR Logging::LogErrorImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)
#define FATAL Logging::LogFatalImpl(__FILE__, __FILE_NAME__, __LINE__, __func__)



/// `LOG_V` prints the value of the expression next to the 'text' of the expression.
/// e.g. std::cout << LOG_V(2) --> '2: 2'
///
///      int a = 2+2;
///      std::cout << LOG_V(a) --> 'a: 4'
///
///      int a = 2+2;
///      std::cout << LOG_V(a+2) --> 'a+2: 6'
#define LOG_V(expression) \
     #expression << ": " << (expression)

// Currently used to log video standards
#define LOG_IN(in) \
    "In: " << (in)

#define LOG_IO(in, out) \
    LOG_IN(in) << "\tOut: " << (out)

//region Colors
// Example usage:
//
//   printf("Hello " RED " there" RESET ", how are " BOLD " you " RESET "?");
//
//   TRACE
//     << "Startup " GREEN "OK!" RESET "\n"
//     << "Values were = " << LOG_V(counter->count) << '\n';

#define RESET       "\033[0m"
#define BLACK       "\033[30m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define WHITE       "\033[37m"
#define BOLD        "\033[1m"

//endregion

namespace Logging {

    std::ostream& LogTraceImpl(const char* file,
                               const char* basefile,
                               int line,
                               const char* func);

    std::ostream& LogInfoImpl(const char* file,
                              const char* basefile,
                              int line,
                              const char* func);

    std::ostream& LogWarnImpl(const char* file,
                              const char* basefile,
                              int line,
                              const char* func);

    std::ostream& LogErrorImpl(const char* file,
                               const char* basefile,
                               int line,
                               const char* func);

    std::ostream& LogFatalImpl(const char* file,
                               const char* basefile,
                               int line,
                               const char* func);

    std::ostream& LogNothingImpl(const char* file,
                               const char* basefile,
                               int line,
                               const char* func);                               

    void Assert(const char* what,
                bool val,
                const char* file,
                const char* basefile,
                int line,
                const char* func,
                const char* message = nullptr);

};

#define ASSERT(x, ...) \
    if (not (x)) {\
        Logging::Assert(#x, false, __FILE__, __BASE_FILE__, __LINE__, __func__, ##__VA_ARGS__);\
        assert(false);\
    }
