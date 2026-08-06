/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "SwUtils.h"
#include <typeinfo>
#include <list>
#include <mutex>

#include "SwUtilsReactorFactory.h"

namespace SwUtils
{
    class MessageQueue;
    using CommandQueueCB = std::function<void()>;
    using AutoRepeatCommandQueueCB = std::function<bool(uint32_t& counter)>;
    using VariableRepeatCommandQueueCB = std::function<int32_t()>;

    class Message
    {
    public:
        Message(uint32_t id)
        :    _doneEvent(0)
        ,    _pQueue(nullptr)
        ,    _exec_time(0)
        ,    _caller_thread_id(0)
        ,    _id(id)
        {
        }

        Message(const Message& other) = delete;
        Message &operator=(const Message& other) = delete;

        virtual ~Message() {}

        virtual void Execute() = 0;
        virtual void PreExecute()
        {
        }

        virtual void PostExecute()
        {
        }

        void Init(MessageQueue* pQueue,
                  ThreadIdType thread_id, uint32_t exec_time, EventHandle doneEvent)
        {
            _pQueue = pQueue;
            _caller_thread_id = thread_id;
            _exec_time = exec_time;
            _doneEvent = doneEvent;
            OnInit();
        }

        virtual void OnInit() {}
        virtual std::string    GetName()        { return ""; }
        uint32_t            GetExecTime()    { return _exec_time; }
        uint32_t            GetID()            { return _id; }
        EventHandle GetDoneEvent()    { return _doneEvent; }
        std::vector<uint32_t>    GetFlushIDs()    { return _flush_ids; }

    protected:
        void Add(Message* pNewMessage, uint32_t deltaTime = 0);

        void AddFlushID(uint32_t id) { _flush_ids.push_back(id); }

        EventHandle     _doneEvent;
        MessageQueue*   _pQueue;

    private:
        uint32_t        _exec_time;
        ThreadIdType    _caller_thread_id;

    protected:
        uint32_t         _id;
        std::vector<uint32_t> _flush_ids;
    };

    class EventMessageItem
    {
    public:
        EventMessageItem(std::shared_ptr<Message> message = nullptr, EventHandle startEvent = 0)
        :    _message(std::move(message))
        ,    _start_event(startEvent)
        {
        }

        std::shared_ptr<Message> _message;
        EventHandle _start_event;
    };

    class ICommandChecker
    {
    public:
        virtual bool TestCommandForRemoval(Message* command) = 0;
    };

    class MessageQueueBase
    {
    public:
        MessageQueueBase()
        {
        }

        virtual std::vector<uint32_t> GetPendingMessageIDs() = 0;
        virtual int GetPendingMessageCount() = 0;
    };

     class MessageQueue : public MessageQueueBase
    {
        using EventMessageList = std::list<EventMessageItem>;

    private:
        std::string GetTheClassName(std::string type_name)
        {
            Replace(type_name, "class ", "");
            return type_name;
        }

    public:
        using MessageList = std::list<std::shared_ptr<Message>>;

        MessageQueue(const char* name, bool auto_start = true,
                     Thread::WorkerThreadPriority priority = Thread::WorkerThreadPriority::NORMAL)
        :    MessageQueueBase()
        ,    _spReactor{ CreateReactorFactory(priority) }
        ,    _newMessageEvent{}
        ,    _newEventMessageEvent{}
        ,    _totalExecutedMessages(0)
        ,    _shutdown(false)
        {
            _spReactor->SetHandles( GetDefaultHandles() );

            _finishedExecuting.Set();

            if (auto_start)
                StartThread();
        }

        virtual ~MessageQueue()
        {
            // Should already have been stopped vi Shutdown phase one
            StopThread();
        }

        bool StartThread()
        {
            bool result = _spReactor->Start();

            std::lock_guard lock(_lock);
            _shutdown = false;
            return result;
        }

        bool StopThread()
        {
            bool result = _spReactor->Stop();

            std::lock_guard lock(_lock);

            // Don't accept any more messages
            _shutdown = true;

            _immediateMessages.clear();
            _timedMessages.clear();
            _eventDrivenMessages.clear();

            return result;
        }

