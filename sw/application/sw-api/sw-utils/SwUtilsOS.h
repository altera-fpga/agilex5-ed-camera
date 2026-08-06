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
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <string>
#include <memory>
#include <chrono>

#ifdef __linux__
#include <sys/types.h>
#include <sys/socket.h>
#endif

#ifdef WIN32
#include <WinSock2.h>
#endif

namespace SwUtils
{

    class FDEventHandle;
    int o_close(int& fd);

#ifdef __linux__
    using EventHandle = FDEventHandle;
    int _vscprintf(const char* format, va_list argptr);
    using ThreadIdType = pid_t;
    using SOCKET = int;
    static const uint32_t WAIT_TIMEOUT = 258; //ETIMEDOUT
    static const int WAIT_OBJECT_0 = 0;
    static const int SOCKET_ERROR = -1;
    static const int INVALID_SOCKET = -1;
    static const int NI_MAXHOST = 1025;
    static const int NI_MAXSERV = 32;
    static const int MAX_PATH = 512;
    #define SHUTDOWN_SOCKET_BOTHWAYS SHUT_RDWR
    #define SOCKET_IOCTL ioctl
    #define STREAM_FILENO fileno
    #define DUP2 dup2
    #define EXECVP execvp
    #define SNPRINTF snprintf
    #define COPYSIGN copysign
    #define SOCKLEN_TYPE socklen_t
    #define INFINITE 0xffffffff
    #define TRUE 1
    #define FALSE 0
    static constexpr char dirSeparatorChar = '/';
    static constexpr std::string dirSeparatorStr = "/";
#else
    using EventHandle = HANDLE;
    using ThreadIdType = uint32_t ;
    #define SHUTDOWN_SOCKET_BOTHWAYS SD_BOTH
    #define SOCKET_IOCTL ioctlsocket
    #define STREAM_FILENO _fileno
    #define DUP2 _dup2
    #define EXECVP _execvp
    #define SNPRINTF _snprintf
    #define COPYSIGN _copysign
    #define Assert ATLASSERT
    #define O_NONBLOCK 04000
    #define	SIGINT		2	/* Interrupt (ANSI).  */
    #define SIGKILL		9
    static constexpr char dirSeparatorChar = '\\';
    static constexpr std::string dirSeparatorStr = "\\";
#endif

    class OS
    {
    public:
        static std::shared_ptr<OS> Get();
        static void SetInstance(std::shared_ptr<OS> instance); // For use in testing only

        virtual bool GetComputerName(char* str, uint32_t* length) = 0;
        virtual int GetCurrentThreadId() = 0;
        virtual void SetEvent(EventHandle event_handle) = 0;
        virtual void ResetEvent(EventHandle event_handle) = 0;
        virtual int WaitForMultipleObjects(int count, EventHandle* events, bool wait_all, uint64_t milliseconds) = 0;
        virtual int WaitForSingleObject(EventHandle event_handle, uint32_t timeout_ms,
                                        bool manual = true) = 0; // Not part of usual signature, needed for calls from Event class
        virtual time_t   GetCTime() = 0; 
        virtual uint32_t GetTimeMS() = 0;
        virtual void Sleep(uint32_t milliseconds) = 0;

    private:
        static std::shared_ptr<OS> s_instance;
    };

    class Event;
    class FDEventHandle
    {
    public:
        FDEventHandle(int file_descriptor = -1, bool manual_reset = true);
        operator int() const;
        bool IsManual();
#ifdef __linux__
        bool operator==(Event& other);
        //bool operator!=(Event& other);
        //bool operator==(int other) const;
#endif
        int		_file_descriptor;
        bool	_manual_reset;
    };

    class Event
    {
    public:
        static const uint32_t INFINITE_TIMEOUT = (uint32_t)-1;
        Event(bool manual = true, bool initial_state = false);
        Event(const Event& assign_event);
        Event(EventHandle assign_event);
        ~Event();
        bool IsValid();
        Event& operator= (EventHandle value);
        Event& operator=(const Event& other);
        bool IsSignalled(uint32_t timeout = 0);
        bool IsSignalled(std::chrono::milliseconds timeout) { return IsSignalled(static_cast<uint32_t>(timeout.count())); }
        void Wait();
        bool Wait(int timeout);
        void Set();
        void Reset();
        operator EventHandle();

    private:
        EventHandle	_event_id;
        bool		_close_on_exit;
        bool		_manual;
    };

    class EventWaiter
    {
    public:
        EventWaiter(EventHandle event_handle = 0);
        void AddHandle(EventHandle event_handle);
        EventHandle Wait(uint32_t timeout = Event::INFINITE_TIMEOUT);
        bool WaitForAll(uint32_t timeout = Event::INFINITE_TIMEOUT);

    private:
        std::vector<EventHandle> _handles;
    };

    int WaitForMultipleFileObjects(int numHandles, FDEventHandle* pHandles, bool wait_all, uint32_t timeout_ms);
    void ShutdownSocket(SOCKET& socket);
    std::string GetExecPath();

} // namespace SwUtils
