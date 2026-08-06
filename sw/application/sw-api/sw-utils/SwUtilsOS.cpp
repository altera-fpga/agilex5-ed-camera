/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "SwUtilsOS.h"
#include "SwUtilsStringUtils.h"
#include <chrono>
#include <thread>
#include <assert.h>
#include "SwUtilsFile.h"
#include <cstdarg>

#ifdef __linux__
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>
#define INVALID_HANDLE_VALUE (-1)
#else
#include <io.h>
#endif

namespace SwUtils
{
    namespace
    {
        class OSImpl : public OS
        {
        public:
            bool GetComputerName(char* str, uint32_t* length)
            {
#ifdef __linux__
                char buffer[513]{};
                ::gethostname(buffer, 512);

                std::string computer_name(buffer);

                if (computer_name.find("none") != std::string::npos)
                {
                    computer_name = "";
                    File hostname_file;
                    std::filesystem::path filename("/mnt/hostname");
                    if (hostname_file.Open(filename, FileOpenFlags::READ, false))
                    {
                        hostname_file.ReadLine(computer_name);
                    }

                    if (computer_name == "")
                    {
                        struct ifaddrs* if_address_struct = 0;
                        struct ifaddrs* ifa = 0;
                        void* temp_address = 0;

                        getifaddrs(&if_address_struct);

                        std::string ip_address_name;
                        for (ifa = if_address_struct; ifa != NULL; ifa = ifa->ifa_next)
                        {
                            if (ifa->ifa_addr->sa_family == AF_INET)
                            {
                                // is a valid IP4 Address
                                temp_address = &((sockaddr_in *)ifa->ifa_addr)->sin_addr;
                                char address_buffer[INET_ADDRSTRLEN];
                                inet_ntop(AF_INET, temp_address, address_buffer, INET_ADDRSTRLEN);
                                std::string str = FormatString("%s", address_buffer);
                                ip_address_name += str;
                                break;
                            }
                            else if (ifa->ifa_addr->sa_family == AF_INET6)
                            {
                                // is a valid IP6 Address
                                temp_address = &((sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
                                char address_buffer[INET6_ADDRSTRLEN];
                                inet_ntop(AF_INET6, temp_address, address_buffer, INET6_ADDRSTRLEN);
                                std::string str = FormatString("%s", address_buffer);
                                ip_address_name += str;
                            }
                        }

                        if (if_address_struct != 0)
                            freeifaddrs(if_address_struct);

                        computer_name = std::move(ip_address_name);
                    }
                }

                uint32_t output_buffer_length = *length;
                uint32_t required_length = (uint32_t)computer_name.length() + 1;
                if (output_buffer_length >= required_length)
                {
                    char* pComputerName = computer_name.data();
                    for (uint32_t i = 0; i < required_length; i++)
                        *str++ = *pComputerName++;
                    *str++ = 0;
                    *length = (required_length - 1);
                    return true;
                }
                else
                {
                    *length = required_length;
                    return false;
                }
#else
                DWORD theLength = 0;
                bool result = ::GetComputerNameA(str, &theLength);
                if (length)
                    *length = theLength;
                return result;
#endif
            }

            int GetCurrentThreadId() override
            {
#ifdef __linux__
                return syscall(SYS_gettid);
#else
                return ::GetCurrentThreadId();
#endif
            }

            void SetEvent(EventHandle event_handle) override
            {
#ifdef __linux__
                uint64_t value = 1;
                ssize_t r = ::write(event_handle, &value, sizeof(value));
                (void)r;
#else
                ::SetEvent(event_handle);
#endif
            }

            void ResetEvent(EventHandle event_handle) override
            {
#ifdef __linux__
                // Since the eventfd is non-blocking, if it is set
                // then read will clear the internal state to 0 (reset)
                // otherwise if already clear it will return EAGAIN
                uint64_t value = 0;
                ssize_t r = ::read(event_handle, &value, sizeof(value));
                (void)r;
#else
                ::ResetEvent(event_handle);
#endif
            }

            virtual time_t GetCTime()
            {
                time_t theTime{};
                ::time(&theTime);
                return theTime;
            }

            // This is a continuous timer, similar to uptime, and is
            // not affected by changing the date/time
            uint32_t GetTimeMS() override
            {
#ifdef __linux__
                static double offset = 0.0;
                static bool first_time = true;

                timespec time_value;
                clock_gettime(CLOCK_MONOTONIC, &time_value);

                double ms_now = time_value.tv_sec * 1000.0 + time_value.tv_nsec / 1000000.0;

                if (first_time)
                {
                    offset = ms_now;
                    first_time = false;
                }

                double diff = ms_now - offset;

                uint32_t ms_now_int = (uint32_t)diff;
                return ms_now_int;
#else
                return ::timeGetTime();
#endif
            }

            int WaitForMultipleObjects(int count, EventHandle* events,
                                    bool wait_all, uint64_t milliseconds) override
            {
            #ifdef WIN32
                std::vector<HANDLE> handles(count);
                for (int i = 0; i < count; i++)
                    handles[i] = (HANDLE)events[i];

                int r = ::WaitForMultipleObjects((uint32_t)count, (const HANDLE*)&handles[0], wait_all ? TRUE : FALSE,
                                                 (uint32_t)milliseconds);
                return r;
            #else
                int r = WaitForMultipleFileObjects(count, events, wait_all, (uint32_t)milliseconds);
                return r;
            #endif
            }

            int WaitForSingleObject(EventHandle event_handle, uint32_t timeout_ms, bool manual) override
            {
#ifdef __linux__
                struct pollfd fds;
                fds.fd = event_handle;
                fds.revents = 0;
                fds.events = POLLIN;

                while (true)
                {
                    int n_set = ::poll(&fds, 1, (timeout_ms == Event::INFINITE_TIMEOUT) ? -1 : timeout_ms);
                    if (n_set <= 0)
                    {
                        return WAIT_TIMEOUT;
                    }

                    if (manual)
                        return WAIT_OBJECT_0;
                    else
                    {
                        uint64_t value = 0;
                        ssize_t r = ::read(event_handle, &value, sizeof(value));
                        (void)r;
                        if (value != 0)
                            return WAIT_OBJECT_0;

                        // Go round again if someone else got it
                    }
                }
#else
                return ::WaitForSingleObject(event_handle, timeout_ms);
#endif
            }

            void Sleep(uint32_t milliseconds)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
            }
        };

    } // namespace

