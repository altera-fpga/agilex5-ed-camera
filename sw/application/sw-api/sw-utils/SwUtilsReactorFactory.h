/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include "SwUtilsReactor.h"
#include "SwUtilsReactorInternal.h"

#include <functional>
#include <vector>
#include <memory>

namespace SwUtils
{
    class ReactorFactory
    {
    public:
        static std::shared_ptr<IReactorInternal> Create(const char* threadName, Thread::WorkerThreadPriority priority,
                                                       IReactorInternal::AllCallbacks allCallbacks);

        using CreateFunction = std::function<std::shared_ptr<IReactorInternal>(const char* threadName,
                                                                              Thread::WorkerThreadPriority priority,
                                                                              IReactorInternal::AllCallbacks allCallbacks)>;
        // For testing
        static void SetCustomCreator( CreateFunction createFunction );

    private:
        static CreateFunction s_customCreator;
    };

} // namespace SwUtils
