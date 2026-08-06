/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once
#include <concepts>

namespace SwUtils
{

template<typename T>
concept StdLock = requires
{
    typename T::mutex_type;
} && std::is_constructible_v<T, typename T::mutex_type&>;

template<typename T, StdLock LockType>
class LockedHandle
{
public:
    LockedHandle(T& handle, typename LockType::mutex_type& mutex)
        : _handle(handle)
        , _lock(mutex)
    {
    }

    const T* operator->() const
    {
        return &_handle;
    }

    T* operator->()
    {
        return const_cast<T*>(static_cast<const LockedHandle<T, LockType>*>(this)->operator->());
    }
    
    const T& Get() const
    {
        return _handle;
    }

    T& Get()
    {
        return const_cast<T&>(static_cast<const LockedHandle<T, LockType>*>(this)->Get());
    }

private:
    T& _handle;
    LockType _lock;
};

} // namespace SwUtils