/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "AppToolkitReactorFactory.h"

namespace AtUtils
{
    ReactorFactory::CreateFunction ReactorFactory::s_customCreator;

    std::shared_ptr<IReactorInternal> ReactorFactory::Create(const char* threadName, Thread::WorkerThreadPriority priority,
                                                             IReactorInternal::AllCallbacks allCallbacks)
    {
        if (s_customCreator)
        {
            return s_customCreator( threadName, priority, std::move(allCallbacks) );
        }

        return std::make_shared<Reactor>( threadName, priority, std::move(allCallbacks) );
    }

    void ReactorFactory::SetCustomCreator( CreateFunction createFunction )
    {
        s_customCreator = std::move(createFunction);
    }

} // namespace AtUtils