        bool IsMessageQueueThread()
        {
            return CheckThreadID();
        }

        // You can supply additional event to break out of AddAndWait
        bool AddAndWait(Message* pInNewMessage, uint32_t deltaTime = 0,
                        std::vector<EventHandle> additionalEvents = {})
        {
            std::shared_ptr<Message> spNewMessage(pInNewMessage);

            // Can't AddAndWait if called from the message queue thread - it
            // will deadlock straight away
            if (IsMessageQueueThread())
            {
                uint32_t exec_time = OS::Get()->GetTimeMS();
                spNewMessage->Init(this, OS::Get()->GetCurrentThreadId(), exec_time, 0);

                // Execute the message right away since we've already got ProcessMessages on the stack
                {
                    std::lock_guard lock(_lock);
                    spNewMessage->PreExecute();
                }

                ExecuteMessage(spNewMessage);

                // PostExecute can calculate stats for this message
                _totalExecutedMessages++;
                spNewMessage->PostExecute();
                return true;
            }

            Event doneEvent;
            Add(spNewMessage, deltaTime, doneEvent);

            // Make sure thread is running if we are going to wait
            if (!_spReactor->IsRunning())
                return false;

            EventWaiter eventWaiter(doneEvent);
            for (auto additionalEvent : additionalEvents)
                eventWaiter.AddHandle(additionalEvent);

            EventHandle doneEventHandle = doneEvent;
            if (eventWaiter.Wait() != static_cast<int>(doneEventHandle))
            {
                // One of the additional events was signalled,
                // so the message doesn't get executed
                RemoveAndDeleteMessage(spNewMessage);
                return false;
            }
            
            return true;
        }

        // return true if OK or false if timed out
        // doneEvent is signalled by another thread when actually completed and must be set up before
        // this is called or we will timeout and return false!
        bool AddAndWaitForEvent(Message* pInNewMessage, EventHandle doneEvent)
        {
            std::shared_ptr<Message> spNewMessage(pInNewMessage);

            OS::Get()->ResetEvent(doneEvent);

            // Can't AddAndWait if called from the message queue thread - it
            // will deadlock straight away
            if (IsMessageQueueThread())
            {
                uint32_t exec_time = OS::Get()->GetTimeMS();
                spNewMessage->Init(this, OS::Get()->GetCurrentThreadId(), exec_time, 0);

                // Execute the message right away since we've already got ProcessMessages on the stack
                {
                    std::lock_guard lock(_lock);
                    spNewMessage->PreExecute();
                }

                ExecuteMessage(spNewMessage);

                // PostExecute can calculate stats for this message
                _totalExecutedMessages++;
                spNewMessage->PostExecute();
                return true;
            }

            Add(spNewMessage);

            // Make sure thread is running if we are going to wait
            if (!_spReactor->IsRunning())
                return false;

            while (true)
            {
                // Watch out for the thread exiting while we are waiting
                uint32_t r = OS::Get()->WaitForMultipleObjects((uint32_t)1, &doneEvent, false, 1000);
                if (r == 0)
                {
                    return true; // Done properly
                }
                else if (r != WAIT_TIMEOUT)
                {
                    RemoveAndDeleteMessage(spNewMessage);
                    return false;
                }
                else
                    return false;
            }
        }

        void Add(CommandQueueCB commandCB, uint32_t deltaStartTime = 0);
        void Add(AutoRepeatCommandQueueCB commandCB, uint32_t periodMS, uint32_t deltaStartTime = 0);
        void Add(VariableRepeatCommandQueueCB commandCB, uint32_t deltaStartTime = 0);

        void Add(Message* pInNewMessage, uint32_t deltaTime = 0, EventHandle doneEvent = 0)
        {
            std::shared_ptr<Message> spNewMessage(pInNewMessage);
            Add(std::move(spNewMessage), deltaTime, doneEvent);
        }

