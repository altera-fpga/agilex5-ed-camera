/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/

#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <mutex>
#include <condition_variable>
#include <chrono>

class Semaphore {
public:
    Semaphore(int count = 0)
        : _count(count)
    {
    }

    inline void notify() {
        std::unique_lock<std::mutex> lock(_cs);
        _count++;
        _cv.notify_one();
    }
    inline bool wait(int timeout) {
        std::unique_lock<std::mutex> lock(_cs);
        while (_count == 0) {
            if (std::cv_status::timeout == _cv.wait_for(lock, std::chrono::milliseconds(timeout))) {
                return false;
            }
        }
        _count--;
        return true;
    }
private:
    std::mutex _cs;
    std::condition_variable _cv;
    int _count;
};

#endif //__SEMAPHORE_H__