    std::shared_ptr<OS> OS::s_instance = std::make_shared<OSImpl>();

    std::shared_ptr<OS> OS::Get()
    {
        return s_instance;
    }

    void OS::SetInstance(std::shared_ptr<OS> instance)
    {
        s_instance = std::move(instance);
    }


#ifdef __linux__

    int o_close(int& fd)
    {
        if (fd == -1)
            return -1;

        int r = ::close(fd);
        fd = -1;
        return r;
    }

    int _vscprintf(const char* format, va_list argptr)
    {
        // Create a local copy of va_list
        // as it might be altered in vfwprintf
        std::va_list ap_local{};
        va_copy(ap_local, argptr);
        int ret = (vsnprintf(0, 0, format, ap_local));
        va_end(ap_local);

        return ret;
    }
#else
    int o_close(int& fd)
    {
        return ::_close(fd);
    }
#endif

    FDEventHandle::FDEventHandle(int file_descriptor, bool manual_reset)
        : _file_descriptor(file_descriptor)
        , _manual_reset(manual_reset)
    {
    }

    FDEventHandle::operator int() const
    {
        return _file_descriptor;
    }

    bool FDEventHandle::IsManual()
    {
        return _manual_reset;
    }

    #ifdef __linux__
    bool FDEventHandle::operator==(Event& other)
    {
        return (_file_descriptor == (int)(EventHandle)other);
    }

    // bool FDEventHandle::operator!=(Event& other)
    // {
    //     return !(*this == other);
    // }    

    // bool FDEventHandle::operator==(int other) const
    // {
    //     return (_file_descriptor == other);
    // }
    #endif

    /////////////////

    #ifdef __linux__
    Event::Event(bool manual, bool initial_state)
        : _close_on_exit(true)
        , _manual(manual)
    {
        _event_id = FDEventHandle(::eventfd(initial_state ? 1 : 0, EFD_NONBLOCK), manual);

        if ((int)_event_id < 0)
            assert(0);
    }

    Event::Event(const Event& assign_event)
        : _event_id(assign_event._event_id)
        , _close_on_exit(false)
        , _manual(assign_event._manual)
    {
    }

