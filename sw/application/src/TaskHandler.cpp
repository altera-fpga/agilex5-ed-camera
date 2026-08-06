/*******************************************************************************
Copyright (C) Altera Corporation

This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the 
License.
*******************************************************************************/
#include "TaskHandler.h"
#include "VvpIspDemo.h"


TaskHandler::TaskHandler():
    _running{false}
{
}


TaskHandler::~TaskHandler()
{
}


void TaskHandler::Start()
{
    if(!_running)
    {
        _running = true;

        for(auto& task: _task)
        {
            if(task._enabled)
            ScheduleTask(task);
        }
    }
}


std::size_t TaskHandler::CreateTask(std::function<void(void)> task_proc, uint32_t interlval)
{
    std::size_t task_id = _task.size();
    _task.emplace_back(Task{false, false, interlval, std::move(task_proc)});
    return task_id;
}


void TaskHandler::EnableTask(const std::size_t task_idx)
{
    if(task_idx < _task.size())
    {
        auto& task = _task[task_idx];
        task._enabled = true;

        if(_running)
            ScheduleTask(task);
    }
}


void TaskHandler::DisableTask(const std::size_t task_idx)
{
    if(task_idx < _task.size())
    {
        auto& task = _task[task_idx];
        task._enabled = false;        
    }    
}


void TaskHandler::ScheduleTask(Task& task)
{
    if(task._scheduled == false){
        auto task_wrapper = [&task](uint32_t&)->bool{
            bool ret = task._enabled;

            if(ret)
                task._proc();

            task._scheduled = ret;
            return ret;                
        };

        VvpIspDemo::Get()->AddCommand(task_wrapper, task._interval);
        task._scheduled = true;
    }
}
