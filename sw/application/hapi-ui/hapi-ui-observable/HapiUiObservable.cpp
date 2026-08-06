/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "HapiUiObservable.hpp"

bool ObservableIpUi::RegisterOnImageSettingsChangeCb(std::function<void(void)> callback) {
    _onImageSettingsChangeCallbacks.push_back(callback);
    return true;
}

bool ObservableIpUi::ImageSettingsChanged() {
    for (const auto& fn : _onImageSettingsChangeCallbacks) {
        fn();
    }
    return true;
}

bool ObservableIpUi::RegisterMinorSettingsChangeCb(std::function<void(void)> callback) {
    _onMinorSettingsChangeCallbacks.push_back(callback);
    return true;
}

bool ObservableIpUi::MinorSettingsChanged() {
    for (const auto& fn : _onMinorSettingsChangeCallbacks) {
        fn();
    }
    return true;
}