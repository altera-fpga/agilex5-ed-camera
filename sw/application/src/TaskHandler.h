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
#include <vector>
#include <functional>


class TaskHandler
{
public:
    TaskHandler();
    ~TaskHandler();
    std::size_t CreateTask(std::function<void(void)> task_proc, uint32_t interlval = 0);
    void EnableTask(const std::size_t task_idx);
    void DisableTask(const std::size_t task_idx);
    void Start();

private:
    struct Task{
        bool _enabled;
        bool _scheduled;
        uint32_t _interval;
        std::function<void(void)> _proc;
    };

    void ScheduleTask(Task& task);

    std::vector<Task> _task;
    bool _running;
};
