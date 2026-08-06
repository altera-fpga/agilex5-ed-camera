/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include "AtUtils.h"

using LinkLockPtr = std::shared_ptr<std::recursive_mutex>;

template<class DataClass>
class Observer;

template<class DataClass>
class INotifier;

template<class DataClass>
class Observer
{
public:
    Observer() : _pNotifier(nullptr), _single_shot(false) {}
    virtual ~Observer()
    {
        // We will automatically unregister with the notifiers on deletion
        bool unregistered = Unregister();
        (void)unregistered;

        // You should call Unregister from your derived class
        assert(!unregistered);
    }

    // When you derive from Observer, you must call Unregister in your
    // derived classes destructor. DoUnregister is simply here to remind
    // you that you must do this.
    virtual void DoUnregister() = 0;

    bool Register(INotifier<DataClass>* pNotifier)
    {
        if (pNotifier == nullptr)
            return false;

        std::lock_guard notifierLock(*(pNotifier->GetNotifierLock()));
        std::lock_guard lock(_privateCs);

        // Make sure not already registered
        if (_pNotifier)
        {
            assert(pNotifier == _pNotifier);
            return false;
        }

        _pNotifier = pNotifier;

        _spLinkLock = pNotifier->AddObserver(this);
        if (!_spLinkLock)
        {
            assert(0);
            return false;
        }

        return true;
    }

    bool Unregister()
    {
        LinkLockPtr link_lock_copy;

        {
            std::lock_guard lock(_privateCs);
            if (!_pNotifier)
                return false;

            // If we have a notifier we will always have a _spLinkLock
            link_lock_copy = _spLinkLock;
        }

        {
            std::lock_guard notifierLock(*link_lock_copy);
            std::lock_guard lock(_privateCs);
            if (!_pNotifier) // We must check again because ClearNotifier could have got in
                return false;

            _pNotifier->RemoveObserver(this);
            _pNotifier = nullptr;

            // Release the link lock
            _spLinkLock.reset();
        }

        return true;
    }

    // Your observer needs to implement this member
    // Return false and the observer will be automatically removed
    virtual bool Update(DataClass& data) = 0;

    // Called from Notifier who has the locks
    void ClearNotifier()
    {
        std::lock_guard lock(_privateCs);
        _pNotifier = nullptr;
        _spLinkLock.reset();
    }

    LinkLockPtr	_spLinkLock;
    std::recursive_mutex	_privateCs;
    INotifier<DataClass>*	_pNotifier;
    // single shot observers get called once and then are destroyed
    bool IsSingleShot() { return _single_shot; }
protected:
    bool _single_shot;
};

template<class DataClass>
class INotifier
{
public:
    virtual ~INotifier() {}
    virtual void ClearObservers() = 0;
    virtual size_t Notify(DataClass& newData) = 0;
    virtual bool Notify(DataClass& newData, double& maxTime, std::string& name) = 0;
    virtual void ObserversChanged(size_t from , size_t to, Observer<DataClass>* pObserver) = 0;

    // Override if you want to know when your first observer registers
    virtual void FirstObserverConnected() {}

    // Override if you want ot know when the last observer unregisters
    virtual void LastObserverDisconnected() {}

    virtual std::string GetNotifierName() = 0;
    virtual void CopyObserverList(std::list<Observer<DataClass>*>& copyTo) = 0;
    virtual LinkLockPtr GetNotifierLock() = 0;
    virtual size_t NumObservers() = 0;
    virtual LinkLockPtr AddObserver(Observer<DataClass>* pObserver) = 0;
    virtual bool RemoveObserver(Observer<DataClass>* pObserver) = 0;
};

template<class DataClass>
class Notifier : public INotifier<DataClass>
{
public:
    Notifier()
    : _spLinkLock(new std::recursive_mutex)
    , _shuttingDown(false)
    {
    }

    virtual ~Notifier()
    {
        // This should be done in the derived classes destructor
        // to be safe, because we will be partially destroyed at this point
        ClearObservers();
    }

    void ClearObservers() override
    {
        std::lock_guard notifierLock(*_spLinkLock);
        _shuttingDown = true;

        while (!_observers.empty())
        {
            size_t previousCount = _observers.size();
             Observer<DataClass>* pObserver = _observers.front();
            _observers.pop_front();
            pObserver->ClearNotifier();

            ObserversChanged(previousCount, _observers.size(), pObserver);
        }
    }

    // Return number of observers for convenience
    size_t Notify(DataClass& newData) override
    {
        std::lock_guard notifierLock(*_spLinkLock);

        size_t nObservers = _observers.size();

        std::list<Observer<DataClass>*> singleshotObservers;
        for (auto pObserver : _observers)
        {
            bool continue_observing = pObserver->Update(newData);
            if (!continue_observing || pObserver->IsSingleShot())
                singleshotObservers.push_back(pObserver);
        }

        // now remove any single shot observers
        for (auto& pSingleshotObserver : singleshotObservers)
        {
            RemoveObserver(pSingleshotObserver);
            pSingleshotObserver->ClearNotifier();
        }

        return nObservers;
    }