        void Add(const std::shared_ptr<Message>& spNewMessage, uint32_t deltaTime = 0, EventHandle doneEvent = 0)
        {
            auto flush_ids = spNewMessage->GetFlushIDs();
            for (auto flush_id : flush_ids)
                FlushMessages(flush_id);

            uint32_t exec_time = OS::Get()->GetTimeMS() + deltaTime;
            spNewMessage->Init(this, OS::Get()->GetCurrentThreadId(), exec_time, doneEvent);

            std::lock_guard lock(_lock);
            if (_shutdown)
                return;

            size_t nMessages = 0;
            (void)nMessages;
            if (deltaTime == 0)
            {
                _immediateMessages.push_back(std::move(spNewMessage));
                nMessages = _immediateMessages.size();
            }
            else
            {
                auto pos = _timedMessages.begin();
                bool inserted = false;
                int index = 0;
                while (pos != _timedMessages.end() && !inserted)
                {
                    auto this_message = *pos;
                    if (this_message->GetExecTime() > exec_time)
                    {
                        pos = _timedMessages.insert(pos, spNewMessage);
                        inserted = true;
                    }
                    pos++;
                    index++;
                }

                if (!inserted)
                {
                    _timedMessages.push_back(spNewMessage);
                }
            }

            _newMessageEvent.Set();
        }

        void Add(Message* pInNewMessage, EventHandle startEvent)
        {
            std::shared_ptr<Message> spNewMessage(pInNewMessage);

            spNewMessage->Init(this, OS::Get()->GetCurrentThreadId(), 0, 0);

            std::lock_guard lock(_lock);

            if (_shutdown)
                return;

            _eventDrivenMessages.push_back(EventMessageItem(spNewMessage, startEvent));
            _newEventMessageEvent.Set();
        }

        void AddFront(Message* pInNewMessage, EventHandle doneEvent = 0)
        {
            std::shared_ptr<Message> spNewMessage(pInNewMessage);

            uint32_t exec_time = OS::Get()->GetTimeMS();
            spNewMessage->Init(this, OS::Get()->GetCurrentThreadId(), exec_time, doneEvent);

            std::lock_guard lock(_lock);
            if (_shutdown)
                return;

            _immediateMessages.insert(_immediateMessages.begin(), std::move(spNewMessage));
            _newMessageEvent.Set();
        }

        bool MessageTypeExists(uint32_t id)
        {
            std::lock_guard lock(_lock);

            for (auto& message : _immediateMessages)
            {
                uint32_t thisID = message->GetID();
                if (thisID == id)
                    return true;
            }

            return false;
        }

        size_t FlushMessages(uint32_t id = (uint32_t)-1, bool executeLast = false)
        {
            MessageList flushedMessages;
            {
                std::lock_guard lock(_lock);

                auto pos = _timedMessages.begin();
                while (pos != _timedMessages.end())
                {
                    auto this_message = *pos;
                    uint32_t this_id = (uint32_t)this_message->GetID();
                    if ((this_id == id) || (id == (uint32_t)-1))
                    {
                        pos = _timedMessages.erase(pos);
                        flushedMessages.push_back(std::move(this_message));
                    }
                    else
                        pos++;
                }

                pos = _immediateMessages.begin();
                while (pos != _immediateMessages.end())
                {
                    auto this_message = *pos;
                    uint32_t this_id = (uint32_t)this_message->GetID();
                    if ((this_id == id) || (id == (uint32_t)-1))
                    {
                        pos = _immediateMessages.erase(pos);
                        flushedMessages.push_back(std::move(this_message));
                    }
                    else
                        pos++;
                }
            } // Release _lock

            size_t nFlushed = flushedMessages.size();

            if (executeLast && !flushedMessages.empty())
            {
                auto last_message = flushedMessages.back();
                flushedMessages.pop_back();

                {
                    std::lock_guard lock(_lock);
                    last_message->PreExecute();
                }

                ExecuteMessage(last_message);

                // Signal the message's done event if required
                if (last_message->GetDoneEvent() != 0)
                    OS::Get()->SetEvent(last_message->GetDoneEvent());

                // PostExecute can calculate stats for this message
                _totalExecutedMessages++;
                last_message->PostExecute();
            }

            return nFlushed;
        }