    Event::Event(EventHandle assign_event)
        : _event_id(assign_event)
        , _close_on_exit(false)
        , _manual(true) // Don't know '_manual' state of other event - use above constructor instead
    {
    }

    Event::~Event()
    {
        if (_close_on_exit)
            o_close(_event_id._file_descriptor);
    }

    bool Event::IsValid()
    {
        return ((int)_event_id >= 0);
    }

    Event& Event::operator= (const Event& other)
    {
        if (this != &other)
        {
            _event_id = other._event_id;
            _close_on_exit = false;
            _manual = other._manual;
        }
        return *this;
    }

    Event& Event::operator= (EventHandle value)
    {
        if (_event_id && (_event_id != value) && _close_on_exit)
            o_close(_event_id._file_descriptor);
        _event_id = value;
        _close_on_exit = false;
        return *this;
    }


    #else

    Event::Event(bool manual, bool initial_state)
        : _close_on_exit(true)
        , _manual(manual)
    {
        _event_id = ::CreateEvent(NULL,
                                manual ? TRUE : FALSE,
                                initial_state ? TRUE : FALSE,
                                "");
    }

    Event::Event(const Event& assign_event)
        : _event_id(assign_event._event_id)
        , _close_on_exit(false)
        , _manual(assign_event._manual)
    {
    }

    Event::Event(EventHandle assign_event)
        : _event_id(assign_event)
        , _close_on_exit(false)
        , _manual(true) // Don't know '_manual' state of other event - use above constructor instead
    {
    }

    Event::~Event()
    {
        if (_close_on_exit)
            ::CloseHandle(_event_id);
    }

    bool Event::IsValid()
    {
        return (_event_id != INVALID_HANDLE_VALUE);
    }

    Event& Event::operator= (EventHandle value)
    {
        if (_event_id && (_event_id != value) && _close_on_exit)
            ::CloseHandle(_event_id);
        _event_id = value;
        _close_on_exit = false;
        return *this;
    }

    Event& Event::operator= (const Event& other)
    {
        _event_id = other._event_id;
        _close_on_exit = false;
        _manual = other._manual;
        return *this;
    }

    #endif

    // Windows/Linux common
    void Event::Set()
    {
        SwUtils::OS::Get()->SetEvent(_event_id);
    }

    void Event::Reset()
    {
        SwUtils::OS::Get()->ResetEvent(_event_id);
    }

    bool Event::IsSignalled(uint32_t timeout)
    {
        return (OS::Get()->WaitForSingleObject(_event_id, timeout, _manual) == WAIT_OBJECT_0);
    }

    void Event::Wait()
    {
        IsSignalled(Event::INFINITE_TIMEOUT);
    }

    bool Event::Wait(int timeout)
    {
        return IsSignalled(timeout);
    }

    Event::operator EventHandle()
    {
        return _event_id;
    }

    ////////////////

    EventWaiter::EventWaiter(EventHandle event_handle)
    {
        if (event_handle != 0)
            _handles.push_back(event_handle);
    }

    void EventWaiter::AddHandle(EventHandle event_handle)
    {
        _handles.push_back(event_handle);
    }

    EventHandle EventWaiter::Wait(uint32_t timeout)
    {
        uint32_t r = OS::Get()->WaitForMultipleObjects((uint32_t)_handles.size(), &_handles[0], false, timeout);
        if (r == WAIT_TIMEOUT)
            return 0;

        uint32_t index = r - WAIT_OBJECT_0;
        if (index < (uint32_t)_handles.size())
            return _handles[index];

        return 0;
    }

    bool EventWaiter::WaitForAll(uint32_t timeout)
    {
    #ifdef __linux__
        // NB timeout is per handle in this __linux__ implementation
        for (size_t i = 0, n = _handles.size(); i < n; i++)
        {
            Event single_event(_handles[i]);
            if (!single_event.IsSignalled(timeout))
                return false;
        }
    #else
        uint32_t r = OS::Get()->WaitForMultipleObjects((uint32_t)_handles.size(), &_handles[0], false, timeout);
        if (r == WAIT_TIMEOUT)
            return false;
    #endif
        return true;
    }