    bool Notify(DataClass& newData, double& maxTime, std::string& name) override
    {
        std::lock_guard notifierLock(*_spLinkLock);
        maxTime = 0.0;
        bool oneFailed = false;

        for (auto pObserver : _observers)
        {
            bool r = pObserver->Update(newData);
            if (!r)
                oneFailed = true;
        }

        maxTime *= 1000.0;
        return oneFailed;
    }

    void ObserversChanged(size_t from , size_t to, Observer<DataClass>* pObserver) override
    {
        if ((from == 0) && pObserver)
            this->FirstObserverConnected();
        else if (to == 0)
            this->LastObserverDisconnected();
    }

    // For logging
    virtual std::string GetNotifierName() override { return ""; }

    void CopyObserverList(std::list<Observer<DataClass>*>& copyTo) override
    {
        std::lock_guard notifierLock(*_spLinkLock);
        for (auto pObserver : _observers)
            copyTo.push_back(pObserver);
    }

    // Use this when you are copying the observer list above
    LinkLockPtr GetNotifierLock() override { return _spLinkLock; }

    size_t NumObservers() override
    {
        std::lock_guard notifierLock(*_spLinkLock);
        size_t n = _observers.size();
        return n;
    }

    // Called from observer, who already has the locks
    virtual LinkLockPtr AddObserver(Observer<DataClass>* pObserver) override
    {
        // Make sure not already added
        for (auto pCheckObserver : _observers)
        {
            if (pCheckObserver == pObserver)
                return nullptr;
        }

        size_t previousCount = _observers.size();
        _observers.push_back(pObserver);

        if (!_shuttingDown && !pObserver->IsSingleShot())
            ObserversChanged(previousCount, _observers.size(), pObserver);

        return _spLinkLock;
    }

    // Called from observer, who already has the lock
    bool RemoveObserver(Observer<DataClass>* pObserver) override
    {
        size_t previousCount = _observers.size();
        int nRemoved = AtUtils::Remove<Observer<DataClass>*>(_observers, pObserver);
        if (nRemoved == 0)
            return false;

        if (!_shuttingDown)
            ObserversChanged(previousCount, _observers.size(), 0);

        return true;
    }

    LinkLockPtr					_spLinkLock;
    std::list<Observer<DataClass>*>	_observers;
    int							_shuttingDown;
};

// An observer can only register for one Notifier. When you
// need updates from more than one Notifier, use the ObserverFunnel,
// which registers using OWNER as your class name and DataClass as
// the type of data you want to observe. You then must
// implement Update(DataClass& data) in your owner class.
//
// ObserverProxy below is similar and maybe more useful
//
template <class OWNER, class DataClass>
class ObserverFunnel : public Observer<DataClass>
{
public:
    ObserverFunnel(OWNER* pOwner, Notifier<DataClass>* pNotifier)
    :	_pOwner(pOwner)
    {
        if (pNotifier)
            Observer<DataClass>::Register(pNotifier);
    }

    virtual ~ObserverFunnel()
    {
        Observer<DataClass>::Unregister();
    }

    virtual void DoUnregister() {}
    virtual bool Update(DataClass& data)
    {
        // Forward on to owner
        if (_pOwner)
            _pOwner->Update(data);

        return true;
    }

protected:
    OWNER* _pOwner;
};

// Forward notification callbacks to a function of your choice
template <class OWNER, class NOTIFY_PARAM>
class ObserverProxy : public Observer<std::shared_ptr<NOTIFY_PARAM>>
{
    using ObserverProxyCallback = void (OWNER::*)(std::shared_ptr<NOTIFY_PARAM>& notify_param);

public:
    ObserverProxy(OWNER* pOwner, ObserverProxyCallback callback,
                  Notifier<std::shared_ptr<NOTIFY_PARAM>>* pNotifier)
    :	_pOwner(pOwner)
    ,	_callback(callback)
    {
        Observer<std::shared_ptr<NOTIFY_PARAM>>::Register(pNotifier);
    }

    virtual ~ObserverProxy()
    {
        Observer<std::shared_ptr<NOTIFY_PARAM>>::Unregister();
    }

    virtual void DoUnregister() {}

    virtual bool Update(std::shared_ptr<NOTIFY_PARAM>& notify_param)
    {
        // Forward the Update to the supplied callback function
        (_pOwner->*_callback)(notify_param);
        return true;
    }

private:
    OWNER* _pOwner;
    ObserverProxyCallback _callback;
};

// Useage, eg:
// typedef ObserverProxy<EyeInputService, NewNodeConnection> NewNodeConnectionObserver;