        size_t FlushMessages(ICommandChecker* pCommandChecker)
        {
            MessageList flushedMessages;
            {
                std::lock_guard lock(_lock);

                MessageList filteredTimedMessages;
                for (auto timedMessage : _timedMessages)
                {
                    if (pCommandChecker->TestCommandForRemoval(timedMessage.get()))
                        flushedMessages.push_back(timedMessage);
                    else
                        filteredTimedMessages.push_back(timedMessage);
                }

                if (filteredTimedMessages.size() != _timedMessages.size())
                    _timedMessages = filteredTimedMessages;

                MessageList filteredImmediateMessages;
                for (auto immediateMessage : _immediateMessages)
                {
                    if (pCommandChecker->TestCommandForRemoval(immediateMessage.get()))
                        flushedMessages.push_back(immediateMessage);
                    else
                        filteredImmediateMessages.push_back(immediateMessage);
                }

                if (filteredImmediateMessages.size() != _immediateMessages.size())
                    _immediateMessages = filteredImmediateMessages;

            } // Release _lock

            size_t nFlushed = flushedMessages.size();
            return nFlushed;
        }

        size_t GetNumMessages()
        {
            std::lock_guard lock(_lock);
            size_t nMessages = _immediateMessages.size();
            nMessages += _timedMessages.size();
            return nMessages;
        }

        // Temporarily freeze the message queue. Can be called from the
        // message queue thread (i.e. during Execute)
        void Freeze(bool state)
        {
            _spReactor->Freeze(state);
        }

        bool IsFrozen()
        {
            return _spReactor->IsFrozen();
        }

        std::vector<uint32_t> GetPendingMessageIDs() override
        {
            std::lock_guard lock(_lock);
            std::vector<uint32_t> pendingIDs;
            for (auto& immediateMessage : _immediateMessages)
                pendingIDs.push_back(immediateMessage->GetID());
            for (auto& timeMessage : _timedMessages)
                pendingIDs.push_back(timeMessage->GetID());

            return pendingIDs;
        }

        int GetPendingMessageCount() override
        {
            return (int)GetNumMessages();
        }

        bool WaitTillFinishedExecuting(uint32_t timeout = INFINITE)
        {
            return _finishedExecuting.IsSignalled(timeout);
        }

        std::recursive_mutex& GetLock()    { return _lock; }
        MessageList* GetMessages()    { return &_immediateMessages; }
        bool CheckThreadID()        { bool match = (_spReactor->GetThreadId() == static_cast<uint32_t>(OS::Get()->GetCurrentThreadId())); return match; }

