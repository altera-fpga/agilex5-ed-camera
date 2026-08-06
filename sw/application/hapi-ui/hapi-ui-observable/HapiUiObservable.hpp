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
#include <functional>

struct IObservableIpUi {
    virtual bool RegisterMinorSettingsChangeCb(std::function<void(void)> callback) = 0;
    virtual bool RegisterOnImageSettingsChangeCb(std::function<void(void)> callback) = 0;
    virtual bool MinorSettingsChanged() = 0;
    virtual bool ImageSettingsChanged() = 0;
};

struct ObservableIpUi : public virtual IObservableIpUi {
    std::vector<std::function<void(void)>> _onMinorSettingsChangeCallbacks;
    std::vector<std::function<void(void)>> _onImageSettingsChangeCallbacks;

    bool RegisterMinorSettingsChangeCb(std::function<void(void)> callback) override;
    bool RegisterOnImageSettingsChangeCb(std::function<void(void)> callback) override;

    bool MinorSettingsChanged() override;
    bool ImageSettingsChanged() override;
};
