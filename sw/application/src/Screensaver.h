/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#pragma once

#include <cstdint>
#include <memory>
#include "LogoControls.h"
#include "ObserverPattern.h"
#include "CommonApplicationBase.h"


using ui_activity_notifier_t = int;


class Screensaver : public Observer<ui_activity_notifier_t>
{
public:
    Screensaver(uint32_t timeout_period, std::function<void(bool)> onScreensaverCb, const std::shared_ptr<INotifier<ui_activity_notifier_t>>& notifier):
    
      _enabled{false},
      _callback_scheduled{false},
      _period(timeout_period), 
      _onScreensaverCb{std::move(onScreensaverCb)},
      _timeout_counter(0),
      _timed_out(false)
    {
        Register(notifier.get());

        if(IsEnabled())
            ScheduleCallback();        
    }

    virtual ~Screensaver()
    {
        DoUnregister();
    }

    void DoUnregister() override
    {
        Unregister();
    }

    bool Update(ui_activity_notifier_t&) override
    {
        ResetTimeout();
        return true;
    }

    bool IsEnabled() const
    {
        return _enabled;
    }

    // Enable() and ScheduleCallback()
    // must be called from the same context -
    // currently the main applicaiton queue
    void Enable(const bool v)
    {
        _enabled = v;

        if(IsEnabled())
            ScheduleCallback();
    }

    void SetTimeoutPeriod(uint32_t period)
    {
        _period = period;
    }

private:
    void ScheduleCallback()
    {
        // Make sure there's only one callback in the application queue
        if(!_callback_scheduled)
        {
            auto callback_func = [this](uint32_t&)->bool {
                if(IsEnabled()){
                    TimeoutCallback();
                    return true;
                }
                // If the callback returns false it won't be automatically re-scheduled
                _callback_scheduled = false;
                return false;
            };

            CommonApplicationBase::Get()->AddCommand(callback_func, 1000);
            _callback_scheduled = true;
        }
    }

    void TimeoutCallback()
    {
        if (_timed_out)
            return;
        
        _timeout_counter++;

        if (_timeout_counter > _period)
        {
            if(_onScreensaverCb)
                _onScreensaverCb(true);                

            _timed_out = true;
        }
    }

    void ResetTimeout()
    {
        if (_timed_out)
        {
            if(_onScreensaverCb)
                _onScreensaverCb(false);

            _timed_out = false;
        }
        _timeout_counter = 0;
    }

    bool _enabled;
    bool _callback_scheduled;
    uint32_t _period;
    std::function<void(bool)> _onScreensaverCb;
    uint32_t _timeout_counter;
    bool _timed_out;
};