    #ifdef WIN32
    bool PrivateWaitForMultipleFileObjects(int numHandles, FDEventHandle* pHandles,
                                        bool bAll, uint32_t timeout_ms, int& result)
    {
        fd_set receive_file_descriptors;
        struct timeval tv;
        int selectret;

        FD_ZERO(&receive_file_descriptors);
        SOCKET max_fd = 0;
        for (int i = 0; i < numHandles; i++)
        {
            if (pHandles[i] != -1)
            {
                SOCKET handle = pHandles[i];
                FD_SET(handle, &receive_file_descriptors);
                if (handle > max_fd)
                    max_fd = pHandles[i];
            }
        }

        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        bool infinite = (timeout_ms == Event::INFINITE_TIMEOUT);

        int nfds = (int)max_fd + 1; // Ignored
        selectret = ::select(nfds, &receive_file_descriptors, 0, 0, infinite ? 0 : &tv);

        if (selectret == SOCKET_ERROR)
        {
            // In windows you can only call select with socket fd's
            int error_code = WSAGetLastError();
            if (error_code == WSAENOTSOCK)
            {
                int here = 0;
            }
            result = -1;
            return true;
        }

        if (selectret == 0)
        {
            result = WAIT_TIMEOUT;
            return true;
        }

        for (int i = 0; i < numHandles; i++)
        {
            if (FD_ISSET(pHandles[i], &receive_file_descriptors))
            {
                uint64_t read_value = 0;

                if (!pHandles[i].IsManual())
                {
                    // In order to reset the eventfd ...
                    // Unfortunately this is not atomic with ::select
                    int n_read = _read((int)(SOCKET)pHandles[i], &read_value, sizeof(read_value));
                    if (n_read <= 0)
                    {
                        // Someone else got in before us, so we need to block again
                        return false;
                    }
                }

                result = i;
                return true;
            }
        }

        result = -3;
        return true;
    }
    #else
    // Return true if done
    bool PrivateWaitForMultipleFileObjects(int numHandles, FDEventHandle* pHandles,
                                        bool bAll, uint32_t timeout_ms, int& result)
    {
        int pollret;

        assert(numHandles <= 16);
        if (numHandles > 16)
            numHandles = 16;

        pollfd fds[16];

        for (int i = 0; i < numHandles; i++)
        {
            fds[i].fd = pHandles[i];
            fds[i].revents = 0;
            fds[i].events = POLLIN;
        }

        pollret = ::poll(fds, numHandles, (timeout_ms == Event::INFINITE_TIMEOUT) ? -1 : timeout_ms);

        if (pollret == SOCKET_ERROR)
        {
            result = -1;
            return true;
        }

        if (pollret == 0)
        {
            result = WAIT_TIMEOUT;
            return true;
        }

        for (int i = 0; i < numHandles; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                uint64_t read_value = 0;

                if (!pHandles[i].IsManual())
                {
                    // In order to reset the eventfd ...
                    // Unfortunately this is not atomic with ::select
                    int n_read = read(pHandles[i], &read_value, sizeof(read_value));
                    if (n_read <= 0)
                    {
                        // Someone else got in before us, so we need to block again
                        return false;
                    }
                }

                result = i;
                return true;
            }
        }
        result = -3;
        return true;
    }
    #endif

    int WaitForMultipleFileObjects(int numHandles, FDEventHandle* pHandles, bool wait_all, uint32_t timeout_ms)
    {
        while (true)
        {
            int result = 0;
            bool done = PrivateWaitForMultipleFileObjects(numHandles, pHandles, wait_all, timeout_ms, result);
            if (done)
                return result;
        }
    }

    void ShutdownSocket(SOCKET& socket_id)
    {
        if (socket_id == INVALID_SOCKET)
            return;

        shutdown(socket_id, SHUTDOWN_SOCKET_BOTHWAYS);

    #ifdef WIN32
        closesocket(socket_id);
    #else
        o_close(socket_id);
    #endif

        socket_id = INVALID_SOCKET;
    }

    std::string GetExecPath()
    {
        std::string path(".");
        path += dirSeparatorStr;

        char buffer[MAX_PATH];
    #ifdef __linux__
        int r = readlink("/proc/self/exe", buffer, MAX_PATH);
    #else
        DWORD r = GetModuleFileName(NULL, buffer, MAX_PATH);
    #endif

        if (r > 0)
        {
            buffer[r % MAX_PATH] = '\0';
            std::filesystem::path exe_path(buffer);
            path = exe_path.parent_path();
            path += dirSeparatorStr;
        }

        return path;
    }
} // namespace SwUtils
