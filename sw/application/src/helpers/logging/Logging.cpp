/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "Logging.h"

namespace Logging {

static bool addNewLines = true;

static uint32_t logc = 0;

std::ostream& LogTraceImpl(const char* file, const char* basefile, int line, const char* func)
{
    std::cout << "[(" << logc++ << ") " << basefile << ':' << line << ">" << func
              << " " WHITE "Trace" RESET "]: ";
    if (addNewLines) {
        return std::cout << "\n\t";
    } else {
        return std::cout;
    }
}

std::ostream& LogInfoImpl(const char* file, const char* basefile, int line, const char* func)
{
    std::cout << "[(" << logc++ << ") " << basefile << ':' << line << ">" << func
              << " " BOLD WHITE "Info" RESET "]: ";
    if (addNewLines) {
        return std::cout << "\n\t";
    } else {
        return std::cout;
    }
}

std::ostream& LogWarnImpl(const char* file, const char* basefile, int line, const char* func)
{
    std::cerr << "[(" << logc++ << ") " << basefile << ':' << line << ">" << func
              << " " BOLD YELLOW "Warn" RESET "]: ";
    if (addNewLines) {
        return std::cerr << "\n\t";
    } else {
        return std::cerr;
    }
}

std::ostream& LogErrorImpl(const char* file, const char* basefile, int line, const char* func)
{
    std::cerr << "[(" << logc++ << ") " << basefile << ':' << line << ">" << func
              << " " RED "Error" RESET "]: ";
    if (addNewLines) {
        return std::cerr << "\n\t";
    } else {
        return std::cerr;
    }
}

std::ostream& LogFatalImpl(const char* file, const char* basefile, int line, const char* func)
{
    std::cerr << "[(" << logc++ << ") " << basefile << ':' << line << ">" << func
              << " " BOLD RED "Fatal" RESET "]: ";
    if (addNewLines) {
        return std::cerr << "\n\t";
    } else {
        return std::cerr;
    }
}

std::ostream& LogNothingImpl(const char* file, const char* basefile, int line, const char* func)
{
    (void)file;
    (void)basefile;
    (void)line;
    (void)func;

    class null_buffer_t : public std::streambuf
    {
        public:
        int overflow(int c) { return c; }
    };    

    null_buffer_t null_buffer;
    static std::ostream null_stream(&null_buffer);
    return null_stream;
}

void Assert(const char* what,
            bool val,
            const char* file,
            const char* basefile,
            int line,
            const char* func,
            const char* message)
{
    if (val) {
        // It's good!
        return;
    }

    std::cerr << '\n'
              << "[" << basefile << ':' << line << ">" << func
              << " " BOLD RED "Assertion" RESET "]: Assertion failed:\n\n"
              << "\t\t [[ " << what << " ]]\n\n";
    if (message != nullptr) {
        std::cerr << message << '\n';
    }
    std::cerr << "\t... sending SIGABRT, attach a debugger to find out what caused this.\n";

    // NOTE The macro calling this function will call assert, so that the bottom
    // of the stack trace is where the error took place, not here.
}

} // namespace Logging