    private:
        void HandleEvent(uint32_t r, const std::vector<EventHandle>& originalHandles)
        {
            if (r == WAIT_TIMEOUT)
            {
                // A timed message is ready
                std::shared_ptr<Message> spTimedMessage;

                {
                    std::lock_guard lock(_lock);
                    size_t n_timed_messages = _timedMessages.size();
                    if (n_timed_messages > 0)
                    {
                        spTimedMessage = _timedMessages.front();
                        _timedMessages.pop_front();

                        spTimedMessage->PreExecute();
                    }
                }

                if (spTimedMessage)
                {
                    // Now execute the message
                    ExecuteMessage(spTimedMessage);

                    // Signal the message's done event if required
                    if (spTimedMessage->GetDoneEvent() != 0)
                        OS::Get()->SetEvent(spTimedMessage->GetDoneEvent());

                    // PostExecute can calculate stats for this message
                    _totalExecutedMessages++;
                    spTimedMessage->PostExecute();
                }
            }
            else if (r == (WAIT_OBJECT_0 + 0))
            {
                // New message event
            }
            else if (r == (WAIT_OBJECT_0 + 1))
            {
                // New event driven message
                // We need to reset the handles array
                std::lock_guard lock(_lock);
                _newEventMessageEvent.Reset();
                std::vector<EventHandle> handles = GetDefaultHandles();

                int i = 0;
                for (auto& event_driven_message : _eventDrivenMessages)
                {
                    handles.push_back(event_driven_message._start_event);
                    i++;
                }

                _spReactor->SetHandles(std::move(handles));
            }
            else if ((r >= (WAIT_OBJECT_0 + 2)) && (r < (WAIT_OBJECT_0 + originalHandles.size())))
            {
                // It's an event driven message who's event has been signalled
                int eventIndex = r - WAIT_OBJECT_0;
                int event_message_offset = eventIndex - 2;
                EventMessageItem event_message_item{};
                {
                    std::lock_guard lock(_lock);
                    auto iter = _eventDrivenMessages.begin();

                    // This is not efficient if you have a large number of event driven messages
                    while ( (event_message_offset > 0) && (iter != _eventDrivenMessages.end()) )
                    {
                        ++iter;
                        --event_message_offset;
                    }

                    if (iter != _eventDrivenMessages.end())
                    {
                        event_message_item = *iter;
                        _eventDrivenMessages.erase(iter);
                        event_message_item._message->PreExecute();
                    }
                }

                if (event_message_item._message)
                {
                    ExecuteMessage(event_message_item._message);

                    // PostExecute can calculate stats for this message
                    _totalExecutedMessages++;
                    event_message_item._message->PostExecute();

                    // Need to remove this item's event handle from the handles vector
                    auto handles = originalHandles;
                    handles.erase(handles.begin() + eventIndex);
                    _spReactor->SetHandles( std::move(handles) );
                }
            }

            // Execute everything on the immediate list
            while (_newMessageEvent.IsSignalled() && !_spReactor->IsShuttingDown())
            {
                _spReactor->WaitIfFrozen();

                std::shared_ptr<Message> spImmediateMessage;

                {
                    std::lock_guard lock(_lock);
                    size_t nImmediateMessage = _immediateMessages.size();
                    if (nImmediateMessage > 0)
                    {
                        spImmediateMessage = _immediateMessages.front();
                        _immediateMessages.pop_front();
                        spImmediateMessage->PreExecute();
                    }
                    else
                        _newMessageEvent.Reset();
                }

                if (spImmediateMessage)
                {
                    // Now execute the message
                    ExecuteMessage(spImmediateMessage);

                    // Signal the message's done event if required
                    if (spImmediateMessage->GetDoneEvent() != 0)
                        OS::Get()->SetEvent(spImmediateMessage->GetDoneEvent());

                    // PostExecute can calculate stats for this message
                    _totalExecutedMessages++;
                    spImmediateMessage->PostExecute();
                }
            }
        }

        void ShuttingDownThread()
        {
            // Shutting down. If there are any messages still in the queue
            // that could be waiting, signal them
            std::lock_guard lock(_lock);
            for (auto message : _timedMessages)
            {
                if (message->GetDoneEvent() != 0)
                    OS::Get()->SetEvent(message->GetDoneEvent());
            }

            for (auto message : _immediateMessages)
            {
                if (message->GetDoneEvent() != 0)
                    OS::Get()->SetEvent(message->GetDoneEvent());
            }
        }

        void RemoveAndDeleteMessage(std::shared_ptr<Message>& spRemoveMessage)
        {
            EventHandle doneEvent = spRemoveMessage->GetDoneEvent();
            std::lock_guard lock(_lock);
            auto pos = _immediateMessages.begin();
            while (pos != _immediateMessages.end())
            {
                auto message = *pos;
                if (message == spRemoveMessage)
                {
                    _immediateMessages.erase(pos);
                    spRemoveMessage.reset();
                    return;
                }
                pos++;
            }

            pos = _timedMessages.begin();
            while (pos != _timedMessages.end())
            {
                auto message = *pos;
                if (message == spRemoveMessage)
                {
                    _timedMessages.erase(pos);
                    spRemoveMessage.reset();
                    return;
                }
                pos++;
            }
            // We didn't find it - it must have already been removed from the list and about to
            // be executed - we have to wait for it to finish
            if (doneEvent != 0)
                OS::Get()->WaitForSingleObject(doneEvent, INFINITE);
        }

        uint32_t HeadWaitTime()
        {
            std::lock_guard lock(_lock);
            if (_spReactor->IsShuttingDown())
                return 1;

            if (_timedMessages.empty())
                return INFINITE;
            uint32_t now = OS::Get()->GetTimeMS();
            auto head_message = _timedMessages.front();
            uint32_t exec_time = head_message->GetExecTime();
            if (now > exec_time)
                return 0; // late already
            else
                return (exec_time - now);
        }

        void ExecuteMessage(std::shared_ptr<Message> message)
        {
            _finishedExecuting.Reset();
            message->Execute();
            _finishedExecuting.Set();
        }

        std::vector<EventHandle> GetDefaultHandles()
        {
            std::vector<EventHandle> handles{};
            handles.push_back(_newMessageEvent);
            handles.push_back(_newEventMessageEvent);
            return handles;
        }

        Reactor::AllCallbacks GetReactorCallbacks()
        {
            Reactor::AllCallbacks callbacks{};

            callbacks.calculateWaitTime = [this] ()
                {
                    return HeadWaitTime();
                };
            callbacks.threadStarted = [this] ()
                {
                };
            callbacks.threadStopped = [this] ()
                {
                    ShuttingDownThread();
                };
            callbacks.handleEvent = [this] (uint32_t waitResult, const std::vector<EventHandle>& originalHandles)
                {
                    HandleEvent(waitResult, originalHandles);
                };

            return callbacks;
        }

        std::shared_ptr<IReactorInternal> CreateReactorFactory(Thread::WorkerThreadPriority priority)
        {
            std::string className = "";
            return ReactorFactory::Create( className.c_str(), priority, GetReactorCallbacks() );
        }

        std::shared_ptr<IReactorInternal> _spReactor;
        Event _newMessageEvent;
        Event _newEventMessageEvent;
        MessageList _immediateMessages;
        MessageList _timedMessages;
        EventMessageList _eventDrivenMessages;

        std::recursive_mutex _lock;
        uint32_t _totalExecutedMessages;
        bool _shutdown;
        Event _finishedExecuting;
    };

    enum class QueueMessageIDs : uint32_t
    {
        SINGLE_CALLBACK_CMD,
        AUTO_REPEAT_CALLBACK_CMD,
        VARIABLE_REPEAT_CALLBACK_CMD,

        LAST_ID
    };

    class SingleCallbackCmd : public Message
    {
    public:
        SingleCallbackCmd(CommandQueueCB callback)
        :	Message((uint32_t)QueueMessageIDs::SINGLE_CALLBACK_CMD)
        ,   _callback(std::move(callback))
        {
        }
        void Execute() override
        {
            if (_callback)
                _callback();
        }

    private:
        CommandQueueCB _callback;
    };    

    // If callback returns false, the auto repeat stops. The callback function
    // can modify the counter if necessary

    class AutoRepeatCmd : public Message
    {
    public:
        AutoRepeatCmd(AutoRepeatCommandQueueCB callback, uint32_t periodMS, uint32_t counter = 0)
        :	Message((uint32_t)QueueMessageIDs::AUTO_REPEAT_CALLBACK_CMD)
        ,   _callback(std::move(callback))
        ,   _periodMS(periodMS)
        ,   _counter(counter)
        {
        }      

        void Execute() override
        {
            if (_callback)
            {
                uint32_t previousCounter = _counter;
                if (_callback(_counter))
                {
                    // Auto increment the counter if the callback hasn't modified it
                    if (_counter == previousCounter)
                        _counter++;
                    _pQueue->Add(new AutoRepeatCmd(_callback, _periodMS, _counter), _periodMS);
                }
            }
        }        

    private:
        AutoRepeatCommandQueueCB _callback;
        uint32_t _periodMS;
        uint32_t _counter;
    };

    // If callback returns false, the auto repeat stops. The callback function
    // can modify the counter if necessary

    class VariableRepeatCmd : public Message
    {
    public:
        VariableRepeatCmd(VariableRepeatCommandQueueCB callback)
        :	Message((uint32_t)QueueMessageIDs::VARIABLE_REPEAT_CALLBACK_CMD)
        ,   _callback(std::move(callback))
        {
        }

        void Execute() override
        {
            if (_callback)
            {
                int32_t nextTimeout = _callback();
                if (nextTimeout >= 0)
                    _pQueue->Add(new VariableRepeatCmd(_callback), (uint32_t)nextTimeout);
            }
        }        

    private:
        VariableRepeatCommandQueueCB _callback;
    };    

} // namespace SwUtils
